/**
 * @file nsga3r.cpp
 * @brief Main implementation of the NSGA-III algorithm.
 *
 * This file implements the NSGA-III routine including population initialization,
 * evaluation, selection, crossover/mutation, merging, ranking and reporting.
 * It also contains several scenario‐related functions used to load and process
 * external configuration and scenario data.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 #include <unistd.h>
 #include "global.h"
 #include "rand.h"
 #include <iostream>
 #include <string>
 #include <misc.hpp>
 #include <vector>
 #include <fmt/core.h>
 #include <filesystem>
 #include <fstream>
 #include <nlohmann/json.hpp>
 
 // For convenience in filesystem operations.
 namespace fs = std::filesystem;
 using json = nlohmann::json;
 
 /**
  * @brief Splits a string by the given delimiter.
  *
  * @param str The input string.
  * @param delim The delimiter character.
  * @param[out] out The vector that will hold the split substrings.
  */
 void split_str(std::string const &str, const char delim,
                std::vector<std::string> &out);
 
 // Global variables used by the NSGA-III routine.
 extern std::vector<double> alpha_stored_vec;
 int nreal;
 int nbin;
 int nobj;
 int ncon;
 int popsize;
 double pcross_real;
 double pcross_bin;
 double pmut_real;
 double pmut_bin;
 double eta_c;
 double eta_m;
 int ngen;
 int nbinmut;
 int nrealmut;
 int nbincross;
 int nrealcross;
 int *nbits;
 double *min_realvar;
 double *max_realvar;
 double *min_binvar;
 double *max_binvar;
 int bitlength;
 int choice;
 int obj1;
 int obj2;
 int obj3;
 int angle1;
 int angle2;
 
 /* Extra variables for MNSGA-II and CBW */
 double *ref_pt[NMAX];
 double *ideal_point;
 double **plane_point;
 double scaling;
 int steps;
 int nref;
 int curr_gen;
 int onref;
 int created_around[NMAX];
 int adaptive_increment;
 int active_ref_pts[10];
 int start_incr;
 
 /* End extra variables for CBW */
 
 /**
  * @namespace my_execute
  * @brief Contains functionality to execute system commands.
  */
 namespace my_execute{
 
     /**
      * @brief Structure to hold the result of a system command execution.
      */
     struct CommandResult {
         std::string output;  ///< STDOUT captured from the command.
         int exitstatus;      ///< Exit status of the command.
 
         /**
          * @brief Overloaded stream insertion operator for CommandResult.
          */
         friend std::ostream &operator<<(std::ostream &os, const CommandResult &result) {
             os << "command exitstatus: " << result.exitstatus << " output: " << result.output;
             return os;
         }
         /**
          * @brief Equality operator.
          */
         bool operator==(const CommandResult &rhs) const {
             return output == rhs.output &&
                    exitstatus == rhs.exitstatus;
         }
         /**
          * @brief Inequality operator.
          */
         bool operator!=(const CommandResult &rhs) const {
             return !(rhs == *this);
         }
     };
 
     /**
      * @brief Class to execute system commands and capture their output.
      */
     class Command {
     public:
         /**
          * @brief Executes a system command and returns its STDOUT and exit status.
          *
          * This function uses popen() to capture the command's STDOUT. If the command fails,
          * a std::runtime_error is thrown.
          *
          * @param command The system command to execute.
          * @return CommandResult containing the command's STDOUT output and exit status.
          */
         static CommandResult exec(const std::string &command) {
             int exitcode = 0;
             std::array<char, 1048576> buffer {};
             std::string result;
             FILE *pipe = popen(command.c_str(), "r");
             if (pipe == nullptr) {
                 throw std::runtime_error("popen() failed!");
             }
             try {
                 std::size_t bytesread;
                 while ((bytesread = std::fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0) {
                     result += std::string(buffer.data(), bytesread);
                 }
             } catch (...) {
                 pclose(pipe);
                 throw;
             }
             exitcode = WEXITSTATUS(pclose(pipe));
             return CommandResult{result, exitcode};
         }
     };
 } // end namespace my_execute
 
 /**
  * @brief Computes the land conversion keys.
  *
  * Iterates over the map @c land_conversion_from_bmp_to and stores the keys in @c lc_keys_.
  * The keys are then sorted.
  */
 void scenario__compute_lc_keys() {
     for (const auto& pair : land_conversion_from_bmp_to) {
         lc_keys_.push_back(pair.first);
     }
     std::sort(lc_keys_.begin(), lc_keys_.end());
 }
 
 /**
  * @brief Computes the animal keys.
  *
  * Iterates over the map @c animal_complete_ and stores the keys in @c animal_keys_.
  * The keys are then sorted.
  */
 void scenario__compute_animal_keys() {
     for (const auto& pair : animal_complete_) {
         animal_keys_.push_back(pair.first);
     }
     std::sort(animal_keys_.begin(), animal_keys_.end());
 }
 
 /**
  * @brief Loads a scenario from a JSON file.
  *
  * This function opens the JSON file given by @p filename and loads various
  * scenario parameters (amounts, conversion mappings, costs, and animal information).
  *
  * @param filename The path to the JSON file.
  */
 void scenario__load(const std::string& filename) {
     std::ifstream file(filename);
     if (!file.is_open()) {
         std::cerr << "Failed to open the file." << std::endl;
         exit(-1);
         return;
     }
     json json_obj = json::parse(file);
     amount_ = json_obj["amount"].get<std::unordered_map<std::string, double>>();
     land_conversion_from_bmp_to = json_obj["land_conversion_to"].get<std::unordered_map<std::string, std::vector<std::string>>>();
     bmp_cost_ = json_obj["bmp_cost"].get<std::unordered_map<std::string, double>>();
     animal_complete_ = json_obj["animal_complete"].get<std::unordered_map<std::string, std::vector<int>>>();
     animal_ = json_obj["animal_unit"].get<std::unordered_map<std::string, double>>();
     scenario__compute_lc_keys();
     scenario__compute_animal_keys();
 }
 
 /**
  * @brief Computes the size (number of variables) for land conversion.
  *
  * @return The total count including keys and associated BMP groups.
  */
 int scenario__compute_lc_size() {
     int counter = 0;
     for (const auto &[key, bmp_group] : land_conversion_from_bmp_to) {
         ++counter;
         for (const auto &bmp : bmp_group) {
             ++counter;
         }
     }
     return counter;
 }
 
 /**
  * @brief Computes the size (number of variables) for animal data.
  *
  * @return The total count including keys and associated BMP groups.
  */
 int scenario__compute_animal_size() {
     int counter = 0;
     for (const auto &[key, bmp_group] : animal_complete_) {
         ++counter;
         for (const auto &bmp : bmp_group) {
             ++counter;
         }
     }
     return counter;
 }
 
 /**
  * @brief Computes the modified alpha value (minus part) for a given key.
  *
  * Retrieves the original amount (or uses @p original_amount) and subtracts the
  * stored value from the global map @c amount_minus_ (if present).
  *
  * @param key The key identifying the scenario element.
  * @param original_amount The original amount value.
  * @return The adjusted value: original amount minus the stored minus value.
  */
 double scenario__alpha_minus2(const std::string& key, double original_amount) {
     double minus = 0.0;
     double original = original_amount;
     if (amount_minus_.contains(key)) {
         minus = amount_minus_.at(key);
     }
     return original - minus;
 }
 
 /**
  * @brief Computes the modified alpha value (plus and minus parts) for a given key.
  *
  * This function adds any stored plus value (from @c amount_plus_) to the result of
  * @ref scenario__alpha_minus2.
  *
  * @param key The key identifying the scenario element.
  * @param original_amount The original amount value.
  * @return The adjusted alpha value.
  */
 double scenario__alpha_plus_minus2(const std::string& key, double original_amount) {
     double plus = 0.0;
     if (amount_plus_.contains(key)) {
         plus = amount_plus_.at(key);
     }
     return plus + scenario__alpha_minus2(key, original_amount);
 }
 
 /**
  * @brief Computes and prints land conversion data.
  *
  * Iterates over the global @c lc_x_ vector and prints formatted information.
  */
 void scenario__compute_lc2() {
     for (const auto& entry : lc_x_) {
         auto [s, h, u, bmp, amount] = entry;
         std::cout << fmt::format("S: {}, h: {}, u: {}, bmp: {}, amount: {}\n", s, h, u, bmp, amount);
     }
 }
 
 /**
  * @brief Sums acreage values into the given map.
  *
  * If the key already exists in @p am, its value is incremented by @p acreage.
  * Otherwise, a new entry is created.
  *
  * @param am The map in which to sum values.
  * @param key The key for which to sum the acreage.
  * @param acreage The acreage value to add.
  */
 void scenario__sum_alpha2(std::unordered_map<std::string, double>& am,  const std::string& key, double acreage) {
     double tmp = 0.0;
     if (am.contains(key)) {
         tmp = am.at(key);
     }
     tmp += acreage;
     am[key] = tmp;
 }
 
 /**
  * @brief Loads alpha values into the given vector.
  *
  * For each parcel (based on @c bmp_grp_src_links_vec and @c s_h_u_vec), the function computes
  * a new alpha value using @ref scenario__alpha_plus_minus2 and appends it to @p my_alpha.
  *
  * @param[out] my_alpha The vector to store computed alpha values.
  */
 void scenario__load_alpha2(std::vector<double>& my_alpha) {
     size_t parcel_idx = 0;
     my_alpha.clear();
     for (auto &&bmp_per_parcel : bmp_grp_src_links_vec) {
         int s = s_h_u_vec[parcel_idx][0];
         int h = s_h_u_vec[parcel_idx][1];
         int u = s_h_u_vec[parcel_idx][2];
         auto from_key = fmt::format("{}_{}_{}", s, h, u);
         double new_alpha = scenario__alpha_plus_minus2(from_key, alpha_stored_vec[parcel_idx]);
         my_alpha.push_back(new_alpha);
         ++parcel_idx;
     }
 }
 
 /**
  * @brief Normalizes animal-related variables.
  *
  * Uses the input vector @p x to compute a normalized cost for animal variables.
  * Also populates the global @c animal_x_ vector.
  *
  * @param x The vector of decision variable values.
  * @return The total cost computed.
  */
 double scenario__normalize_animal2(const std::vector<double>& x) {
     int counter = animal_begin_;
     double total_cost = 0.0;
     animal_x_.clear();
     for (const std::string& key : animal_keys_) {
         std::vector<int> bmp_group = animal_complete_[key];
         std::vector<std::pair<double, int>> grp_tmp;
         std::vector<std::string> key_split;
         split_str(key, '_', key_split);
         std::string base_condition = key_split[0];
         std::string county = key_split[1];
         std::string load_source = key_split[2];
         std::string animal_id = key_split[3];
         double sum = x[counter];
         ++counter;
         for (int bmp : bmp_group) {
             grp_tmp.push_back({x[counter], bmp});
             sum += x[counter];
             ++counter;
         }
         double pct_accum = 0.0;
         fmt::print("grp_tmp: {}\n", grp_tmp.size());
         for (auto [pct, bmp] : grp_tmp) {
             double norm_pct = pct / sum;
             if (norm_pct * animal_[key] > 1.0) {
                 double amount = (norm_pct * animal_[key]);
                 std::string key_bmp_cost = fmt::format("{}_{}", 8, bmp);
                 double cost = amount * bmp_cost_[key_bmp_cost];
                 total_cost += cost;
                 animal_x_.push_back({std::stoi(base_condition), std::stoi(county), std::stoi(load_source), std::stoi(animal_id), bmp, amount});
             }
         }
     }
     return total_cost;
 }
 
 /**
  * @brief Normalizes land conversion variables.
  *
  * Uses the input vector @p x to compute a normalized cost for land conversion variables.
  * Also updates global maps @c amount_minus_ and @c amount_plus_ and the vector @c lc_x_.
  *
  * @param x The vector of decision variable values.
  * @return The total cost computed.
  */
 double scenario__normalize_land_conversion2(const std::vector<double>& x) {
     int counter = lc_begin_;
     amount_minus_.clear();
     amount_plus_.clear();
     lc_x_.clear();
     double total_cost = 0.0;
     for (const auto& key : lc_keys_) {
         std::vector<std::string> bmp_group = land_conversion_from_bmp_to[key];
         std::vector<std::pair<double, std::string>> grp_tmp;
         std::vector<std::string> key_split;
         double sum = x[counter];
         split_str(key, '_', key_split);
         ++counter;
         for (std::string bmp : bmp_group) {
             grp_tmp.push_back({x[counter], bmp});
             sum += x[counter];
             ++counter;
         }
         double pct_accum = 0.0;
         std::vector<std::pair<double, std::string>> grp_pct_tmp;
         for (auto line : grp_tmp) {
             double pct = line.first;
             std::string to = line.second;
             double norm_pct = pct / sum;
             std::vector<std::string> out_to;
             split_str(to, '_', out_to);
             auto bmp = std::stoi(out_to[0]);
             auto key_to = fmt::format("{}_{}_{}", key_split[0], key_split[1], out_to[1]);
             scenario__sum_alpha2(amount_plus_, key_to, norm_pct * amount_[key]);
             pct_accum += norm_pct;
             if (norm_pct * amount_[key] > 1.0) {
                 double amount = (norm_pct * amount_[key]);
                 auto key_bmp_cost = fmt::format("{}_{}", 8, bmp);
                 double cost = amount * bmp_cost_[key_bmp_cost];
                 total_cost += cost;
                 lc_x_.push_back({std::stoi(key_split[0]), std::stoi(key_split[1]), std::stoi(key_split[2]), bmp, norm_pct * amount_[key]});
             }
         }
         scenario__sum_alpha2(amount_minus_, key, pct_accum * amount_[key]);
     }
     return total_cost;
 }
 
 /**
  * @brief Main entry point for the NSGA-III routine.
  *
  * This function parses command-line arguments, initializes the population,
  * loads scenario/configuration data, executes the evolutionary loop and finally
  * reports and writes the final population and related statistics to output files.
  *
  * @param argc Number of command-line arguments.
  * @param argv Array of command-line arguments.
  * @return 0 on success.
  */
 int main(int argc, char **argv)
 {
     int i, j;
     FILE *fpt1;
     FILE *fpt2;
     FILE *fpt3;
     FILE *fpt4;
     FILE *fpt5;
     FILE *fpt6;
     FILE *gp = NULL;
     population *parent_pop;
     population *child_pop;
     population *mixed_pop;
     std::string algorithm = "nsga2";
     n_injected_points = 0;
 
     if (argc < 2)
     {
         std::clog << "Usage ./nsga3r random_seed UUID  [corecast | mathmodel | corecast+smartinit | mathmodel+smartinit] #injected_points injected_points_filename\n";
         std::clog << "Usage Example: ./nsga3r 0.1 00000000-0000-0000-0000-000000000075 mathmodel 11 injected_points/tucker_vw_nds.in < ../input_data/tucker_vw.in\n";
         exit(1);
     }
     seed = (double) std::stof(argv[1]);
     emo_uuid = argv[2];
     std::string eval_options[] = {"corecast", "mathmodel", "corecast+smartinit", "mathmodel+smartinit"};
     opt4cast_evaluation = argv[3];
     n_injected_points = std::stoi(argv[4]);
     injected_points_filename = argv[5];
     popsize = std::stoi(argv[6]);
     ngen = std::stoi(argv[7]);
     mode = std::stoi(argv[8]);
     std::string scenario_filename = argv[9];
     double ipopt_reduction = std::stof(argv[10]);
     double ipopt_popsize = std::stoi(argv[11]);
     int corecast_gen = std::stoi(argv[12]);
     int cost_profile_idx = std::stoi(argv[13]);
     int pollutant_idx = 0;
     double limit_alpha = 1.0;
 
     scenario__load("/opt/opt4cast/csvs/prueba.json");
     std::string msu_cbpo_path = getEnvVar("MSU_CBPO_PATH", "/opt/opt4cast");
     if (find(cbegin(eval_options), cend(eval_options), opt4cast_evaluation) == cend(eval_options)) {
         std::cerr << "Invalid option: " << opt4cast_evaluation << ", please use [send | retrieve]\n";
         return -1;
     }
     if (opt4cast_evaluation == "corecast+smartinit" || opt4cast_evaluation == "mathmodel+smartinit") {
         if (injected_points_filename == "./") {
             injected_points_filename = fmt::format("{}/output/nsga3/{}/config/ipopt.json", msu_cbpo_path, emo_uuid);
         }
         if (!fs::exists(injected_points_filename)) {
             int ipopt = 1;
             std::string env_var = "OPT4CAST_EPS_CNSTR_PATH";
             std::string EPS_CNSTR_PATH = getEnvVar(env_var);
             std::string exec_string = fmt::format("{} {} {} {} {} {} {} {}",
                                                   EPS_CNSTR_PATH, emo_uuid, ipopt_reduction, pollutant_idx, ipopt, limit_alpha, cost_profile_idx, ipopt_popsize);
             using namespace my_execute;
             CommandResult nullbyteCommand = Command::exec(exec_string);
             std::ofstream ofile("/tmp/filename2.txt");
             ofile << exec_string << std::endl;
             ofile << "Output using fread: " << nullbyteCommand << std::endl;
             ofile.close();
         }
         std::string dir_path = fmt::format("{}/output/nsga3/{}/front", msu_cbpo_path, emo_uuid);
         if (!fs::exists(dir_path))
             fs::create_directories(dir_path);
         std::string filename_src = fmt::format("{}/output/nsga3/{}/config/ipopt.json", msu_cbpo_path, emo_uuid);
         std::string filename_dst = fmt::format("{}/output/nsga3/{}/front/epsilon.json", msu_cbpo_path, emo_uuid);
         if (fs::exists(filename_src))
             fs::copy(filename_src, filename_dst, fs::copy_options::update_existing);
     }
     else {
         n_injected_points = 0;
         injected_points_filename = "";
     }
 
     int my_new_variables = 0;
     int animal_variables = 0;
     if (is_lc_enabled) {
         my_new_variables = scenario__compute_lc_size();
     }
     if (is_animal_enabled) {
         animal_variables = scenario__compute_animal_size();
     }
     std::cout << "My new variables: " << my_new_variables << std::endl;
     if (seed <= 0.0 || seed >= 1.0)
     {
         printf("\n Entered seed value is wrong, seed value must be in (0,1) \n");
         exit(1);
     }
     try {
         std::string dir_path = fmt::format("{}/output/nsga3/{}", msu_cbpo_path, emo_uuid);
         std::filesystem::create_directories(dir_path);
     }
     catch(const std::exception & error) {
         std::cout << "error\n" << error.what() << "\n";
         std::clog << fmt::format("when saving files last generation") << error.what();
     }
     try {
         std::string dir_path = fmt::format("{}/output/nsga3/{}/front", msu_cbpo_path, emo_uuid);
         std::filesystem::create_directories(dir_path);
     }
     catch(const std::exception & error) {
         std::cout << "error\n" << error.what() << "\n";
         std::clog << fmt::format("when saving files last generation") << error.what();
     }
     std::string stored_path = fmt::format("{}/output/nsga3/{}/front/", msu_cbpo_path, emo_uuid);
     std::string specific_path = stored_path + "initial_pop.out";
     fpt1 = fopen(specific_path.c_str(), "w");
     specific_path = stored_path + "final_pop.out";
     fpt2 = fopen(specific_path.c_str(), "w");
     specific_path = stored_path + "best_pop.out";
     fpt3 = fopen(specific_path.c_str(), "w");
     specific_path = stored_path + "all_pop.out";
     fpt4 = fopen(specific_path.c_str(), "w");
     specific_path = stored_path + "params.out";
     fpt5 = fopen(specific_path.c_str(), "w");
     specific_path = stored_path + "ref.out";
     fpt6 = fopen(specific_path.c_str(), "w");
     fprintf(fpt1, "# This file contains the data of initial population\n");
     fprintf(fpt2, "# This file contains the data of final population\n");
     fprintf(fpt3, "# This file contains the data of final feasible population (if found)\n");
     fprintf(fpt4, "# This file contains the data of all generations\n");
     fprintf(fpt5, "# This file contains information about inputs as read by the program\n");
 
     scanf("%d", &nobj);
     printf("nobj: %d\n", nobj);
     if (nobj < 1)
     {
         printf("\n number of objectives entered is : %d", nobj);
         printf("\n Wrong number of objectives entered, hence exiting \n");
         exit(1);
     }
     scanf("%d", &steps);
     printf("steps: %d\n", steps);
     steps = (int) popsize * 0.90;
     if (steps < 0)
     {
         printf("\n number of steps entered is : %d", steps);
         printf("\n Wrong number of steps entered, hence exiting \n");
         exit(1);
     }
     scaling = (double)steps;
     if (steps == 0)
     {
         scanf("%d", &nref);
         onref = nref;
         scaling = 20.0;
     }
     scanf("%d", &adaptive_increment);
     printf("adaptive_increment: %d\n", adaptive_increment);
     adaptive_increment = 1;
     int my_popsize;
     scanf("%d", &my_popsize);
     printf("my_popsize: %d\n", my_popsize);
     if (popsize < 4 || (popsize % 4) != 0)
     {
         std::clog << fmt::format("\n population size read is : {}\n", popsize);
         std::clog << "\n Wrong population size entered, hence exiting \n";
         exit(1);
     }
     int my_ngen;
     scanf("%d", &my_ngen);
     printf("my_ngen: %d\n", my_ngen);
     scanf("%d", &ncon);
     printf("ncon: %d\n", ncon);
     ncon++;
     if (ncon < 0)
     {
         std::clog << fmt::format("\n number of constraints entered is : {}\n", ncon);
         std::clog << "Wrong number of constraints enetered, hence exiting \n";
         exit(1);
     }
     scanf("%d", &nreal);
     printf("nreal: %d\n", nreal);
     ef_size_ = nreal;
     ef_begin_ = 0;
     ef_end_ = ef_begin_ + ef_size_ - 1;
     if (is_lc_enabled) {
         lc_size_ = my_new_variables;
         lc_begin_ = nreal;
         lc_end_ = lc_begin_ + lc_size_ - 1;
     }
     if (is_animal_enabled) {
         animal_size_ = animal_variables;
         animal_begin_ = ef_size_ + lc_size_;
         animal_end_ = animal_begin_ + animal_size_ - 1;
     }
     nreal += (my_new_variables + animal_variables);
     if (nreal < 0)
     {
         std::clog << fmt::format("\n number of real variables entered is : {}\n", nreal);
         std::clog << "Wrong number of variables entered, hence exiting \n";
         exit(1);
     }
     if (nreal != 0)
     {
         min_realvar = (double *)malloc(nreal * sizeof(double));
         max_realvar = (double *)malloc(nreal * sizeof(double));
         for (i = 0; i < nreal; i++)
         {
             min_realvar[i] = 0.0;
             max_realvar[i] = 1.0;
             if (max_realvar[i] <= min_realvar[i])
             {
                 std::clog << "\n Wrong limits entered for the min and max bounds of real variable, hence exiting \n";
                 exit(1);
             }
         }
         scanf("%lf", &pcross_real);
         printf("pcross_real: %lf\n", pcross_real);
         if (pcross_real < 0.0 || pcross_real > 1.0)
         {
             std::clog << fmt::format("\n Probability of crossover entered is : {}\n", pcross_real);
             std::clog << "Entered value of probability of crossover of real variables is out of bounds, hence exiting \n";
             exit(1);
         }
         pmut_real = 1.0 / (double)nreal;
         if (pmut_real < 0.0 || pmut_real > 1.0)
         {
             std::clog << fmt::format("\n Probability of mutation entered is : {}\n", pmut_real);
             std::clog << "Entered value of probability of mutation of real variables is out of bounds, hence exiting \n";
             exit(1);
         }
         scanf("%lf", &eta_c);
         printf("eta_c: %lf\n", eta_c);
         if (eta_c <= 0)
         {
             std::clog << fmt::format("\n The value entered is : {}\n", eta_c);
             std::clog << "Wrong value of distribution index for crossover entered, hence exiting \n";
             exit(1);
         }
         scanf("%lf", &eta_m);
         printf("eta_m: %lf\n", eta_m);
         if (eta_m <= 0)
         {
             std::clog << fmt::format("\n The value entered is : %e\n", eta_m);
             std::clog << "Wrong value of distribution index for mutation entered, hence exiting \n";
             exit(1);
         }
     }
     scanf("%d", &nbin);
     printf("nbin: %d\n", nbin);
     if (nbin != 0)
     {
         std::clog << fmt::format("Number of binary variables entered is : {}\n", nbin);
         std::clog << "Wrong number of binary variables entered, hence exiting \n";
         exit(1);
     }
     if (nbin != 0)
     {
         nbits = (int *)malloc(nbin * sizeof(int));
         min_binvar = (double *)malloc(nbin * sizeof(double));
         max_binvar = (double *)malloc(nbin * sizeof(double));
         for (i = 0; i < nbin; i++)
         {
             scanf("%d", &nbits[i]);
             if (nbits[i] < 1)
             {
                 std::clog << "\n Wrong number of bits for binary variable entered, hence exiting\n";
                 exit(1);
             }
             scanf("%lf", &min_binvar[i]);
             scanf("%lf", &max_binvar[i]);
             if (max_binvar[i] <= min_binvar[i])
             {
                 std::clog << "Wrong limits entered for the min and max bounds of binary variable entered, hence exiting\n";
                 exit(1);
             }
         }
         scanf("%lf", &pcross_bin);
         if (pcross_bin < 0.0 || pcross_bin > 1.0)
         {
             std::clog << fmt::format("Probability of crossover entered is : {}\n", pcross_bin);
             std::clog << "Entered value of probability of crossover of binary variables is out of bounds, hence exiting \n";
             exit(1);
         }
         scanf("%lf", &pmut_bin);
         if (pmut_bin < 0.0 || pmut_bin > 1.0)
         {
             std::clog << fmt::format("Probability of mutation entered is : %e\n", pmut_bin);
             std::clog << "Entered value of probability  of mutation of binary variables is out of bounds, hence exiting \n";
             exit(1);
         }
     }
     if (nreal == 0 && nbin == 0)
     {
         std::clog << "\n Number of real as well as binary variables, both are zero, hence exiting\n";
         exit(1);
     }
     choice = 0;
     scanf("%d", &choice);
     if (choice != 0 && choice != 1)
     {
         std::clog << fmt::format("Entered the wrong choice, hence exiting, choice entered was {}\n", choice);
         exit(1);
     }
     if (choice == 1)
     {
         gp = popen(GNUPLOT_COMMAND, "w");
         if (gp == NULL)
         {
             std::clog << "Could not open a pipe to gnuplot, check the definition of GNUPLOT_COMMAND in file global.h\n";
             std::clog << "Edit the string to suit your system configuration and rerun the program\n";
             exit(1);
         }
         if (nobj == 2)
         {
             scanf("%d", &obj1);
             if (obj1 < 1 || obj1 > nobj)
             {
                 std::clog << fmt::format("Wrong value of X objective entered, value entered was {}\n", obj1);
                 exit(1);
             }
             scanf("%d", &obj2);
             if (obj2 < 1 || obj2 > nobj)
             {
                 std::clog << fmt::format("\n Wrong value of Y objective entered, value entered was {}\n", obj2);
                 exit(1);
             }
             obj3 = -1;
         }
         else
         {
             scanf("%d", &choice);
             if (choice != 2 && choice != 3)
             {
                 std::clog << fmt::format("Entered the wrong choice, hence exiting, choice entered was {}\n", choice);
                 exit(1);
             }
             if (choice == 2)
             {
                 scanf("%d", &obj1);
                 if (obj1 < 1 || obj1 > nobj)
                 {
                     std::clog << fmt::format("\n Wrong value of X objective entered, value entered was {}\n", obj1);
                     exit(1);
                 }
                 scanf("%d", &obj2);
                 if (obj2 < 1 || obj2 > nobj)
                 {
                     std::clog << fmt::format("\n Wrong value of Y objective entered, value entered was {}\n", obj2);
                     exit(1);
                 }
                 obj3 = -1;
             }
             else
             {
                 scanf("%d", &obj1);
                 if (obj1 < 1 || obj1 > nobj)
                 {
                     std::clog << fmt::format("\n Wrong value of X objective entered, value entered was {}\n", obj1);
                     exit(1);
                 }
                 scanf("%d", &obj2);
                 if (obj2 < 1 || obj2 > nobj)
                 {
                     std::clog << fmt::format("Wrong value of Y objective entered, value entered was {}\n", obj2);
                     exit(1);
                 }
                 scanf("%d", &obj3);
                 if (obj3 < 1 || obj3 > nobj)
                 {
                     std::clog << fmt::format("Wrong value of Z objective entered, value entered was {}\n", obj3);
                     exit(1);
                 }
                 scanf("%d", &angle1);
                 if (angle1 < 0 || angle1 > 180)
                 {
                     std::clog << "Wrong value for first angle entered, hence exiting\n";
                     exit(1);
                 }
                 scanf("%d", &angle2);
                 if (angle2 < 0 || angle2 > 360)
                 {
                     std::clog << "Wrong value for second angle entered, hence exiting\n";
                     exit(1);
                 }
             }
         }
     }
     if (mode > 0) {
         popsize = 1;
         ngen = 1;
     }
     fprintf(fpt5, "\n Population size = %d", popsize);
     fprintf(fpt5, "\n Number of generations = %d", ngen);
     fprintf(fpt5, "\n Number of objective functions = %d", nobj);
     fprintf(fpt5, "\n Number of constraints = %d", ncon);
     fprintf(fpt5, "\n Number of real variables = %d", nreal);
     if (nreal != 0)
     {
         for (i = 0; i < nreal; i++)
         {
             fprintf(fpt5, "\n Lower limit of real variable %d = %e", i + 1, min_realvar[i]);
             fprintf(fpt5, "\n Upper limit of real variable %d = %e", i + 1, max_realvar[i]);
         }
         fprintf(fpt5, "\n Probability of crossover of real variable = %e", pcross_real);
         fprintf(fpt5, "\n Probability of mutation of real variable = %e", pmut_real);
         fprintf(fpt5, "\n Distribution index for crossover = %e", eta_c);
         fprintf(fpt5, "\n Distribution index for mutation = %e", eta_m);
     }
     fprintf(fpt5, "\n Number of binary variables = %d", nbin);
     if (nbin != 0)
     {
         for (i = 0; i < nbin; i++)
         {
             fprintf(fpt5, "\n Number of bits for binary variable %d = %d", i + 1, nbits[i]);
             fprintf(fpt5, "\n Lower limit of binary variable %d = %e", i + 1, min_binvar[i]);
             fprintf(fpt5, "\n Upper limit of binary variable %d = %e", i + 1, max_binvar[i]);
         }
         fprintf(fpt5, "\n Probability of crossover of binary variable = %e", pcross_bin);
         fprintf(fpt5, "\n Probability of mutation of binary variable = %e", pmut_bin);
     }
     fprintf(fpt5, "\n Seed for random number generator = %e", seed);
     fprintf(fpt5, "\n Approach %d used for reference points", adaptive_increment);
     bitlength = 0;
     if (nbin != 0)
     {
         for (i = 0; i < nbin; i++)
         {
             bitlength += nbits[i];
         }
     }
     fprintf(fpt1, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
     fprintf(fpt2, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
     fprintf(fpt3, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
     fprintf(fpt4, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
     nbinmut = 0;
     nrealmut = 0;
     nbincross = 0;
     nrealcross = 0;
     start_incr = 0;
     for (i = 0; i < 10; i++)
         active_ref_pts[i] = -1000000;
     parent_pop = (population *)malloc(sizeof(population));
     child_pop = (population *)malloc(sizeof(population));
     mixed_pop = (population *)malloc(sizeof(population));
     allocate_memory_pop(parent_pop, popsize);
     allocate_memory_pop(child_pop, popsize);
     allocate_memory_pop(mixed_pop, 2 * popsize);
     ideal_point = (double *)malloc(nobj * sizeof(double));
     plane_point = (double **)malloc(nobj * sizeof(double *));
     for (i = 0; i < nobj; i++)
         plane_point[i] = (double *)malloc(nobj * sizeof(double));
     for (i = 0; i < NMAX; i++)
         ref_pt[i] = (double *)malloc(nobj * sizeof(double));
     create_ref_points(steps);
     randomize();
     read_global_files(emo_uuid.c_str(), pollutant_id, cost_profile_idx);
     int nvars, nconstraints;
     get_info(nvars, nconstraints, fmt::format("{}/output/nsga3/config/", msu_cbpo_path));
     ++nconstraints;
     std::cout << fmt::format("{}/output/nsga3/config/", msu_cbpo_path) << emo_uuid << "\n";
     initialize_pop(parent_pop);
     if (mode > 0) {
         for (int _i(0); _i < nvars; ++_i) {
             parent_pop->ind[0].xreal[_i] = 0.0;
         }
         CSVReader my_reader(scenario_filename);
         std::vector<std::vector<std::string>> dataList = my_reader.getData();
         int _s, _h, _u, _b;
         double _a;
         for (int _i(0); _i < (int)dataList.size(); ++_i) {
             _s = std::stoi(dataList[_i][0]);
             _h = std::stoi(dataList[_i][1]);
             _u = std::stoi(dataList[_i][2]);
             _b = std::stoi(dataList[_i][3]);
             _a = std::stod(dataList[_i][4]);
             std::string s_tmp = fmt::format("{}_{}_{}_{}", _s, _h, _u, _b);
             std::cout << "s_tmp is " << s_tmp << "\n";
             int idx = get_s_h_u_b(s_tmp);
             if (idx < 0){
                 std::cout << "GTP Error, no s h u b in the file\n";
             }
             parent_pop->ind[0].xreal[idx] = _a;
         }
     }
     std::clog << "Initialization done, now performing first generation\n";
     curr_gen = 1;
     decode_pop(parent_pop);
     evaluate_pop(parent_pop, curr_gen, ngen, corecast_gen, true, curr_gen);
     assign_rank(parent_pop);
     find_ideal_point(parent_pop);
     elitist_sorting(parent_pop, parent_pop);
     report_pop(parent_pop, fpt1);
     fprintf(fpt4, "# gen = 1\n");
     report_pop(parent_pop, fpt4);
     std::cout << "gen = 1\n";
     fflush(stdout);
     fflush(fpt1);
     fflush(fpt2);
     fflush(fpt3);
     fflush(fpt4);
     fflush(fpt5);
     sleep(1);
     bool reevaluate_flag = true;
     for (i = 2; i <= ngen; i++)
     {
         curr_gen = i;
         if (curr_gen >= corecast_gen && reevaluate_flag == true) {
             reevaluate_flag = false;
             evaluate_pop(parent_pop, curr_gen, ngen, corecast_gen, true, curr_gen-1);
         }
         selection(parent_pop, child_pop);
         mutation_pop(child_pop);
         decode_pop(child_pop);
         evaluate_pop(child_pop, curr_gen, ngen, corecast_gen, false, curr_gen);
         merge(parent_pop, child_pop, mixed_pop);
         assign_rank_mixedpop(mixed_pop);
         update_ideal_point(child_pop);
         if (algorithm == "nsga3") {
             elitist_sorting(mixed_pop, parent_pop);
         }
         else {
             fill_nondominated_sort(mixed_pop, parent_pop);
         }
         fprintf(fpt4, "# gen = %d\n", i);
         if (choice != 0 && i == ngen)
             onthefly_display(parent_pop, gp, i);
         onthefly_display2(parent_pop, i);
         std::cout << fmt::format("gen = {}\n", i);
     }
     if (mode > 0) {
         onthefly_display(parent_pop, gp, 1);
         std::string outcome = fmt::format("{}/output/nsga3/{}/front/outcome.txt", msu_cbpo_path, emo_uuid);
         std::ofstream outcome_f(outcome);
         outcome_f.precision(10);
         outcome_f << fmt::format("{:.2f},{:.2f}\n", parent_pop->ind[0].obj[0], parent_pop->ind[0].obj[1]);
         outcome_f.close();
     }
     std::cout << "Generations finished, now reporting solutions\n";
     for (i = 0; i < nref; i++)
     {
         for (j = 0; j < nobj; j++)
             fprintf(fpt6, "%f\t", ref_pt[i][j]);
         fprintf(fpt6, "\n");
     }
     report_pop(parent_pop, fpt2);
     report_feasible(parent_pop, fpt3);
     if (nreal != 0)
     {
         fprintf(fpt5, "\n Number of crossover of real variable = %d", nrealcross);
         fprintf(fpt5, "\n Number of mutation of real variable = %d", nrealmut);
     }
     if (nbin != 0)
     {
         fprintf(fpt5, "\n Number of crossover of binary variable = %d", nbincross);
         fprintf(fpt5, "\n Number of mutation of binary variable = %d", nbinmut);
     }
     fprintf(fpt5, "\n Number of reference points initially = %d", onref);
     fprintf(fpt5, "\n Number of reference points finally = %d", nref);
     fflush(stdout);
     fflush(fpt1);
     fflush(fpt2);
     fflush(fpt3);
     fflush(fpt4);
     fflush(fpt5);
     fflush(fpt6);
     fclose(fpt1);
     fclose(fpt2);
     fclose(fpt3);
     fclose(fpt4);
     fclose(fpt5);
     fclose(fpt6);
     if (choice != 0)
     {
         pclose(gp);
     }
     if (nreal != 0)
     {
         free(min_realvar);
         free(max_realvar);
     }
     if (nbin != 0)
     {
         free(min_binvar);
         free(max_binvar);
         free(nbits);
     }
     for (i = 0; i < nobj; i++)
     {
         free(plane_point[i]);
     }
     for (i = 0; i < NMAX; i++)
     {
         free(ref_pt[i]);
     }
     free(plane_point);
     free(ideal_point);
     deallocate_memory_pop(parent_pop, popsize);
     deallocate_memory_pop(child_pop, popsize);
     deallocate_memory_pop(mixed_pop, 2 * popsize);
     free(parent_pop);
     free(child_pop);
     free(mixed_pop);
     std::cout << "\n Routine successfully exited \n";
     return (0);
 }
 