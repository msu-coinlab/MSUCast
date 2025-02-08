// Created by: Gregorio Toscano

#ifndef SCENARIO_H
#define SCENARIO_H

#include <vector>
#include <string>
#include <unordered_map>

/**
 * @class Scenario
 * @brief A class representing a scenario for optimization algorithms.
 */
class Scenario {
public:
    /**
     * @brief Constructor to initialize a Scenario object.
     */
    Scenario();

    /**
     * @brief Loads the scenario from files.
     * @param filename The filename for loading scenario data.
     * @param filename_scenario The filename for loading scenario configuration.
     */
    void load(const std::string& filename, const std::string& filename_scenario);

    /**
     * @brief Loads the neighbors from a file.
     * @param filename The filename for loading neighbors data.
     */
    void load_neighbors(const std::string& filename);

    /**
     * @brief Computes the efficiency keys.
     */
    void compute_efficiency_keys();

    /**
     * @brief Computes the land cover keys.
     */
    void compute_lc_keys();

    /**
     * @brief Computes the animal keys.
     */
    void compute_animal_keys();

    /**
     * @brief Computes the manure keys.
     */
    void compute_manure_keys();

    /**
     * @brief Computes the size of the efficiency data.
     * @return The size of the efficiency data.
     */
    size_t compute_efficiency_size();

    /**
     * @brief Computes the size of the land cover data.
     * @return The size of the land cover data.
     */
    size_t compute_lc_size();

    /**
     * @brief Computes the size of the animal data.
     * @return The size of the animal data.
     */
    size_t compute_animal_size();

    /**
     * @brief Computes the size of the manure data.
     * @return The size of the manure data.
     */
    size_t compute_manure_size();

    /**
     * @brief Computes the efficiency.
     * @return The computed efficiency.
     */
    int compute_ef();

    /**
     * @brief Gets the size of the efficiency data.
     * @return The size of the efficiency data.
     */
    size_t get_ef_size() {
        return ef_size_;
    }

    /**
     * @brief Gets the size of the land cover data.
     * @return The size of the land cover data.
     */
    size_t get_lc_size() {
        return lc_size_;
    }

    /**
     * @brief Gets the size of the animal data.
     * @return The size of the animal data.
     */
    size_t get_animal_size() {
        return animal_size_;
    }

    /**
     * @brief Gets the size of the manure data.
     * @return The size of the manure data.
     */
    size_t get_manure_size() {
        return manure_size_;
    }

    /**
     * @brief Initializes a vector with default values.
     * @param x The vector to initialize.
     */
    void initialize_vector(std::vector<double>& x);

    /**
     * @brief Initializes the scenario with given parameters.
     * @param filename The filename for loading data.
     * @param scenario_filename The filename for loading scenario configuration.
     * @param is_ef_enabled Flag to enable efficiency.
     * @param is_lc_enabled Flag to enable land cover.
     * @param is_animal_enabled Flag to enable animal.
     * @param is_manure_enabled Flag to enable manure.
     * @param manure_nutrients_file The filename for loading manure nutrients data.
     */
    void init(const std::string& filename, const std::string& scenario_filename, bool is_ef_enabled, bool is_lc_enabled, bool is_animal_enabled, bool is_manure_enabled, const std::string& manure_nutrients_file);

    /**
     * @brief Gets the number of variables.
     * @return The number of variables.
     */
    size_t get_nvars() {
        return nvars_;
    }

    /**
     * @brief Gets the beginning index of the land cover data.
     * @return The beginning index of the land cover data.
     */
    size_t get_lc_begin() {
        return lc_begin_;
    }

    /**
     * @brief Gets the beginning index of the animal data.
     * @return The beginning index of the animal data.
     */
    size_t get_animal_begin() {
        return animal_begin_;
    }

    /**
     * @brief Gets the beginning index of the manure data.
     * @return The beginning index of the manure data.
     */
    size_t get_manure_begin() {
        return manure_begin_;
    }

