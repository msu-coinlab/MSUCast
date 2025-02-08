//
// Created by gtoscano on 4/1/23.
//

#ifndef CBO_EVALUATION_MISC_UTILITIES_H
#define CBO_EVALUATION_MISC_UTILITIES_H

#include <vector>
#include <string>
#include <tuple>
#include <nlohmann/json.hpp>

namespace misc_utilities {

    /**
    * @brief Reads environment variables.
    *
    * This function reads an environment variable.
    * If the variable is not set it returns a default value.
    *
    * @param key The environment variable's name.
    * @param default_value The default value.
    * @return Either the environment variable or the default value.
    */
    std::string get_env_var(std::string const &key, std::string const &default_value);

    /**
    * @brief Reads loads from a specified file.
    * @param loads_filename The name of the file containing the loads.
    * @return A map containing the loads.
    */
    std::unordered_map<std::string, double> read_loads(std::string loads_filename);

    /**
    * @brief Copies files with a specific prefix from the source to the destination.
    * @param source The source directory.
    * @param destination The destination directory.
    * @param prefix_in The input prefix.
    * @param prefix_out The output prefix.
    * @return True if the copying was successful.
    */
    bool copy_prefix_in_to_prefix_out(const std::string& source, const std::string& destination, const std::string& prefix_in, const std::string& prefix_out);

    /**
    * @brief Changes the extension of a given filename.
    * @param filename The original filename.
    * @param new_extension The new extension.
    * @return The filename with the new extension.
    */
    std::string change_extension(const std::string& filename, const std::string& new_extension);

    /**
    * @brief Moves files from the source directory to the destination directory.
    * @param source_dir The source directory.
    * @param destination_dir The destination directory.
    * @param n_sets The number of sets to move.
    * @param delta_counter The delta counter.
    */
    void move_files(const std::string& source_dir, const std::string& destination_dir, int n_sets, int delta_counter);

    /**
    * @brief Moves PF files from the source directory to the destination directory.
    * @param source_dir The source directory.
    * @param destination_dir The destination directory.
    * @param pf_files A vector of PF files.
    */
    void move_pf(const std::string& source_dir, const std::string& destination_dir, std::vector<std::string> pf_files);

    /**
    * @brief Lists the contents of a directory.
    * @param path The directory path.
    */
    void ls_path(std::string path);

    /**
    * @brief Extracts the path and ID from a given path string.
    * @param path The input path string.
    * @return A tuple containing the extracted path and ID.
    */
    std::tuple<std::string, std::string> extract_path_and_id(const std::string& path);

    /**
    * @brief Merges two Parquet files into a single output file.
    * @param file1 The first Parquet file.
    * @param file2 The second Parquet file.
    * @param output_file The output file.
    */
    void merge_parquet_files(const std::string& file1, const std::string& file2, const std::string& output_file);

    /**
    * @brief Finds a file with a specific prefix in a given path.
    * @param path The directory path.
    * @param prefix The file prefix.
    * @return The found file path.
    */
    std::string find_file(std::string path, std::string prefix);

    /**
    * @brief Finds files with a specific prefix in a given path.
    * @param path The directory path.
    * @param prefix The file prefix.
    * @return A vector of found file paths.
    */
    std::vector<std::string> find_files(std::string path, std::string prefix);

    /**
    * @brief Splits a string by a given delimiter.
    * @param str The input string.
    * @param delim The delimiter.
    * @param out The output vector of split strings.
    */
    void split_str(std::string const &str, const char delim, std::vector<std::string> &out);

    /**
    * @brief Copies a full directory from the source to the destination.
    * @param source The source directory.
    * @param destination The destination directory.
    * @return True if the copying was successful.
    */
    bool copy_full_directory(const std::string& source, const std::string& destination);

    /**
    * @brief Copies a file from the source to the destination.
    * @param source The source file.
    * @param destination The destination file.
    * @return True if the copying was successful.
    */
    bool copy_file(const std::string& source, const std::string& destination);

    /**
    * @brief Gets the current time as a string.
    * @return The current time string.
    */
    std::string current_time();

    /**
    * @brief Generates a random double within the given bounds.
    * @param lower_bound The lower bound.
    * @param upper_bound The upper bound.
    * @return The generated random double.
    */
    double rand_double(double lower_bound, double upper_bound);

    /**
    * @brief Creates a directory at the given path.
    * @param dir_path The directory path.
    */
    void mkdir(std::string dir_path);

    /**
    * @brief Reads a JSON file.
    * @param filename The name of the JSON file.
    * @return The read JSON object.
    */
    nlohmann::json read_json_file(const std::string& filename);

    /**
    * @brief Writes a JSON object to a file.
    * @param filename The name of the JSON file.
    * @param json_obj The JSON object to write.
    */
    void write_json_file(const std::string& filename, nlohmann::json& json_obj);

    /**
    * @brief Merges two JSON objects.
    * @param j1 The first JSON object.
    * @param j2 The second JSON object.
    * @return The merged JSON object.
    */
    nlohmann::json merge_json(const nlohmann::json& j1, const nlohmann::json& j2);

    /**
    * @brief Merges two JSON files into a single output file.
    * @param parent_json_path The path to the parent JSON file.
    * @param current_json_path The path to the current JSON file.
    * @param output_json_path The path to the output JSON file.
    */
    void merge_json_files(const std::string& parent_json_path, const std::string& current_json_path, const std::string& output_json_path);
}

#endif //CBO_EVALUATION_MISC_UTILITIES_H
