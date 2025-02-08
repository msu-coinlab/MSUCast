#ifndef PSO_H
#define PSO_H

#include <iostream>
#include <vector>
#include "particle.h"
#include "scenario.h" 
#include "execute.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

//#include "spdlog/spdlog.h"
//#include "spdlog/sinks/stdout_color_sinks.h"

/**
 * @class PSO
 * @brief Particle Swarm Optimization class
 */
class PSO {
public:
    /**
     * @brief Constructor to initialize the PSO algorithm.
     * @param nparts Number of particles.
     * @param nobjs Number of objectives.
     * @param max_iter Maximum number of iterations.
     * @param w Inertia weight.
     * @param c1 Cognitive coefficient.
     * @param c2 Social coefficient.
     * @param lb Lower bound.
     * @param ub Upper bound.
     * @param input_filename The input filename.
     * @param scenario_filename The scenario filename.
     * @param out_dir The output directory.
     * @param is_ef_enabled Flag to enable EF.
     * @param is_lc_enabled Flag to enable LC.
     * @param is_animal_enabled Flag to enable animal.
     * @param is_manure_enabled Flag to enable manure.
     * @param manure_nutrients_file The manure nutrients file.
     */
    PSO(int nparts, int nobjs, int max_iter, double w, double c1, double c2, double lb, double ub, const std::string& input_filename, const std::string& scenario_filename, const std::string& out_dir, bool is_ef_enabled, bool is_lc_enabled, bool is_animal_enabled, bool is_manure_enabled, const std::string& manure_nutrients_file);

    /**
     * @brief Destructor.
     */
    ~PSO();

    /**
     * @brief Copy constructor.
     * @param p The PSO object to copy.
     */
    PSO(const PSO &p);

    /**
     * @brief Assignment operator.
     * @param p The PSO object to assign from.
     * @return Reference to the assigned PSO object.
     */
    PSO& operator=(const PSO &p);

    /**
     * @brief Initializes the PSO algorithm.
     */
    void init();

    /**
     * @brief Optimizes using the PSO algorithm.
     */
    void optimize();

    /**
     * @brief Prints the PSO details.
     */
    void print();

    /**
     * @brief Generates a specified number of UUIDs.
     * @param n The number of UUIDs to generate.
     * @return A vector of generated UUIDs.
     */
    std::vector<std::string> generate_n_uuids(int n);

    /**
     * @brief Copies Parquet files for IPOPT.
     * @param path The path to copy to.
     * @param parent_uuid The parent UUID.
     * @param uuids The UUIDs to copy.
     */
    void copy_parquet_files_for_ipopt(const std::string& path, const std::string& parent_uuid, const std::vector<std::string>& uuids);

    /**
     * @brief Gets the global best position.
     * @return The global best position.
     */
    std::vector<std::vector<double>> get_gbest_x() {
        return gbest_x;
    }

    /**
     * @brief Gets the global best position (const reference).
     * @return The global best position (const reference).
     */
    const std::vector<std::vector<double>>& get_gbest_x_reference() {
        return gbest_x;
    }

    /**
     * @brief Gets the global best fitness values.
     * @return The global best fitness values.
     */
    std::vector<std::vector<double>> get_gbest_fx() {
        return gbest_fx;
    }

    /**
     * @brief Gets the global best fitness values (const reference).
     * @return The global best fitness values (const reference).
     */
    const std::vector<std::vector<double>>& get_gbest_fx_reference() {
        return gbest_fx;
    }

    /**
     * @brief Gets the global best particles.
     * @return The global best particles.
     */
    const std::vector<Particle>& get_gbest() const {
        return gbest_;
    }

    /**
     * @brief Saves the global best particles to the specified directory.
     * @param out_dir The output directory.
     */
    void save_gbest(std::string out_dir);

private:
    int dim; ///< Dimension of the particles.
    int nparts; ///< Number of particles.
    int nobjs; ///< Number of objectives.
    int max_iter; ///< Maximum number of iterations.
    double w; ///< Inertia weight.
    double c1; ///< Cognitive coefficient.
    double c2; ///< Social coefficient.

    Scenario scenario_; ///< Scenario object.
    std::vector<Particle> particles; ///< Vector of particles.
    std::vector<Particle> gbest_; ///< Global best particles.
    std::vector<std::vector<double>> gbest_x; ///< Global best positions.
    std::vector<std::vector<double>> gbest_fx; ///< Global best fitness values.

    double lower_bound; ///< Lower bound for positions.
    double upper_bound; ///< Upper bound for positions.
    void update_gbest(); ///< Updates the global best particles.

    // CAST
    void init_cast(const std::string& input_filename, const std::string& scenario_filename, const std::string& manure_nutrients_file);
    std::string emo_uuid_; ///< UUID for the emotion data.
    int ef_size_; ///< EF size.
    int lc_size_; ///< LC size.
    int animal_size_; ///< Animal size.
    int manure_size_; ///< Manure size.

    void evaluate(); ///< Evaluates the particles.
    void update_pbest(); ///< Updates the personal best particles.
    bool is_ef_enabled_; ///< Flag to enable EF.
    bool is_lc_enabled_; ///< Flag to enable LC.
    bool is_animal_enabled_; ///< Flag to enable animal.
    bool is_manure_enabled_; ///< Flag to enable manure.
    std::string input_filename_; ///< Input filename.
    std::string scenario_filename_; ///< Scenario filename.
    std::string out_dir_; ///< Output directory.
    Execute execute; ///< Execute object.
    std::vector<std::vector<std::string>> exec_uuid_log_; ///< Log of execution UUIDs.

    void exec_ipopt(); ///< Executes IPOPT.
    void exec_ipopt_all_sols(); ///< Executes IPOPT for all solutions.
    void delete_tmp_files(); ///< Deletes temporary files.

    void evaluate_ipopt_sols(const std::string& sub_dir, const std::string& ipopt_uuid, double animal_cost, double manure_cost);

    double best_lc_cost_; ///< Best land cover cost.
    double best_animal_cost_; ///< Best animal cost.
    //std::shared_ptr<spdlog::logger> logger_; ///< Logger object.
};

#endif // PSO_H