    /**
     * @brief Gets the scenario ID.
     * @return The scenario ID.
     */
    size_t get_scenario_id() {
        return scenario_id_;
    }

    /**
     * @brief Normalizes the land cover data.
     * @param x The input vector.
     * @param lc_x The normalized land cover data.
     * @param amount_minus The map for amount minus.
     * @param amount_plus The map for amount plus.
     * @return The normalized land cover cost.
     */
    double normalize_lc(const std::vector<double>& x, std::vector<std::tuple<int, int, int, int, double>>& lc_x, std::unordered_map<std::string, double>& amount_minus, std::unordered_map<std::string, double>& amount_plus);

    /**
     * @brief Normalizes the animal data.
     * @param x The input vector.
     * @param animal_x The normalized animal data.
     * @return The normalized animal cost.
     */
    double normalize_animal(const std::vector<double>& x, std::vector<std::tuple<int, int, int, int, int, double>>& animal_x);

    /**
     * @brief Normalizes the manure data.
     * @param x The input vector.
     * @param manure_x The normalized manure data.
     * @return The normalized manure cost.
     */
    double normalize_manure(const std::vector<double>& x, std::vector<std::tuple<int, int, int, int, int, double>>& manure_x);

    /**
     * @brief Writes the land cover data to a file.
     * @param lc_x The land cover data.
     * @param out_filename The output filename.
     * @return The status of the write operation.
     */
    int write_land(const std::vector<std::tuple<int, int, int, int, double>>& lc_x, const std::string& out_filename);

    /**
     * @brief Writes the animal data to a file.
     * @param animal_x The animal data.
     * @param out_filename The output filename.
     * @return The status of the write operation.
     */
    int write_animal(const std::vector<std::tuple<int, int, int, int, int, double>>& animal_x, const std::string& out_filename);

    /**
     * @brief Writes the manure data to a file.
     * @param manure_x The manure data.
     * @param out_filename The output filename.
     * @return The status of the write operation.
     */
    int write_manure(const std::vector<std::tuple<int, int, int, int, int, double>>& manure_x, const std::string& out_filename);

    /**
     * @brief Sends the files to a specified destination.
     * @param emo_uuid The UUID for the emotion data.
     * @param exec_uuid_vec The vector of execution UUIDs.
     * @return A vector of file paths.
     */
    std::vector<std::string> send_files(const std::string& emo_uuid, const std::vector<std::string>& exec_uuid_vec);

    /**
     * @brief Writes the land cover data to a JSON file.
     * @param lc_x The land cover data.
     * @param out_filename The output filename.
     * @return The status of the write operation.
     */
    size_t write_land_json(const std::vector<std::tuple<int, int, int, int, double>>& lc_x, const std::string& out_filename);

    /**
     * @brief Writes the animal data to a JSON file.
     * @param animal_x The animal data.
     * @param out_filename The output filename.
     * @return The status of the write operation.
     */
    size_t write_animal_json(const std::vector<std::tuple<int, int, int, int, int, double>>& animal_x, const std::string& out_filename);

    /**
     * @brief Writes the manure data to a JSON file.
     * @param manure_x The manure data.
     * @param out_filename The output filename.
     * @return The status of the write operation.
     */
    size_t write_manure_json(const std::vector<std::tuple<int, int, int, int, int, double>>& manure_x, const std::string& out_filename);

    /**
     * @brief Gets the alpha value for a given key.
     * @param key The key for which to get the alpha value.
     * @return The alpha value.
     */
    double get_alpha(std::string key) { return amount_[key]; }

    /**
     * @brief Gets the alpha map.
     * @return The alpha map.
     */
    const std::unordered_map<std::string, double> get_alpha() const { return amount_; }

    /**
     * @brief Computes the cost for a given parcel.
     * @param parcel The parcel data.
     * @return The computed cost.
     */
    double compute_cost(const std::vector<std::tuple<int, int, int, int, double>>& parcel);

    /**
     *