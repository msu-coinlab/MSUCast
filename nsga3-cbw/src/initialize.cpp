/* Data initializtion routines */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include "global.h"
#include "rand.h"
#include <misc.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <map>
#include <filesystem>
#include <random>
#include <algorithm>

using json = nlohmann::json;
namespace fs = std::filesystem;

std::string prefix_init_file;
int sols_init_file;
int n_injected_points;
std::string injected_points_filename;

std::vector<std::string> my_s_h_u_vec;

/**
 * @brief Structure to represent a variable with location and amount information.
 */
struct var_t {
    std::string name;          ///< Name of the variable
    std::vector<int> location; ///< Location coordinates
    double amount;             ///< Amount value
    int bmp;                   ///< BMP (Best Management Practice) identifier
};

/**
 * @brief Converts a var_t object to JSON format.
 * @param j JSON object to store the data
 * @param p var_t object to be converted
 */
void to_json(json &j, const var_t &p) {
    j = json{{"name",  p.name},
             {"location", p.location},
             {"amount",   p.amount},
             {"bmp", p.bmp}};
}

/**
 * @brief Converts JSON data to a var_t object.
 * @param j JSON object containing the data
 * @param p var_t object where data will be stored
 */
void from_json(const json &j, var_t &p) {
    j.at("name").get_to(p.name);
    j.at("location").get_to(p.location);
    j.at("amount").get_to(p.amount);
    j.at("bmp").get_to(p.bmp);
}

/**
 * @brief Reads land conversion data from a JSON file.
 * @param init_pop Vector to store the initialized population
 * @param json_filename Name of the JSON file to read
 */
void read_json_lc_file(std::vector<std::vector<double> > &init_pop, std::string json_filename) {
    std::ifstream ifs_lc(json_filename);
    json jf_lc = json::parse(ifs_lc);
    std::unordered_map<std::string, double> loaded_lc_map;

    for (json j_lc: jf_lc) {
        for (json bmp_lc: j_lc) {
            var_t my_bmp = bmp_lc;
            auto s_lc = my_bmp.location[0];
            auto h_lc = my_bmp.location[1];
            auto u_lc = my_bmp.location[2];
            auto t_bmp_lc = my_bmp.location[3];
            std::string curr_parcel_lc = fmt::format("{}_{}_{}", s_lc, h_lc, u_lc, t_bmp_lc);
            loaded_lc_map[curr_parcel_lc] = my_bmp.amount;
        }
    }
}

/**
 * @brief Reads initialization data from a JSON file.
 * @param init_pop Vector to store the initialized population
 * @param json_filename Name of the JSON file to read
 */
void read_json_file(std::vector<std::vector<double> > &init_pop, std::string json_filename) {
    // Implementation details...
}

/**
 * @brief Generates a random number within a specified range.
 * @param n Upper bound of the range (exclusive)
 * @return Random integer between 0 and n
 */
int get_random_number(int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, n);
    return dist(gen);
}

/**
 * @brief Initializes a population with individuals.
 * 
 * This function initializes a population by either reading from a JSON file
 * or generating random individuals. It handles both injected points and
 * randomly generated individuals.
 *
 * @param pop Pointer to the population to be initialized
 */
void initialize_pop(population *pop)
{
    int i;
    std::vector<std::vector<double> > init_pop;
    read_json_file(init_pop, injected_points_filename);

    n_injected_points = (int) init_pop.size();
    for (i = 0; i < n_injected_points; i++) {
        if (i >= popsize)
            break;

        for (int j = 0; j < nreal; j++) {
            pop->ind[i].xreal[j] = init_pop[i][j];
        }
    }
    for (i = n_injected_points; i < popsize; i++)
    {
        if (n_injected_points != 0) {
            int injected = get_random_number(n_injected_points-1);
            for (int j = 0; j < nreal; j++) {
                pop->ind[i].xreal[j] = init_pop[injected][j];
            }
        }
        initialize_ind(&(pop->ind[i]), n_injected_points);
    }
    return;
}

/**
 * @brief Initializes a single individual.
 * 
 * This function initializes an individual with either random values or
 * specific values based on the problem configuration. It handles both
 * real-valued and binary variables.
 *
 * @param ind Pointer to the individual to be initialized
 * @param n_injected_points Number of points already injected into the population
 */
void initialize_ind(individual *ind, int n_injected_points)
{
    int j, k;
    if (nreal != 0)
    {
        for (j = 0; j < ef_end_; j++)
        {
            if(n_injected_points == 0) {
                ind->xreal[j] = rndreal(min_realvar[j], max_realvar[j]);
            }
        }
        // Handle land conversion and animal enabled cases...
    }

    if (nbin != 0)
    {
        for (j = 0; j < nbin; j++)
        {
            for (k = 0; k < nbits[j]; k++)
            {
                if (randomperc() <= 0.5)
                {
                    ind->gene[j][k] = 0;
                }
                else
                {
                    ind->gene[j][k] = 1;
                }
            }
        }
    }
    return;
}