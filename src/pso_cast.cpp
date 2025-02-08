/**
 * @file pso_cast.cpp
 * @brief Main entry point for running the Particle Swarm Optimization (PSO) algorithm.
 */

 #include <iostream>
 #include <fstream>
 #include <string>
 #include <vector>
 #include <fmt/core.h>
 #include <unordered_map>
 
 #include "json.hpp"
 #include "pso.h"
 #include "particle.h"
 #include "scenario.h"
 #include "misc_utilities.h"
 
 using json = nlohmann::json;
 
 namespace {
     /**
      * @brief Saves a 2D vector of data to a file.
      * @param data The 2D vector to save.
      * @param filename The name of the file to save the data to.
      */
     void save(const std::vector<std::vector<double>>& data, const std::string& filename) {
         std::ofstream outFile(filename);
 
         // Check if the file opened successfully
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
      * @brief Saves a vector of Particle objects to two files: one for positions and one for objective function values.
      * @param data The vector of Particle objects to save.
      * @param filename The name of the file to save the particle positions.
      * @param filename_fx The name of the file to save the particle objective function values.
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
 }
 
 /**
  * @brief Main function for executing the Particle Swarm Optimization algorithm.
  * 
  * The program can be executed with command-line arguments specifying input files and settings.
  * 
  * Example usage:
  * ```
  * ./pso_cast input.json scenario.json output_dir 1 1 0 0 manure_nutrients.json
  * ```
  * 
  * @param argc Number of command-line arguments.
  * @param argv Array of command-line argument strings.
  * @return Exit status code (0 for success).
  */
 int main(int argc, char *argv[]) {
     // Default PSO parameters
     int nparts = 20; ///< Number of particles
     int nobjs = 2; ///< Number of objective functions
     int max_iter = 20; ///< Maximum number of iterations
     double c1 = 1.4; ///< Cognitive coefficient
     double c2 = 1.4; ///< Social coefficient
     double w = 0.7; ///< Inertia weight
     double lb = 0.0; ///< Lower bound for particle position
     double ub = 1.0; ///< Upper bound for particle position
 
     // Input and output settings
     std::string input_filename;
     std::string scenario_filename;
     std::string dir_output = "./";
 
     // Feature flags
     bool is_ef_enabled = false;
     bool is_lc_enabled = true;
     bool is_animal_enabled = false;
     bool is_manure_enabled = false;
     std::string manure_nutrients_file = "manure_nutrients.json";
 
     // Parse command-line arguments
     if (argc > 1) {
         input_filename = argv[1];
         scenario_filename = argv[2];
         dir_output = argv[3];
         is_ef_enabled = std::stoi(argv[4]);
         is_lc_enabled = std::stoi(argv[5]);
         is_animal_enabled = std::stoi(argv[6]);
         is_manure_enabled = std::stoi(argv[7]);
     } 
     if (argc > 8) {
         manure_nutrients_file = argv[8];
     }
 
     // Initialize PSO algorithm
     PSO pso(nparts, nobjs, max_iter, w, c1, c2, lb, ub, input_filename, scenario_filename, dir_output, 
             is_ef_enabled, is_lc_enabled, is_animal_enabled, is_manure_enabled, manure_nutrients_file);
 
     // Run the optimization process
     pso.optimize();
 
     // Save the global best solution
     pso.save_gbest(dir_output);
 
     return 0;
 }
 