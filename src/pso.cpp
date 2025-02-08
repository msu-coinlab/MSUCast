/**
 * @file pso.cpp
 * @brief Implementation of the Particle Swarm Optimization (PSO) algorithm and related helper functions.
 *
 * This file implements the PSO algorithm for multi-objective optimization along with functions for
 * reading cost files, determining Pareto fronts, saving results to CSV, and performing post‐processing
 * using an external optimizer (Ipopt). Helper functions for file operations and JSON/scenario parsing are
 * also included.
 *
 * Dependencies: fmt, nlohmann::json, Boost, range-v3, crossguid, and various STL libraries.
 */

 #include <stdio.h>
 #include <iostream>
 
 #include <iostream>
 #include <vector>
 #include <random>
 #include <algorithm>
 #include <functional>
 #include <numeric>
 #include <cmath>
 #include <functional> // For std::reference_wrapper
 #include <fstream>
 
 #include <range/v3/all.hpp>
 
 #include "external_archive.h"
 #include "particle.h"
 #include "pso.h"
 #include "scenario.h"
 #include "misc_utilities.h"
 
 #include <crossguid/guid.hpp>
 #include <fmt/core.h>
 #include <regex>
 #include <filesystem>
 #include <boost/algorithm/string.hpp>
 #include <optional>
 #include <fstream>
 #include <algorithm>
 
 #include <nlohmann/json.hpp>
 
 //#include "spdlog/spdlog.h"
 //#include "spdlog/sinks/stdout_color_sinks.h"
 
 namespace fs = std::filesystem;
 using json = nlohmann::json;
 
 /***************************************************************************/
 /**
  * @brief Data structure to hold cost information for a solution.
  */
 struct CostData {
     double objective1;               ///< Value for objective 1.
     double objective2;               ///< Value for objective 2.
     std::optional<double> objective3;///< Optional third objective.
     std::string file_index;          ///< Identifier (index) for the file/solution.
 };
 
 /**
  * @brief Checks if solution A dominates solution B.
  *
  * For two objectives, A dominates B if both objective values are less than or equal and at least one is strictly less.
  * For three objectives, domination is checked only if both solutions have a value for the third objective.
  *
  * @param a The first CostData solution.
  * @param b The second CostData solution.
  * @param num_objectives Number of objectives (2 or 3).
  * @return true if a dominates b, false otherwise.
  */
 bool dominates(const CostData& a, const CostData& b, int num_objectives) {
     if (num_objectives == 2) {
         return (a.objective1 <= b.objective1 && a.objective2 <= b.objective2) &&
                (a.objective1 < b.objective1 || a.objective2 < b.objective2);
     } else if (num_objectives == 3 && a.objective3.has_value() && b.objective3.has_value()) {
         return (a.objective1 <= b.objective1 && a.objective2 <= b.objective2 && a.objective3.value() <= b.objective3.value()) &&
                (a.objective1 < b.objective1 || a.objective2 < b.objective2 || a.objective3.value() < b.objective3.value());
     }
     return false;
 }
 
 /**
  * @brief Finds the Pareto front among a set of cost data.
  *
  * Returns the file indices corresponding to solutions that are non-dominated.
  *
  * @param data Vector of CostData representing the solutions.
  * @param num_objectives Number of objectives in the optimization problem.
  * @return std::vector<std::string> Vector of file indices that form the Pareto front.
  */
 std::vector<std::string> find_pareto_front(const std::vector<CostData>& data, int num_objectives) {
     std::vector<std::string> pareto_front;
 
     for (const auto& current : data) {
         bool is_dominated = false;
         for (const auto& other : data) {
             if (dominates(other, current, num_objectives)) {
                 is_dominated = true;
                 break;
             }
         }
         if (!is_dominated) {
             pareto_front.push_back(current.file_index);
         }
     }
     return pareto_front;
 }
 
 /**
  * @brief Reads cost data from JSON files in a directory.
  *
  * The function iterates over the directory, looking for filenames ending with "_costs.json",
  * parses each JSON file, and extracts the cost values for each objective.
  *
  * @param objs A vector of objective names to extract.
  * @param directory The directory to search for cost files.
  * @return std::vector<CostData> Vector of cost data extracted from the files.
  */
 std::vector<CostData> readCostFiles(const std::vector<std::string>& objs, const std::string& directory) {
     std::vector<CostData> data;
     int num_objectives = objs.size();
 
     for (const auto& entry : fs::directory_iterator(directory)) {
         std::string filename = entry.path().filename().string();
         if (filename.ends_with("_costs.json")) {
             std::ifstream file(entry.path());
             json j;
             file >> j;
 
             CostData cost_data;
             cost_data.objective1 = j[objs[0]];
             cost_data.objective2 = j[objs[1]];
             if (num_objectives == 3) {
                 cost_data.objective3 = j[objs[2]];
             } else {
                 cost_data.objective3 = std::nullopt;
             }
             // Extract and store everything before the first underscore
             cost_data.file_index = filename.substr(0, filename.find('_'));
 
             data.push_back(cost_data);
         }
     }
 
     return data;
 }
 
 /**
  * @brief Finds Pareto front files by reading cost data and determining non-dominated solutions.
  *
  * @param objs A vector of objective names.
  * @param directory The directory containing the cost files.
  * @return std::vector<std::string> A vector of file indices that are on the Pareto front.
  */
 std::vector<std::string> findParetoFrontFiles(const std::vector<std::string>& objs, const std::string& directory) {
     int num_objectives = objs.size();
     std::vector<CostData> data = readCostFiles(objs, directory);
     return find_pareto_front(data, num_objectives);
 }
 
 /**
  * @brief Writes cost data to a CSV file.
  *
  * The function sorts the cost data by file index (converted to a number) and writes the
  * objectives to the CSV file.
  *
  * @param data Vector of CostData to write.
  * @param output_file The path of the CSV output file.
  * @param objs A vector of objective names (for header purposes).
  */
 void writeCSV(const std::vector<CostData>& data, const std::string& output_file, const std::vector<std::string>& objs) {
     std::vector<CostData> sorted_data = data;
     std::sort(sorted_data.begin(), sorted_data.end(), [](const CostData& a, const CostData& b) {
         return std::stol(a.file_index) < std::stol(b.file_index);
     });
     std::ofstream csv_file(output_file);
 
     // Write the data with fixed precision
     csv_file << std::fixed << std::setprecision(6);
     for (const auto& entry : sorted_data) {
         csv_file << entry.objective1 << "," << entry.objective2;
         if (entry.objective3.has_value()) {
             csv_file << "," << entry.objective3.value();
         }
         csv_file << "\n";
     }
     csv_file.close();
 }
 
 /***************************************************************************/
 /**
  * @brief Anonymous namespace containing internal helper functions.
  */
 namespace {
 
     /**
      * @brief Replaces the ending substring of a given string.
      *
      * If the string ends with oldEnding, it is replaced by newEnding.
      *
      * @param str The original string.
      * @param oldEnding The substring to replace.
      * @param newEnding The new substring.
      * @return std::string The modified string.
      */
     std::string replace_ending(const std::string& str, const std::string& oldEnding, const std::string& newEnding) {
         if (str.ends_with(oldEnding)) {
             return str.substr(0, str.size() - oldEnding.size()) + newEnding;
         }
         return str;
     }
     
     /**
      * @brief Saves a 2D vector of doubles to a file.
      *
      * Each row is written on a new line with space-separated values.
      *
      * @param data The 2D vector of doubles.
      * @param filename The output filename.
      */
     void save(const std::vector<std::vector<double>>& data, const std::string& filename) {
         std::ofstream outFile(filename);
         
         if (!outFile) {
             std::cerr << "Failed to open the file: " << filename << ".\n";
             return;
         }
         
         for (const auto& row : data) {
             for (const auto& val : row) {
                 outFile << val << ' ';
             }
             outFile << '\n';
         }
         outFile.close();
     }
 
     /**
      * @brief Saves Particle data to two separate files.
      *
      * The first file receives the position vectors (x) and the second the objective values (fx).
      *
      * @param data Vector of Particle objects.
      * @param filename The filename for saving the positions.
      * @param filename_fx The filename for saving the objective values.
      */
     void save(const std::vector<Particle>& data, const std::string& filename, const std::string& filename_fx) {
         std::ofstream outFile(filename);
         if (!outFile) {
             std::cerr << "Failed to open the file: " << filename << ".\n";
             return;
         }
         
         for (const auto& row : data) {
             for (const auto& val : row.get_x()) {
                 outFile << val << ' ';
             }
             outFile << '\n';
         }
         outFile.close();
 
         std::ofstream out_file_fx(filename_fx);
         if (!out_file_fx) {
             std::cerr << "Failed to open the file: " << filename_fx << ".\n";
             return;
         }
         
         for (const auto& row : data) {
             for (const auto& val : row.get_fx()) {
                 out_file_fx << val << ' ';
             }
             out_file_fx << '\n';
         }
         out_file_fx.close();
     }
 
     /**
      * @brief Reads scenarios from a keyed JSON file and applies an alpha factor.
      *
      * Each scenario is read as a list of parcels. The function uses the key from each parcel,
      * splits it, and looks up an alpha factor from the given dictionary. The amount is scaled by alpha.
      *
      * @param filename The JSON filename.
      * @param alpha_dict A dictionary mapping keys to alpha factors.
      * @return std::vector<std::vector<std::tuple<int, int, int, int, double>>> A vector of scenarios,
      *         each scenario is a vector of tuples representing parcel data.
      */
     std::vector<std::vector<std::tuple<int, int, int, int, double>>> read_scenarios_keyed_json2(std::string filename, const std::unordered_map<std::string, double>& alpha_dict) {
         std::vector<std::vector<std::tuple<int, int, int, int, double>>> scenarios_list;
         std::ifstream file(filename);
         if (!file.is_open()) {
             std::cerr << "Failed to open the file: " << filename << ".\n";
             exit(-1);
         }
     
         json json_obj = json::parse(file);
         for (const auto &scenario_list : json_obj) {
             std::vector<std::tuple<int, int, int, int, double>> parcel_list;
             for (const auto& parcel : scenario_list) {
                 std::vector<std::string> result_vec;
                 auto key = parcel["name"].get<std::string>();
                 boost::split(result_vec, key, boost::is_any_of("_"));
                 auto lrseg = result_vec[0];
                 auto agency = result_vec[1];
                 auto load_src = result_vec[2];
                 auto bmp = result_vec[3];
                 double alpha = 0.0;
                 try {
                     alpha = 1.0; // or use alpha_dict.at(fmt::format("{}_{}_{}", lrseg, agency, load_src));
                 }
                 catch (const std::exception& e) {
                     std::cerr << "Key not found: " << fmt::format("{}_{}_{}", lrseg, agency, load_src) << '\n';
                     for (const auto& [key, val] : alpha_dict) {
                         std::cerr << key << '\n';
                     }
                     std::cerr << e.what() << '\n';
                     exit(-1);
                 }
                 auto amount = parcel["amount"].get<double>() * alpha;
                 parcel_list.emplace_back(std::stoi(lrseg), std::stoi(agency), std::stoi(load_src), std::stoi(bmp), amount);
             }
             scenarios_list.emplace_back(parcel_list);
         }
         return scenarios_list;
     }
 
     /**
      * @brief Reads scenarios from a keyed JSON file (without scaling by 1.0) using an alpha dictionary.
      *
      * Similar to read_scenarios_keyed_json2, but uses the alpha value from the dictionary.
      *
      * @param filename The JSON filename.
      * @param alpha_dict A dictionary mapping keys to alpha factors.
      * @return std::vector<std::vector<std::tuple<int, int, int, int, double>>> A vector of scenarios.
      */
     std::vector<std::vector<std::tuple<int, int, int, int, double>>> read_scenarios_keyed_json(std::string filename, const std::unordered_map<std::string, double>& alpha_dict) {
         std::vector<std::vector<std::tuple<int, int, int, int, double>>> scenarios_list;
         std::ifstream file(filename);
         if (!file.is_open()) {
             std::cerr << "Failed to open the file: " << filename << ".\n";
             exit(-1);
         }
     
         json json_obj = json::parse(file);
         for (const auto &scenario_list : json_obj) {
             std::vector<std::tuple<int, int, int, int, double>> parcel_list;
             for (const auto& parcel : scenario_list) {
                 std::vector<std::string> result_vec;
                 auto key = parcel["name"].get<std::string>();
                 boost::split(result_vec, key, boost::is_any_of("_"));
                 auto lrseg = result_vec[0];
                 auto agency = result_vec[1];
                 auto load_src = result_vec[2];
                 auto bmp = result_vec[3];
                 double alpha = 0.0;
                 try {
                     alpha = alpha_dict.at(fmt::format("{}_{}_{}", lrseg, agency, load_src));
                 }
                 catch (const std::exception& e) {
                     std::cerr << "Key not found: " << fmt::format("{}_{}_{}", lrseg, agency, load_src) << '\n';
                     for (const auto& [key, val] : alpha_dict) {
                         std::cerr << key << '\n';
                     }
                     std::cerr << e.what() << '\n';
                     exit(-1);
                 }
                 auto amount = parcel["amount"].get<double>() * alpha;
                 parcel_list.emplace_back(std::stoi(lrseg), std::stoi(agency), std::stoi(load_src), std::stoi(bmp), amount);
             }
             scenarios_list.emplace_back(parcel_list);
         }
         return scenarios_list;
     }
     
     /**
      * @brief Reads a scenario from a JSON file.
      *
      * The JSON file is expected to contain a mapping from a key to a double (amount) and the key is split into parts.
      *
      * @param filename The JSON filename.
      * @return std::vector<std::tuple<int, int, int, int, double>> A vector of tuples representing the scenario.
      */
     std::vector<std::tuple<int, int, int, int, double>> read_scenario_json(std::string filename) {
         std::vector<std::tuple<int, int, int, int, double>> parcel_list;
     
         std::ifstream file(filename);
         if (!file.is_open()) {
             std::cerr << "Failed to open the file: " << filename << ".\n";
             exit(-1);
         }
     
         json json_obj = json::parse(file);
         auto scenario = json_obj.get<std::unordered_map<std::string, double>>();
         for (const auto& [key, amount] : scenario) {
             std::vector<std::string> result_vec;
             boost::split(result_vec, key, boost::is_any_of("_"));
             parcel_list.emplace_back(std::stoi(result_vec[0]), std::stoi(result_vec[1]), std::stoi(result_vec[2]), std::stoi(result_vec[3]), amount);
         }
     
         return parcel_list;
     }
     
 } // end anonymous namespace
 /***************************************************************************/
 
 /*==================== PSO Class Member Functions ====================*/
 
 /**
  * @brief PSO class constructor.
  *
  * Initializes a PSO instance with the given parameters and then calls init_cast() to perform further setup.
  *
  * @param nparts Number of particles.
  * @param nobjs Number of objectives.
  * @param max_iter Maximum number of iterations.
  * @param w Inertia weight.
  * @param c1 Cognitive coefficient.
  * @param c2 Social coefficient.
  * @param lb Lower bound for decision variables.
  * @param ub Upper bound for decision variables.
  * @param input_filename Path to the input file.
  * @param scenario_filename Path to the scenario file.
  * @param out_dir Output directory.
  * @param is_ef_enabled Flag to enable EF.
  * @param is_lc_enabled Flag to enable LC.
  * @param is_animal_enabled Flag to enable Animal.
  * @param is_manure_enabled Flag to enable Manure.
  * @param manure_nutrients_file Path to the manure nutrients file.
  */
 PSO::PSO(int nparts, int nobjs, int max_iter, double w, double c1, double c2,
          double lb, double ub, const std::string& input_filename,
          const std::string& scenario_filename, const std::string& out_dir,
          bool is_ef_enabled, bool is_lc_enabled, bool is_animal_enabled, bool is_manure_enabled,
          const std::string& manure_nutrients_file)
 {
     out_dir_ = out_dir;
     is_ef_enabled_ = is_ef_enabled;
     is_lc_enabled_ = is_lc_enabled;
     is_animal_enabled_ = is_animal_enabled;
     is_manure_enabled_ = is_manure_enabled;
     ef_size_ = 0;
     lc_size_ = 0;
     animal_size_ = 0;
     manure_size_ = 0;
     init_cast(input_filename, scenario_filename, manure_nutrients_file);
     input_filename_ = input_filename;
     scenario_filename_ = scenario_filename;
     this->nparts = nparts;
     this->nobjs = nobjs;
     this->max_iter = max_iter;
     this->w = w;
     this->c1 = c1;
     this->c2 = c2;
     this->lower_bound = lb;
     this->upper_bound = ub;
     //logger_ = spdlog::stdout_color_mt("PSO");
 }
 
 /**
  * @brief PSO class copy constructor.
  *
  * @param p PSO instance to copy from.
  */
 PSO::PSO(const PSO &p) {
     this->dim = p.dim;
     this->nparts = p.nparts;
     this->nobjs = p.nobjs;
     this->max_iter = p.max_iter;
     this->w = p.w;
     this->c1 = p.c1;
     this->c2 = p.c2;
     this->particles = p.particles;
     this->gbest_x = p.gbest_x;
     this->gbest_fx = p.gbest_fx;
     this->lower_bound = p.lower_bound;
     this->upper_bound = p.upper_bound;
 
     this->is_ef_enabled_ = p.is_ef_enabled_;
     this->is_lc_enabled_ = p.is_lc_enabled_;
     this->is_animal_enabled_ = p.is_animal_enabled_;
     this->is_manure_enabled_ = p.is_manure_enabled_;
     this->input_filename_ = p.input_filename_;
     this->out_dir_ = p.out_dir_;
     this->emo_uuid_ = p.emo_uuid_;
     this->lc_size_ = p.lc_size_;
     this->animal_size_ = p.animal_size_;
     this->manure_size_ = p.manure_size_;
     this->exec_uuid_log_ = p.exec_uuid_log_;
     this->scenario_ = p.scenario_;
     this->execute = p.execute;
     this->gbest_ = p.gbest_;
     //this->logger_ = p.logger_;
 }
 
 /**
  * @brief PSO class assignment operator.
  *
  * @param p PSO instance to assign from.
  * @return PSO& Reference to the assigned instance.
  */
 PSO& PSO::operator=(const PSO &p) {
     if (this == &p) {
         return *this;
     }
     this->dim = p.dim;
     this->nparts = p.nparts;
     this->nobjs = p.nobjs;
     this->max_iter = p.max_iter;
     this->w = p.w;
     this->c1 = p.c1;
     this->c2 = p.c2;
     this->particles = p.particles;
     this->gbest_x = p.gbest_x;
     this->gbest_fx = p.gbest_fx;
     this->lower_bound = p.lower_bound;
     this->upper_bound = p.upper_bound;
     return *this;
 }
 
 /**
  * @brief PSO class destructor.
  */
 PSO::~PSO() {
     //delete_tmp_files();
 }
 
 /**
  * @brief Deletes temporary files created during the optimization.
  *
  * Iterates over the logged execution UUIDs and deletes files matching the pattern.
  */
 void PSO::delete_tmp_files(){
     int counter = 0;
     std::string directory = fmt::format("/opt/opt4cast/output/nsga3/{}", emo_uuid_);
     for (const auto& exec_uuid_vec : exec_uuid_log_) {
         for (const auto& exec_uuid: exec_uuid_vec) {
             auto list_files = misc_utilities::find_files(directory, exec_uuid);
             for (const auto &file : list_files) {
                 auto full_path = fmt::format("{}/{}", directory, file);
                 try {
                     if (fs::exists(full_path)) {
                         fs::remove(full_path);
                         //std::cout << "\t\tDeleted: " << full_path << std::endl;
                     } else {
                         //logger_->error("File not found: {}", full_path);
                         std::cout << "\t\tFile not found: " << full_path << std::endl;
                     }
                 } catch (const std::exception &e) {
                     std::cerr << "\tError deleting full_path " << full_path << ": " << e.what() << std::endl;
                 }
             }
         }
     }
 }
 
 /**
  * @brief Initializes the casting process for the PSO algorithm.
  *
  * Generates a new unique identifier (UUID) for the execution, creates an output directory,
  * initializes the scenario, and determines the dimensions.
  *
  * @param input_filename Path to the input file.
  * @param scenario_filename Path to the scenario file.
  * @param manure_nutrients_file Path to the manure nutrients file.
  */
 void PSO::init_cast(const std::string& input_filename, const std::string& scenario_filename, const std::string& manure_nutrients_file) {
     emo_uuid_ = xg::newGuid().str();
     fmt::print("emo_uuid: {}\n", emo_uuid_);
     std::string emo_path = fmt::format("/opt/opt4cast/output/nsga3/{}/", emo_uuid_);
     std::unordered_map<std::string, std::tuple<int, double, double , double, double>> generation_fx;
     std::unordered_map<std::string, int> generation_uuid_idx;
     misc_utilities::mkdir(emo_path);
 
     scenario_.init(input_filename, scenario_filename, is_ef_enabled_, is_lc_enabled_, is_animal_enabled_, is_manure_enabled_, manure_nutrients_file);
     ef_size_ = scenario_.get_ef_size();
     fmt::print("ef_size: {}\n", ef_size_);
     lc_size_ = scenario_.get_lc_size();
     fmt::print("lc_size: {}\n", lc_size_);
     animal_size_ = scenario_.get_animal_size();
     fmt::print("animal_size: {}\n", animal_size_);
     manure_size_ = scenario_.get_manure_size();
     fmt::print("manure_size: {}\n", manure_size_);
     dim = lc_size_ + animal_size_ + manure_size_ + ef_size_;
     fmt::print("dim: {}\n", dim);
     nobjs = 2;
 }
 
 /**
  * @brief Initializes the PSO swarm.
  *
  * Reserves space for particles, initializes each particle's position using the scenario,
  * evaluates the initial swarm, and sets the personal best for each particle.
  */
 void PSO::init() {
     particles.reserve(nparts);
     for (int i = 0; i < nparts; i++) {
         particles.emplace_back(dim, nobjs, w, c1, c2, lower_bound, upper_bound);
         auto x = particles[i].get_x();
         scenario_.initialize_vector(x);
         particles[i].init(x);
     }
     evaluate();
     for (int i = 0; i < nparts; i++) {
         particles[i].init_pbest();
     }
     update_gbest();
 }
 
 /**
  * @brief Runs the PSO optimization loop.
  *
  * After initialization, the swarm is iterated for a maximum number of iterations.
  * In each iteration, particles update their positions based on a randomly selected global best,
  * the swarm is re-evaluated, and personal and global bests are updated.
  */
 void PSO::optimize() {
     init();
     for (int i = 0; i < max_iter; i++) {
         fmt::print(" =================================================================\n                      iteration: {}\n=================================================================\n", i);
         for (int j = 0; j < nparts; j++) {
             std::uniform_int_distribution<> dis(0, gbest_.size() - 1);
             int index = dis(gen);
             const auto& curr_gbest = gbest_[index].get_x();
             particles[j].update(curr_gbest);
         }
         evaluate();
         update_pbest();
         update_gbest();
     }
     fmt::print("======================Finaliza Optimize===========================================\n");
     exec_ipopt_all_sols();
 }
 
 /**
  * @brief Updates each particle's personal best solution.
  */
 void PSO::update_pbest() {
     for (int j = 0; j < nparts; j++) {
         particles[j].update_pbest();
     }
 }
 
 /**
  * @brief Generates a vector of n new UUIDs.
  *
  * @param n The number of UUIDs to generate.
  * @return std::vector<std::string> Vector of generated UUID strings.
  */
 std::vector<std::string> PSO::generate_n_uuids(int n) {
     std::vector<std::string> uuids;
     for (int i = 0; i < n; i++) {
         uuids.push_back(xg::newGuid().str());
     }
     return uuids;
 }
 
 /**
  * @brief Copies Parquet files for the external optimizer (Ipopt).
  *
  * For each UUID provided, the corresponding animal and manure Parquet files (if they exist) are copied.
  *
  * @param path The directory path.
  * @param parent_uuid The parent solution UUID.
  * @param uuids Vector of UUIDs for the new copies.
  */
 void PSO::copy_parquet_files_for_ipopt(const std::string& path, const std::string& parent_uuid, const std::vector<std::string>& uuids) {
     for (const auto& uuid : uuids) {
         auto animal_src = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", path, parent_uuid);
         if (fs::exists(animal_src)) {
             auto animal_dst = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", path, uuid);
             misc_utilities::copy_file(animal_src, animal_dst);
         }
         auto manure_src = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", path, parent_uuid);
         if (fs::exists(manure_src)) {
             auto manure_dst = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", path, uuid);
             misc_utilities::copy_file(manure_src, manure_dst);
         }
     }
 }
 
 /**
  * @brief Executes Ipopt optimization on selected global best solutions.
  *
  * Selects minimum, median, and maximum solutions (by cost) among the global best particles,
  * writes cost JSON files, calls the external Execute object to run Ipopt,
  * and post-processes the Ipopt results by moving and merging files.
  */
 void PSO::exec_ipopt_all_sols(){
     Execute execute;
     int min_idx = 0;
     int max_idx = 0; 
     int mid_idx;
     std::vector<double> values;
     for (const auto& particle : gbest_) {
         values.emplace_back(particle.get_fx()[0]);
     }
     std::vector<size_t> indices(values.size());
     std::iota(indices.begin(), indices.end(), 0);
     std::sort(indices.begin(), indices.end(),
         [&values](size_t i1, size_t i2) { return values[i1] < values[i2]; });
     min_idx = indices[0];
     max_idx = indices[indices.size() - 1];
     mid_idx = indices[indices.size() / 2];
     std::vector<int> idx_vec = {min_idx, mid_idx, max_idx};
     std::string path = fmt::format("/opt/opt4cast/output/nsga3/{}", emo_uuid_);
     std::string ipopt_path = fmt::format("{}/ipopt", path);
     int counter = 0;
     for (const auto& idx : idx_vec) { 
         auto lc_cost = gbest_[idx].get_lc_cost();
         auto animal_cost = gbest_[idx].get_animal_cost();
         auto manure_cost = gbest_[idx].get_manure_cost();
         auto parent_uuid = gbest_[idx].get_uuid();
         auto parent_uuid_path = fmt::format("{}/{}", path, parent_uuid);
 
         json costs_json_file;
         costs_json_file["lc_cost"] = lc_cost;
         costs_json_file["animal_cost"] = animal_cost;
         costs_json_file["manure_cost"] = manure_cost;
         misc_utilities::write_json_file(fmt::format("{}_costs.json", parent_uuid_path), costs_json_file);
 
         fmt::print("Particle Selected Cost: {}\n", gbest_[idx].get_fx()[0]);
         std::string postfix;
         if (idx == min_idx) postfix = "min";
         else if (idx == max_idx) postfix = "max";
         else postfix = "median";
 
         std::string report_loads_path = fmt::format("{}_reportloads.csv", parent_uuid_path);
         fmt::print("===================================================================================== Scenario_id: {}\n", scenario_.get_scenario_id());
         execute.get_json_scenario(scenario_.get_scenario_id(), report_loads_path, parent_uuid_path);
         auto base_scenario_filename = fmt::format("{}_reportloads_processed.json", parent_uuid_path);
         fmt::print("base_scenario_filename: {}\n", base_scenario_filename);
 
         json uuids_json;
         int pollutant_idx = 0;
         double ipopt_reduction = 0.30;  
         int nsteps = 10;
 
         auto reportloads_json_path = base_scenario_filename;
         auto scenario_json_path = scenario_filename_;
         fmt::print("parent_uuid_path: {}", parent_uuid_path);
 
         execute.execute_new(
                 base_scenario_filename,
                 scenario_filename_,
                 parent_uuid_path,
                 pollutant_idx,
                 ipopt_reduction,
                 nsteps,
                 1,
                 parent_uuid_path 
             );
         fmt::print("before move_files\n");
         misc_utilities::move_files(parent_uuid_path, ipopt_path, nsteps, counter * nsteps);
         ++counter;
     }
 
     std::vector<std::string> objectives = {"cost", "EoS-N"};  // Example objectives
     std::string directory = ipopt_path; 
     std::string pf_path = fmt::format("{}/front", path);
     std::string csv_path = fmt::format("{}/front/pareto_front.txt", path);
 
     fmt::print("before find pareto frontfiles \n");
     std::vector<std::string> pf_files = findParetoFrontFiles(objectives, directory);
     misc_utilities::move_pf(ipopt_path, pf_path, pf_files);
 
     std::vector<CostData> pf_data = readCostFiles(objectives, pf_path);
     writeCSV(pf_data, csv_path, objectives);
 }
 
 /**
  * @brief Evaluates Ipopt solutions.
  *
  * Reads the parent scenario, obtains Ipopt solutions, writes land and JSON files,
  * copies additional files if necessary, and then updates cost information.
  *
  * @param sub_dir The sub-directory name for Ipopt results.
  * @param ipopt_uuid The UUID of the selected Ipopt solution.
  * @param animal_cost The animal cost component.
  * @param manure_cost The manure cost component.
  */
 void PSO::evaluate_ipopt_sols(const std::string& sub_dir, const std::string& ipopt_uuid, double animal_cost, double manure_cost) {
     std::vector<std::string> exec_uuid_vec;
     std::unordered_map<std::string, double> total_cost_map;
     std::string emo_path = fmt::format("/opt/opt4cast/output/nsga3/{}", emo_uuid_);
     auto ipopt_in_filename = fmt::format("{}/{}_impbmpsubmittedland.json", emo_path, ipopt_uuid);
     fmt::print("ipopt_in_filename: {}\n", ipopt_in_filename);
     std::vector<std::tuple<int, int, int, int, double>> parent_list;
     if(is_lc_enabled_){
         parent_list = read_scenario_json(ipopt_in_filename);
     }
     std::string filename_ipopt_out = fmt::format("{}/config/ipopt.json", emo_path);
     std::string filename_ipopt_out2 = fmt::format("{}/config/ipopt2.json", emo_path);
     fmt::print("filename_ipopt_out: {}\n", filename_ipopt_out);
     auto alpha_dict = scenario_.get_alpha();
     auto ipopt_lists = read_scenarios_keyed_json2(filename_ipopt_out2, alpha_dict);
 
     for (const auto& parcel_list : ipopt_lists) {
         std::vector<std::tuple<int, int, int, int, double>> combined;
         if(is_lc_enabled_){
             combined = parent_list;
         }
         combined.insert(combined.end(), parcel_list.begin(), parcel_list.end());
 
         std::string exec_uuid = xg::newGuid().str();
         exec_uuid_vec.emplace_back(exec_uuid);
         auto land_filename = fmt::format("{}/{}_impbmpsubmittedland.parquet", emo_path, exec_uuid);
         scenario_.write_land(combined, land_filename);
         scenario_.write_land_json(combined, replace_ending(land_filename, ".parquet", ".json"));
 
         if(is_animal_enabled_){
             auto animal_dst = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", emo_path, exec_uuid);
             misc_utilities::copy_file(fmt::format("{}/{}_impbmpsubmittedanimal.parquet", emo_path, ipopt_uuid), animal_dst );
         }
         if(is_manure_enabled_){
             auto manure_dst = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", emo_path, exec_uuid);
             misc_utilities::copy_file(fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", emo_path, ipopt_uuid), manure_dst );
         }
         total_cost_map[exec_uuid] = scenario_.compute_cost(combined) + animal_cost + manure_cost;
     }
 
     auto results = scenario_.send_files(emo_uuid_, exec_uuid_vec);
     auto dir_path = fmt::format("{}/config/{}", emo_path, sub_dir);
     misc_utilities::mkdir(dir_path);
     misc_utilities::copy_file(fmt::format("{}/config/ipopt.json", emo_path), fmt::format("{}/ipopt.json", dir_path));
     misc_utilities::copy_file(fmt::format("{}/config/ipopt2.json", emo_path), fmt::format("{}/ipopt2.json", dir_path));
     
     std::vector<std::vector<double>> result_fx;
     int counter = 0;
     for (const auto& result : results) {
         std::vector<std::string> result_vec;
         misc_utilities::split_str(result, '_', result_vec);
         auto exec_uuid = result_vec[0];
         result_fx.push_back({total_cost_map[exec_uuid], std::stod(result_vec[1])});
 
         std::regex pattern(exec_uuid);
         auto str_replacement = std::to_string(counter);
         auto found_files = misc_utilities::find_files(emo_path, exec_uuid);
         for (const auto& filename : found_files) {
             fmt::print("filename: {}\n", filename);
             auto filename_dst = std::regex_replace(filename, pattern, str_replacement);
             misc_utilities::copy_file(fmt::format("{}/{}", emo_path, filename), fmt::format("{}/{}", dir_path, filename_dst));
         }
         counter++;
     } 
 
     for(const auto& result : result_fx) {
         fmt::print("ipopt solution : [{}, {}]\n", result[0], result[1]);
     }
     exec_uuid_log_.push_back(exec_uuid_vec);
     save(result_fx, fmt::format("{}/config/{}/pareto_front_ipopt.txt", emo_path, sub_dir));
     fmt::print("end\n");
 }
 
 /**
  * @brief Executes Ipopt on the best particle among the global best solutions.
  *
  * Selects the particle with the lowest cost, executes Ipopt locally, updates outputs,
  * and then evaluates the Ipopt solutions.
  */
 void PSO::exec_ipopt(){
     Execute execute;
     Particle particle_selected;
     bool flag = true;
     for (const auto& particle : gbest_) {
         if (flag || particle_selected.get_fx()[0] > particle.get_fx()[0]) {
             particle_selected = particle;
             flag = false;
         }
     }
     auto ipopt_uuid = particle_selected.get_uuid();
     fmt::print("ipopt_uuid: {}\n", ipopt_uuid);
     auto in_file = fmt::format("/opt/opt4cast/output/nsga3/{}/{}_reportloads.csv", emo_uuid_, ipopt_uuid);
     auto lc_cost = particle_selected.get_lc_cost();
     auto animal_cost = particle_selected.get_animal_cost();
     auto manure_cost = particle_selected.get_manure_cost();
     execute.set_files(emo_uuid_, in_file);
     execute.execute(emo_uuid_, 0.50, 6, 20);
     std::string  in_path = "/opt/opt4cast/output/nsga3/592e98d5-2d52-4d25-99cb-76f88a6d4e09/config/reportloads_processed.json"; 
     std::string out_path = "/opt/opt4cast/output/nsga3/592e98d5-2d52-4d25-99cb-76f88a6d4e09/config/scenario.json";
     std::string uuids = "/opt/opt4cast/output/nsga3/592e98d5-2d52-4d25-99cb-76f88a6d4e09/config/uuids.json";
     int pollutant_idx = 0;
     double ipopt_reduction = 0.7;
     int ipopt_popsize = 20;
     execute.execute_local(
         in_path,
         out_path,
         pollutant_idx,
         ipopt_reduction,
         ipopt_popsize
     ); 
     fmt::print("Particle Selected Cost: {}\n", particle_selected.get_fx()[0]);
     execute.update_output(emo_uuid_, particle_selected.get_fx()[0]);
     fmt::print("======================== best_lc_cost_: {}\n", lc_cost);
     fmt::print("======================== best_animal_cost_: {}\n", animal_cost);
     fmt::print("======================== best_manure_cost_: {}\n", manure_cost);
     std::string sub_dir = "ipopt_results";
     evaluate_ipopt_sols(sub_dir, ipopt_uuid, animal_cost, manure_cost);
 }
 
 /**
  * @brief Updates the global best solutions (gbest) using non-dominated sorting.
  *
  * Iterates over all particles and updates the set of global best solutions.
  */
 void PSO::update_gbest() {
     for (int j = 0; j < nparts; j++) {
         update_non_dominated_solutions(gbest_, particles[j]);
     }
 }
 
 /**
  * @brief Prints the global best objective values and positions.
  */
 void PSO::print() {
     std::cout << "gbest_fx: ";
     for(auto& row : gbest_fx) {
         for(auto& val : row) {
             std::cout << val << " ";
         }
         std::cout << "\n";
     }
     std::cout << "gbest_x: ";
     for(auto& row : gbest_x) {
         for(auto& val : row) {
             std::cout << val << " ";
         }
         std::cout << "\n";
     }
     std::cout << "--------------------------\n";
 }
 
 /**
  * @brief Evaluates all particles in the swarm.
  *
  * For each particle, the function computes various costs based on the scenario (land, animal, manure),
  * writes corresponding files, sends files for external evaluation, and then updates the objective values.
  */
 void PSO::evaluate() {
     std::vector<std::string> exec_uuid_vec;
     std::vector<double> total_cost_vec(nparts, 0.0);
     std::unordered_map<std::string, int> generation_uuid_idx;
     std::string emo_path = fmt::format("/opt/opt4cast/output/nsga3/{}/", emo_uuid_);
     for (int i = 0; i < nparts; i++) {
         std::vector<std::tuple<int, int, int, int, double>> lc_x;
         std::vector<std::tuple<int, int, int, int, int, double>> animal_x;
         std::vector<std::tuple<int, int, int, int, int, double>> manure_x;
         std::unordered_map<std::string, double> amount_minus;
         std::unordered_map<std::string, double> amount_plus;
         double total_cost = 0.0;
 
         const auto& x = particles[i].get_x();
         std::string exec_uuid = xg::newGuid().str();
         particles[i].set_uuid(exec_uuid);
         bool flag = true;
         if(is_ef_enabled_){
             // EF normalization (if enabled)
         }
         if(is_lc_enabled_){
             double lc_cost  = scenario_.normalize_lc(x, lc_x, amount_minus, amount_plus);
             particles[i].set_amount_minus(amount_minus);
             particles[i].set_amount_plus(amount_plus);
             particles[i].set_lc_cost(lc_cost);
             total_cost += lc_cost;
             particles[i].set_lc_x(lc_x);
             auto land_filename = fmt::format("{}/{}_impbmpsubmittedland.parquet", emo_path, exec_uuid);
             scenario_.write_land(lc_x, land_filename);
             if (!std::filesystem::exists(land_filename)) {
                 total_cost = 9999999999999.99;
                 particles[i].set_lc_cost(lc_cost);
                 particles[i].set_fx(total_cost, total_cost);
                 flag = false;
             }
             scenario_.write_land_json(lc_x, replace_ending(land_filename, ".parquet", ".json"));
         }
         if(is_animal_enabled_){
             auto animal_cost = scenario_.normalize_animal(x, animal_x); 
             particles[i].set_animal_cost(animal_cost);
             total_cost += animal_cost;
             particles[i].set_animal_x(animal_x);
             auto animal_filename = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", emo_path, exec_uuid);
             scenario_.write_animal(animal_x, animal_filename);
             if (!std::filesystem::exists(animal_filename)) {
                 total_cost = 9999999999999.99;
                 particles[i].set_animal_cost(animal_cost);
                 particles[i].set_fx(total_cost, total_cost);
                 flag = false;
             }
             scenario_.write_animal_json(animal_x, replace_ending(animal_filename, ".parquet", ".json"));
         }
         if(is_manure_enabled_){
             auto manure_cost = scenario_.normalize_manure(x, manure_x); 
             particles[i].set_manure_cost(manure_cost);
             total_cost += manure_cost;
             particles[i].set_manure_x(manure_x);
             auto manure_filename = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", emo_path, exec_uuid);
             scenario_.write_manure(manure_x, manure_filename);
             if (!std::filesystem::exists(manure_filename)) {
                 total_cost = 9999999999999.99;
                 particles[i].set_manure_cost(manure_cost);
                 particles[i].set_fx(total_cost, total_cost);
                 flag = false;
             }
             scenario_.write_manure_json(manure_x, replace_ending(manure_filename, ".parquet", ".json"));
         }
         if(flag){
             generation_uuid_idx[exec_uuid] = i;
             total_cost_vec[i] = total_cost;
             exec_uuid_vec.push_back(exec_uuid);
         }
     }
 
     auto results = scenario_.send_files(emo_uuid_, exec_uuid_vec);
     for (auto const& key : results) {
         std::vector<std::string> result_vec;
         misc_utilities::split_str(key, '_', result_vec);
         auto stored_idx = generation_uuid_idx[result_vec[0]];
         particles[stored_idx].set_fx(total_cost_vec[stored_idx], std::stod(result_vec[1]));
     } 
 
     for (int i = 0; i < nparts; i++) {
         const auto& new_solution_fx = particles[i].get_fx();
         if (new_solution_fx[1] >= 9999999999999.0) {
             fmt::print("New solution fx[{}]: [{}, {}]\n", particles[i].get_uuid(), new_solution_fx[0], new_solution_fx[1]);
         }
         else {
             fmt::print("new_solution_fx[{}]: [{}, {}]\n", i, new_solution_fx[0], new_solution_fx[1]);
         }
     }
     exec_uuid_log_.push_back(exec_uuid_vec);
 }
 
 /**
  * @brief Saves the global best solutions to the specified output directory.
  *
  * Copies the entire output (front) directory to the given location.
  *
  * @param out_dir The output directory to save the global best solutions.
  */
 void PSO::save_gbest(std::string out_dir) {
     auto pf_path = fmt::format("{}", out_dir);
     std::string emo_path = fmt::format("/opt/opt4cast/output/nsga3/{}/front", emo_uuid_);
     misc_utilities::copy_full_directory(emo_path, pf_path);
 }
 