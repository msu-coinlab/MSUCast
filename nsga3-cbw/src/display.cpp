/**
 * @file display.cpp
 * @brief Routines to display the population information using gnuplot.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 #include <string.h>
 #include <unistd.h>
 #include <iostream>
 #include <string>
 #include <fmt/core.h>
 #include <filesystem>
 #include <fstream>
 
 #include "global.h"
 #include "rand.h"
 #include "misc.hpp"
 
 extern int mode;
 namespace fs = std::filesystem;
 
 std::vector<double> true_pf_lb;
 std::vector<double> true_pf_ub;
 double hv_raw_pf;
 double pct_hv_to_exit;
 
 /**
  * @brief Displays the current population for the subsequent generation.
  * 
  * This function processes the population and writes data to several output
  * files, including Pareto front data and UUID registry information.
  * It also copies relevant files for visualization.
  * 
  * @param pop Pointer to the population structure.
  * @param gp Pointer to the GNUplot file.
  * @param ii Current generation index.
  */
 void onthefly_display(population *pop, FILE *gp, int ii)
 {
     int i, j;
     int flag;
     FILE *fpt, *fp;
 
     std::string msu_cbpo_path = getEnvVar("MSU_CBPO_PATH", "/opt/opt4cast");
     std::string stored_path = fmt::format("{}/output/nsga3/{}/front/", msu_cbpo_path, emo_uuid);
     std::string specific_path = stored_path + "plot.out";
     
     fpt = fopen(specific_path.c_str(), "w");
     specific_path = stored_path + "pareto_front.out";
     fp = fopen(specific_path.c_str(), "a");
     specific_path = stored_path + "pareto_front_uuid.out";
     std::ofstream ofile(specific_path);
 
     specific_path = fmt::format("{}{}", stored_path, "pareto_front.out");
 
     flag = 0;
     std::string dir_path = fmt::format("{}/output/nsga3/{}/front", msu_cbpo_path, emo_uuid);
     
     if (!fs::exists(dir_path)) 
         fs::create_directories(dir_path);
 
     specific_path = stored_path + "exec_uuid_registry.out";
     std::ofstream ofile_registry(specific_path);
     for (const auto &[key, value] : uuid_registry)
         ofile_registry << key << " " << value << std::endl;
     ofile_registry.close();
 
     specific_path = stored_path + "uuid_surviving_pop_registry.out";
     std::ofstream ofile_surviving_pop_registry(specific_path);
     for (const auto &[key, value] : uuid_surviving_pop_registry)
         ofile_surviving_pop_registry << key << " " << value << std::endl;
     ofile_surviving_pop_registry.close();
 
     for (i = 0; i < popsize; i++) {
         if (pop->ind[i].constr_violation >= 0.0 && pop->ind[i].rank == 1) {
             auto uuid_registry_idx = fmt::format("{}_{}", ii, i);
             std::string tmp_obj = fmt::format("{}", pop->ind[i].obj[0]);
             for (int j = 1; j < nobj; ++j) {
                 tmp_obj = fmt::format("{},{}", tmp_obj, pop->ind[i].obj[j]);
             }
 
             std::string exec_uuid = (ii == 1) ? uuid_registry[uuid_registry_idx] : uuid_surviving_pop_registry[uuid_registry_idx];
 
             ofile << fmt::format("{},{},{}\n", i, exec_uuid, tmp_obj);
             
             // Copy relevant files for visualization
             std::vector<std::string> file_types = {
                 "_output_t.csv", "_output_t.txt", "_reportloads.csv",
                 "_reportloads.parquet", "_impbmpsubmittedland.parquet",
                 "_impbmpsubmittedanimal.parquet", "_bmp_summary_t.txt"
             };
 
             for (const auto &file_type : file_types) {
                 std::string filename_src = fmt::format("{}/output/nsga3/{}/{}{}", msu_cbpo_path, emo_uuid, exec_uuid, file_type);
                 std::string filename_dst = fmt::format("{}/output/nsga3/{}/front/{}{}", msu_cbpo_path, emo_uuid, i, file_type);
                 if (fs::exists(filename_src)) 
                     fs::copy(filename_src, filename_dst, fs::copy_options::update_existing);
             }
         }
     }
 
     fprintf(fp, "#\n");
     fclose(fpt);
     fclose(fp);
     ofile.close();
 }
 
 /**
  * @brief Saves the current population data for the given generation.
  * 
  * @param pop Pointer to the population structure.
  * @param ii Current generation index.
  */
 void onthefly_display2(population *pop, int ii)
 {
     int i, j;
     int flag;
     FILE *fp;
 
     std::string msu_cbpo_path = getEnvVar("MSU_CBPO_PATH", "/opt/opt4cast");
     std::string stored_path = fmt::format("{}/output/nsga3/{}/front/gen_{}_sol.out", msu_cbpo_path, emo_uuid, ii);
     fp = fopen(stored_path.c_str(), "a");
 
     flag = 0;
     for (i = 0; i < popsize; i++) {
         if (true) {
             if (nobj < 4) {
                 if (choice != 3) {
                     fprintf(fp, "%e\t%e\n", pop->ind[i].obj[obj1 - 1], pop->ind[i].obj[obj2 - 1]);
                 } else {
                     fprintf(fp, "%e\t%e\t%e\n", pop->ind[i].obj[obj1 - 1], pop->ind[i].obj[obj2 - 1], pop->ind[i].obj[obj3 - 1]);
                 }
             } else {
                 for (j = 0; j < nobj; j++) {
                     fprintf(fp, "%e\t", pop->ind[i].obj[j]);
                 }
                 fprintf(fp, "\n");
             }
         }
     }
     fclose(fp);
 }
 
 /**
  * @brief Saves the current Pareto front and returns the count of solutions.
  * 
  * @param pop Pointer to the population structure.
  * @return The number of solutions in the current Pareto front.
  */
 int onthefly_display2(population *pop)
 {
     int i, j;
     FILE *fpt; 
     fpt = fopen("current_pareto_front.out", "w");
     fprintf(fpt, "#\n");
     int counter = 0;
 
     for (i = 0; i < popsize; i++) {
         if (pop->ind[i].constr_violation == 0.0 && pop->ind[i].rank == 1) {
             if (nobj < 4) {
                 double norm_obj_1 = (pop->ind[i].obj[obj1 - 1] - true_pf_lb[obj1-1]) / (true_pf_ub[obj1-1] - true_pf_lb[obj1-1]);
                 double norm_obj_2 = (pop->ind[i].obj[obj2 - 1] - true_pf_lb[obj2-1]) / (true_pf_ub[obj2-1] - true_pf_lb[obj2-1]);
                 
                 if (pop->ind[i].obj[obj1-1] <= true_pf_ub[obj1-1] && pop->ind[i].obj[obj2 - 1] <= true_pf_ub[obj2-1]) {
                     fprintf(fpt, "%e %e\n", norm_obj_1, norm_obj_2);
                     ++counter;
                 }
             }
         }
     }
     
     fprintf(fpt, "#\n");
     fclose(fpt);
     return counter;
 }
 