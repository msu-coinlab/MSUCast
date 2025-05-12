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
    
    void save(const std::vector<std::vector<double>>& data, const std::string& filename) {
        std::ofstream outFile(filename);
        
        // Check if the file opened successfully
        if (!outFile) {
            std::cerr << "Failed to open the file: "<<filename<<".\n";
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
    void save(const std::vector<Particle>& data, const std::string& filename, const std::string& filename_fx) {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Failed to open the file: "<<filename<<".\n";
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
            std::cerr << "Failed to open the file: "<<filename_fx<<".\n";
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


int main (int argc, char *argv[]) {
    int nparts = 5;
    int nobjs = 2;
    int max_iter = 2;
    double c1 = 1.4;
    double c2 = 1.4;
    double w = 0.7;
    double lb = 0.0;
    double ub = 1.0;
    std::string input_filename;
    std::string scenario_filename;
    std::string dir_output = "./";
    bool is_ef_enabled = false;
    bool is_lc_enabled = true;
    bool is_animal_enabled = false;
    bool is_manure_enabled = false;
    std::string manure_nutrients_file = "manure_nutrients.json";
    std::string base_land_bmp_file;
    // Jefferson.json 
    //./PSOCast Nelson.json an-all 0 0 1
    // ./pso /opt/opt4cast/output/nsga3/2d3661e2-2012-493f-87ef-65b544f14902/config/reportloads_processed.json /opt/opt4cast/output/nsga3/2d3661e2-2012-493f-87ef-65b544f14902/config/scenario.json ./test 1 1 0 0
    //
    if (argc > 1) {
        input_filename = argv[1];
        scenario_filename = argv[2];
        dir_output = argv[3];
        is_ef_enabled = std::stoi(argv[4]);
        is_lc_enabled = std::stoi(argv[5]);
        is_animal_enabled = std::stoi(argv[6]);
        is_manure_enabled = std::stoi(argv[7]);
        manure_nutrients_file = argv[8];
        base_land_bmp_file = argv[9];
    } 
    
        
    

    PSO pso(nparts, nobjs, max_iter, w, c1, c2, lb, ub, input_filename, scenario_filename, dir_output, is_ef_enabled, is_lc_enabled, is_animal_enabled, is_manure_enabled, manure_nutrients_file, base_land_bmp_file);
    pso.optimize();
    pso.save_gbest(dir_output);
    //std::vector<Particle> gbest = pso.get_gbest();
    //save(gbest, "gbest_x_2.txt", "gbest_fx_2.txt" );
    //pso.print();
    //std::vector<double> gbest_x = pso.get_gbest_x();

    return 0;
}

