/**
 * @file misc_utilities.cpp
 * @brief Collection of miscellaneous utility functions for file management, JSON manipulation,
 *        Parquet file handling, and other helper operations.
 *
 * This file provides various utility functions including reading and merging Parquet files,
 * file/directory operations, random number generation, and JSON processing.
 *
 * @author
 * Gregorio Toscano
 * @date 2023-04-01
 */

 #include <tuple>
 #include <filesystem>
 #include <iostream>
 #include <string>
 #include <vector>
 #include <chrono>
 #include <ctime>
 #include <random>
 #include <fstream>
 #include <nlohmann/json.hpp>
 #include <unordered_map>
 
 #include <parquet/arrow/writer.h>
 #include <parquet/exception.h>
 #include <fmt/core.h>
 
 #include <arrow/api.h>
 #include <arrow/io/api.h>
 #include <arrow/compute/api_aggregate.h>
 #include <parquet/arrow/reader.h>
 #include <memory>
 
 using json = nlohmann::json;
 namespace fs = std::filesystem;
 
 namespace misc_utilities {
 
 /**
  * @brief Reads loads from a Parquet file and returns them in a map.
  *
  * This function opens a Parquet file, reads specific columns (indices 7 to 15), computes the sum
  * for each column, and maps the sums to corresponding load keys.
  *
  * @param loads_filename The path to the Parquet file containing load data.
  * @return std::unordered_map<std::string, double> A map where keys represent load identifiers and values are their corresponding sum.
  */
 std::unordered_map<std::string, double> read_loads(std::string loads_filename) {
     std::shared_ptr<arrow::io::ReadableFile> infile;
     PARQUET_ASSIGN_OR_THROW(infile, arrow::io::ReadableFile::Open(loads_filename, arrow::default_memory_pool()));
     std::unique_ptr<parquet::arrow::FileReader> reader;
     PARQUET_THROW_NOT_OK(parquet::arrow::OpenFile(infile, arrow::default_memory_pool(), &reader));
     std::shared_ptr<arrow::ChunkedArray> array;
     arrow::Datum sum;
 
     std::unordered_map<std::string, double> loads;
     std::vector<double> output_tmp;
 
     for (int col = 7; col < 16; ++col) {
         PARQUET_THROW_NOT_OK(reader->ReadColumn(col, &array));
         PARQUET_ASSIGN_OR_THROW(sum, arrow::compute::Sum(array));
 
         auto sum_scalar = std::dynamic_pointer_cast<arrow::DoubleScalar>(sum.scalar());
         if (!sum_scalar) {
             fmt::print("Error: Sum is not a DoubleScalar\n");
             return {};
         }
 
         double val = sum_scalar->value;
         output_tmp.push_back(val);
     }
 
     try {
         loads["EoS-N"] = output_tmp[0];
         loads["EoS-P"] = output_tmp[1];
         loads["EoS-S"] = output_tmp[2];
         loads["EoR-N"] = output_tmp[3];
         loads["EoR-P"] = output_tmp[4];
         loads["EoR-S"] = output_tmp[5];
         loads["EoT-N"] = output_tmp[6];
         loads["EoT-P"] = output_tmp[7];
         loads["EoT-S"] = output_tmp[8];
     }
     catch (const std::exception& e) {
         fmt::print("Error: {}\n", e.what());
     }
 
     return loads;
 }
 
 /**
  * @brief Changes the file extension of the given filename.
  *
  * If the filename already has an extension, it will be replaced with the new extension.
  * Otherwise, the new extension is appended to the filename.
  *
  * @param filename The original filename.
  * @param new_extension The new extension to apply (include the '.' if desired).
  * @return std::string The filename with the new extension.
  */
 std::string change_extension(const std::string& filename, const std::string& new_extension) {
     std::string new_filename = filename;
     
     // Find the last period in the filename, which is likely the start of the extension
     size_t last_dot = new_filename.find_last_of('.');
     
     if (last_dot != std::string::npos) {
         // Replace the existing extension with the new one
         new_filename = new_filename.substr(0, last_dot) + new_extension;
     } else {
         // If there's no extension, just append the new extension
         new_filename += new_extension;
     }
     
     return new_filename;
 }
 
 /**
  * @brief Moves a set of files from a source directory to a destination directory.
  *
  * The function constructs source and destination filenames based on a list of predefined postfixes.
  *
  * @param source_dir Directory containing the source files.
  * @param destination_dir Directory where the files should be moved.
  * @param n_sols Number of solutions (indexed files) to move.
  * @param delta_counter A counter value added to the index for destination filenames.
  */
 void move_files(const std::string& source_dir, const std::string& destination_dir, int n_sols, int delta_counter) {
     std::vector<std::string> source_files = {"_impbmpsubmittedland.json", "_impbmpsubmittedland.parquet", "_impbmpsubmittedanimal.json", "_impbmpsubmittedanimal.parquet", "_impbmpsubmittedmanuretransport.json", "_impbmpsubmittedmanuretransport.parquet", "_reportloads.csv", "_reportloads.parquet", "_costs.json", ".csv"};
     try {
         // Create the destination directory if it doesn't exist
         if (!fs::exists(destination_dir)) {
             fs::create_directory(destination_dir);
         }
         for (int i = 0; i < n_sols; i++) {
             for (const auto& file_postfix : source_files) {
                 // Construct the source and destination paths
                 std::string source_file = fmt::format("{}/{}{}", source_dir,  i,  file_postfix);
                 std::string dest_file = fmt::format("{}/{}{}", destination_dir,  i+delta_counter,  file_postfix);
     
                 fmt::print("Moving file: {} to {}\n", source_file, dest_file);
                 // Move the file
                 if (fs::exists(source_file)) {
                     fs::rename(source_file, dest_file);
                 }
             }
         }
     
     
     } catch (const fs::filesystem_error& e) {
         std::cerr << "Error moving files: " << e.what() << std::endl;
     }
 }
 
 /**
  * @brief Moves files with specific prefixes from a source directory to a destination directory.
  *
  * For each file prefix provided in pf_files, the function constructs source filenames using
  * a list of predefined postfixes and moves them to the destination directory.
  *
  * @param source_dir Directory containing the source files.
  * @param destination_dir Destination directory where the files will be moved.
  * @param pf_files Vector of file prefixes to process.
  */
 void move_pf(const std::string& source_dir, const std::string& destination_dir, std::vector<std::string> pf_files) {
     std::vector<std::string> source_files = {"_impbmpsubmittedland.json", "_impbmpsubmittedland.parquet", "_impbmpsubmittedanimal.json", "_impbmpsubmittedanimal.parquet", "_impbmpsubmittedmanuretransport.json", "_impbmpsubmittedmanuretransport.parquet", "_reportloads.csv", "_reportloads.parquet", "_costs.json", ".csv"};
     try {
         // Create the destination directory if it doesn't exist
         if (!fs::exists(destination_dir)) {
             fs::create_directory(destination_dir);
         }
         // Iterate through the file prefixes
         int iter = 0;
         for (const auto& file_prefix: pf_files) {
             for (const auto& file_postfix : source_files) {
                 // Construct the source and destination paths
                 std::string source_file = fmt::format("{}/{}{}", source_dir,  file_prefix,  file_postfix);
                 std::string dest_file = fmt::format("{}/{}{}", destination_dir,  iter,  file_postfix);
     
                 fmt::print("Moving file: {} to {}\n", source_file, dest_file);
                 // Move the file
                 if (fs::exists(source_file)) {
                     fs::rename(source_file, dest_file);
                 }
             }
             ++iter;
         }
     
     
     } catch (const fs::filesystem_error& e) {
         std::cerr << "Error moving files: " << e.what() << std::endl;
     }
 }
 
 /**
  * @brief Moves all files from one directory to another.
  *
  * The function iterates over all entries in the source directory, moves each file to the destination,
  * copies it, and then removes the original file.
  *
  * @param from Source directory.
  * @param to Destination directory.
  */
 void move_files_from_to(std::string from, std::string to) {
     for (const auto& entry : fs::directory_iterator(from)) {
         fs::rename(entry.path(), to);
         fs::copy(entry.path(), to);
         fs::remove(entry.path());
     }
 }
 
 /**
  * @brief Lists all files in a given directory.
  *
  * Prints the file paths of all entries found in the specified directory.
  *
  * @param path The directory path to list.
  */
 void ls_path(std::string path) {
     std::cout << "*******************Listing files in " << path << std::endl;
     for (const auto& entry : fs::directory_iterator(path)) {
         std::cout << entry.path() << std::endl;
     }
 }
 
 /**
  * @brief Extracts the directory path and file ID from a full path.
  *
  * Splits the input string at the last '/' character. If no '/' is found, the original path is returned
  * with an empty ID.
  *
  * @param path The full file path.
  * @return std::tuple<std::string, std::string> A tuple where the first element is the directory path and
  *         the second element is the file ID.
  */
 std::tuple<std::string, std::string> extract_path_and_id(const std::string& path) {
     // Find the position of the last '/'
     size_t pos = path.find_last_of('/');
     
     // If no '/' is found, return the original path and an empty ID
     if (pos == std::string::npos) {
         return std::make_tuple(path, "");
     }
     
     // Separate the path up to the last '/' and the ID after the '/'
     std::string path_part = path.substr(0, pos);
     std::string id_part = path.substr(pos + 1);
     
     return std::make_tuple(path_part, id_part);
 }
 
 /**
  * @brief Merges two Parquet files into one.
  *
  * Opens two Parquet files, reads their tables, concatenates them, and writes the combined table
  * to a new Parquet file.
  *
  * @param file1 Path to the first Parquet file.
  * @param file2 Path to the second Parquet file.
  * @param output_file Path where the merged Parquet file will be written.
  */
 void merge_parquet_files(const std::string& file1, const std::string& file2, const std::string& output_file) {
     // Open first Parquet file
     std::shared_ptr<arrow::Table> table1;
     {
         std::shared_ptr<arrow::io::ReadableFile> infile1;
         PARQUET_ASSIGN_OR_THROW(infile1, arrow::io::ReadableFile::Open(file1));
         
         std::unique_ptr<parquet::arrow::FileReader> arrow_reader1;
         PARQUET_THROW_NOT_OK(parquet::arrow::OpenFile(infile1, arrow::default_memory_pool(), &arrow_reader1));
         
         PARQUET_THROW_NOT_OK(arrow_reader1->ReadTable(&table1));
     }
     
     // Open second Parquet file
     std::shared_ptr<arrow::Table> table2;
     {
         std::shared_ptr<arrow::io::ReadableFile> infile2;
         PARQUET_ASSIGN_OR_THROW(infile2, arrow::io::ReadableFile::Open(file2));
         
         std::unique_ptr<parquet::arrow::FileReader> arrow_reader2;
         PARQUET_THROW_NOT_OK(parquet::arrow::OpenFile(infile2, arrow::default_memory_pool(), &arrow_reader2));
         
         PARQUET_THROW_NOT_OK(arrow_reader2->ReadTable(&table2));
     }
     
     // Concatenate the two tables
     std::shared_ptr<arrow::Table> combined_table;
     PARQUET_ASSIGN_OR_THROW(combined_table, arrow::ConcatenateTables({table1, table2}, arrow::ConcatenateTablesOptions::Defaults(), arrow::default_memory_pool()));
     
     // Specify the Parquet version and write the combined table to a new Parquet file
     parquet::WriterProperties::Builder builder;
     builder.version(parquet::ParquetVersion::PARQUET_1_0); // Set Parquet version to 1.0
     std::shared_ptr<parquet::WriterProperties> props = builder.build();
     
     std::shared_ptr<arrow::io::FileOutputStream> outfile;
     PARQUET_ASSIGN_OR_THROW(outfile, arrow::io::FileOutputStream::Open(output_file));
     
     PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*combined_table, arrow::default_memory_pool(), outfile, 1024, props));
 }
 
 /**
  * @brief Checks if a given file is a Parquet file.
  *
  * The function reads the first 4 bytes (header) and the last 4 bytes (footer) of the file to verify
  * if they match the Parquet signature ("PAR1").
  *
  * @param filename The file to check.
  * @return true if the file is a valid Parquet file; false otherwise.
  */
 bool is_parquet_file(const std::string& filename) {
     std::ifstream file(filename, std::ios::binary);
 
     if (!file) {
         std::cerr << "Failed to open the file: " << filename << ".\n";
         return false;
     }
 
     char header[4];
     char footer[4];
 
     file.read(header, sizeof(header));
 
     if (!file.seekg(-4, std::ios::end)) {
         std::cerr << "File too small.\n";
         return false;
     }
 
     file.read(footer, sizeof(footer));
 
     return std::string_view(header, 4) == "PAR1" && std::string_view(footer, 4) == "PAR1";
 }
 
 /**
  * @brief Retrieves the value of an environment variable.
  *
  * Returns the value of the environment variable if it exists; otherwise, returns the provided default value.
  *
  * @param key The environment variable key.
  * @param default_value The default value to return if the environment variable is not set.
  * @return std::string The environment variable's value or the default value.
  */
 std::string get_env_var(std::string const &key, std::string const &default_value) {
     const char *val = std::getenv(key.c_str());
     return val == nullptr ? std::string(default_value) : std::string(val);
 }
 
 /**
  * @brief Finds the first CSV file in a directory with a given prefix.
  *
  * Searches through the specified directory for a regular file with the ".csv" extension whose stem
  * starts with the provided prefix.
  *
  * @param path The directory path to search.
  * @param prefix The prefix that the filename should start with.
  * @return std::string The found filename, or an empty string if no matching file is found.
  */
 std::string find_file(std::string path, std::string prefix) {
     std::string found_filename;
     for (const auto& entry : fs::directory_iterator(path)) {
         if (entry.is_regular_file()
             && entry.path().extension() == ".csv"
             && entry.path().stem().string().starts_with(prefix)) {
             found_filename = entry.path();
             break;
         }
     }
     return found_filename;
 }
 
 /**
  * @brief Finds all files in a directory with a given prefix.
  *
  * Searches the directory for regular files whose stem starts with the specified prefix.
  *
  * @param path The directory path to search.
  * @param prefix The prefix that the filenames should start with.
  * @return std::vector<std::string> A vector containing the filenames that match the prefix.
  */
 std::vector<std::string> find_files(std::string path, std::string prefix) {
     std::vector<std::string> found_filenames;
     for (const auto& entry : fs::directory_iterator(path)) {
         if (entry.is_regular_file()
             //&& entry.path().extension() == ".csv"
             && entry.path().stem().string().starts_with(prefix)) {
             found_filenames.emplace_back(entry.path().filename());
         }
     }
     return found_filenames;
 }
 
 /**
  * @brief Splits a string into tokens based on a specified delimiter.
  *
  * The resulting tokens are stored in the provided output vector.
  *
  * @param str The string to split.
  * @param delim The delimiter character used to split the string.
  * @param[out] out The vector that will contain the split tokens.
  */
 void split_str(std::string const &str, const char delim,
                std::vector<std::string> &out) {
     out.clear();
 
     std::stringstream s(str);
 
     std::string s2;
 
     while (std::getline(s, s2, delim)) {
         out.push_back(s2); // store the token
     }
 }
 
 /**
  * @brief Copies an entire directory recursively.
  *
  * Checks if the source exists and is a directory, then copies its entire contents to the destination.
  *
  * @param source The source directory.
  * @param destination The destination directory.
  * @return true if the copy operation was successful; false otherwise.
  */
 bool copy_full_directory(const std::string& source, const std::string& destination) {
     try {
         // Check if the source directory exists
         if (!fs::exists(source)) {
             std::cerr << "Error: Source directory does not exist" << std::endl;
             return false;
         }
 
         // Check if the source is actually a directory
         if (!fs::is_directory(source)) {
             std::cerr << "Error: Source is not a directory" << std::endl;
             return false;
         }
 
         // Create the destination directory if it doesn't exist
         if (!fs::exists(destination)) {
             fs::create_directories(destination);
         }
 
         // Copy the entire directory
         fs::copy(source, destination, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
         
         return true;
     } catch (const std::exception& ex) {
         std::cerr << "Error: " << ex.what() << std::endl;
         return false;
     }
 }
 
 /**
  * @brief Copies a single file from the source to the destination.
  *
  * @param source Path to the source file.
  * @param destination Path where the file should be copied.
  * @return true if the file was successfully copied; false otherwise.
  */
 bool copy_file(const std::string& source, const std::string& destination) {
     try {
         // Check if the source file exists
         std::cout << "source: " << source << std::endl;
         if (!fs::exists(source)) {
             std::cerr << "Error: Source file does not exist: " << source << std::endl;
             return false;
         }
 
         // Copy the file to the destination
         fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
         return true;
     } catch (const std::exception& ex) {
         std::cerr << "Error: " << ex.what() << std::endl;
         return false;
     }
 }
 
 /**
  * @brief Copies files with specific extensions from a source directory to a destination directory.
  *
  * Only files with extensions ".json", ".parquet", and ".csv" are copied.
  *
  * @param source The source directory.
  * @param destination The destination directory.
  * @return true if the files were successfully copied; false otherwise.
  */
 bool copy_files(const std::string& source, const std::string& destination) {
     try {
         // Check if the source directory exists
         std::vector<std::string> extensions = {".json", ".parquet", ".csv"};
         for (const auto& entry : fs::directory_iterator(source)) {
             if (entry.is_regular_file()
                 && std::find(extensions.begin(), extensions.end(), entry.path().extension().string()) != extensions.end()) {
                 
                 // Convert the path components to strings before formatting
                 std::string dest_file = fmt::format("{}/{}", destination, entry.path().filename().string());
                 fs::copy_file(entry.path(), dest_file, fs::copy_options::overwrite_existing);
             }
         }
     } catch (const std::exception& ex) {
         std::cerr << "Error: " << ex.what() << std::endl;
         return false;
     }
     return true;
 }
 
 /**
  * @brief Copies files from a source directory to a destination directory, changing the file prefix.
  *
  * Files that have a filename starting with `prefix_in` are copied to the destination with their prefix
  * replaced by `prefix_out`.
  *
  * @param source The source directory.
  * @param destination The destination directory.
  * @param prefix_in The original prefix to match.
  * @param prefix_out The new prefix to use for the copied files.
  * @return true if the operation was successful; false otherwise.
  */
 bool copy_prefix_in_to_prefix_out(const std::string& source, const std::string& destination, const std::string& prefix_in, const std::string& prefix_out) {
     try {
         std::vector<std::string> extensions = {".json", ".parquet", ".csv"}; 
 
         for (const auto& entry : fs::directory_iterator(source)) {
             if (entry.is_regular_file()
                 && std::find(extensions.begin(), extensions.end(), entry.path().extension().string()) != extensions.end()
                 && entry.path().stem().string().starts_with(prefix_in)) {
                 
                 // Replace prefix_in with prefix_out in the stem
                 std::string new_filename = entry.path().stem().string();
                 new_filename.replace(0, prefix_in.size(), prefix_out);
 
                 // Create the full destination path
                 std::string dest_file = fmt::format("{}/{}{}", destination, new_filename, entry.path().extension().string());
 
                 // Copy the file to the destination with the new name
                 fs::copy_file(entry.path(), dest_file, fs::copy_options::overwrite_existing);
             }
         }
         return true;
     } catch (const std::exception& ex) {
         std::cerr << "Error: " << ex.what() << std::endl;
         return false;
     }
 }
 
 /**
  * @brief Gets the current system time as a formatted string.
  *
  * The time is formatted as "HH:MM:SS".
  *
  * @return std::string The current time.
  */
 std::string current_time() {
     // Get the current time
     auto now = std::chrono::system_clock::now();
 
     // Convert the time to a tm struct
     std::time_t now_c = std::chrono::system_clock::to_time_t(now);
     std::tm now_tm = *std::localtime(&now_c);
 
     // Format the time as a string
     std::ostringstream oss;
     oss << std::put_time(&now_tm, "%H:%M:%S");
     return oss.str();
 }
 
 std::random_device rd;
 std::mt19937 gen(rd());
 std::uniform_real_distribution<> dis(0, 1);
 
 /**
  * @brief Generates a random double between the specified lower and upper bounds.
  *
  * @param lower_bound The lower bound of the range.
  * @param upper_bound The upper bound of the range.
  * @return double A random double within the specified range.
  */
 double rand_double(double lower_bound, double upper_bound) {
     return lower_bound + dis(gen) * (upper_bound - lower_bound);
 }
 
 /**
  * @brief Creates a directory if it does not exist.
  *
  * If the directory already exists, no action is taken.
  *
  * @param dir_path The path of the directory to create.
  */
 void mkdir(std::string dir_path) {
     if (!fs::exists(dir_path)) {
         if (fs::create_directories(dir_path)) {
             std::cout << "Directory created successfully." << std::endl;
         } else {
             std::cerr << "Failed to open the directory: " << dir_path << ".\n";
         }
     } else {
         std::cout << "Directory already exists. Doing nothing." << std::endl;
     }
 }
 
 /**
  * @brief Reads a JSON file and parses its content.
  *
  * @param filename The path to the JSON file.
  * @return json The parsed JSON object. Returns an empty JSON object if reading fails.
  */
 json read_json_file(const std::string& filename) {
     // Open the JSON file
     std::ifstream file(filename);
     if (!file.is_open()) {
         std::cerr << "Failed to open the base scenario file: " << filename << std::endl;
         return json{}; // Return an empty JSON object
     }
 
     // Parse the JSON file directly into a nlohmann::json object
     json json_obj;
     try {
         json_obj = json::parse(file);
     } catch (const json::parse_error& e) {
         throw std::runtime_error("Failed to parse JSON file: " + filename + "; error: " + e.what());
         json_obj = json{}; // Return an empty JSON object
     }
     file.close();
     return json_obj;
 }
 
 /**
  * @brief Writes a JSON object to a file.
  *
  * @param filename The path to the output file.
  * @param json_obj The JSON object to write.
  *
  * @throws std::runtime_error if the file cannot be opened.
  */
 void write_json_file(const std::string& filename, json& json_obj) {
     std::ofstream file(filename);
     if (!file.is_open()) {
         throw std::runtime_error("Could not open file: " + filename);
     }
 
     file << json_obj.dump(0); // Use no indentation; change the parameter to prettify if needed
     file.close();
 }
 
 /**
  * @brief Merges two JSON objects.
  *
  * Keys from the second JSON object will overwrite those in the first if they exist.
  *
  * @param j1 The first JSON object.
  * @param j2 The second JSON object.
  * @return json The merged JSON object.
  */
 json merge_json(const json& j1, const json& j2) {
     json result = j1;
     for (auto& el : j2.items()) {
         result[el.key()] = el.value();
     }
     return result;
 }
 
 /**
  * @brief Reads and modifies a JSON file string for merging.
  *
  * If this is the first file in a merge sequence, the function replaces the last closing brace with a comma.
  * Otherwise, it removes the first opening brace.
  *
  * @param file_path The path to the JSON file.
  * @param is_first_file Flag indicating if this is the first file in the merge.
  * @return std::string The modified JSON string.
  */
 std::string read_and_modify_json(const std::string& file_path, bool is_first_file) {
     std::ifstream file(file_path);
     std::string json_str((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
     file.close();
 
     if (is_first_file) {
         // Replace the last closing brace with a comma
         size_t pos = json_str.rfind('}');
         if (pos != std::string::npos) {
             json_str.replace(pos, 1, ",");
         }
     } else {
         // Remove the first opening brace
         size_t pos = json_str.find('{');
         if (pos != std::string::npos) {
             json_str.erase(pos, 1);
         }
     }
 
     return json_str;
 }
 
 /**
  * @brief Merges two JSON files into one.
  *
  * Reads and modifies two JSON files to concatenate them into a valid JSON object, then writes the merged object to an output file.
  *
  * @param parent_json_path The path to the parent JSON file.
  * @param current_json_path The path to the current JSON file.
  * @param output_json_path The path where the merged JSON file will be saved.
  */
 void merge_json_files(const std::string& parent_json_path, 
                       const std::string& current_json_path, 
                       const std::string& output_json_path) {
     // Read and modify both JSON files
     std::string parent_json_str = read_and_modify_json(parent_json_path, true);
     std::string current_json_str = read_and_modify_json(current_json_path, false);
 
     // Concatenate the modified strings
     std::string merged_json_str = parent_json_str + current_json_str;
 
     // Parse the concatenated string into a JSON object to validate it
     nlohmann::json merged_json = nlohmann::json::parse(merged_json_str);
 
     // Output the merged JSON to the specified file
     std::ofstream output_file(output_json_path);
     if (output_file.is_open()) {
         output_file << merged_json.dump(0); // Save with no indentation; adjust parameter for pretty printing if desired
         output_file.close();
         std::cout << "Merged JSON saved to " << output_json_path << std::endl;
     } else {
         std::cerr << "Failed to open the output file!" << std::endl;
     }
 }
 
 /*
 // The following function is commented out.
 // double compute_cost(const std::string& loads_filename, int cost_profile_id) {
 //     // Function implementation for computing cost from load data.
 // }
 */
 
 } // namespace misc_utilities
 