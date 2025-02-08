#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fmt/core.h>



/**
 * @class Execute
 * @brief A class to handle execution of different scenarios and operations.
 */
class Execute {
public:
    /**
     * @brief Default constructor.
     */
    Execute() = default;

    /**
     * @brief Sets the files for the given UUID and report loads filename.
     * @param emo_uuid The UUID for the emotion data.
     * @param report_loads_filename The filename for the report loads.
     */
    void set_files(const std::string& emo_uuid, const std::string& report_loads_filename);

    /**
     * @brief Gets the files for the given UUID and path.
     * @param emo_uuid The UUID for the emotion data.
     * @param path_to The path to get the files from.
     */
    void get_files(const std::string& emo_uuid, const std::string& path_to);

    /**
     * @brief Executes the given parameters.
     * @param emo_uuid The UUID for the emotion data.
     * @param ipopt_reduction The reduction parameter for IPOPT.
     * @param cost_profile_idx The index for the cost profile.
     * @param ipopt_popsize The population size for IPOPT.
     */
    void execute(const std::string& emo_uuid, double ipopt_reduction, int cost_profile_idx, int ipopt_popsize);

    /**
     * @brief Executes locally with the given parameters.
     * @param in_path The input path.
     * @param out_path The output path.
     * @param pollutant_idx The index for the pollutant.
     * @param ipopt_reduction The reduction parameter for IPOPT.
     * @param ipopt_popsize The population size for IPOPT.
     */
    void execute_local(
        const std::string& in_path,
        const std::string& out_path,
        int pollutant_idx, //0
        double ipopt_reduction, //0.30
        int ipopt_popsize //10
    );

    /**
     * @brief Executes a new scenario with the given parameters.
     * @param base_scenario The base scenario name.
     * @param scenario The new scenario name.
     * @param out_path The output path.
     * @param pollutant_idx The index for the pollutant.
     * @param ipopt_reduction The reduction parameter for IPOPT.
     * @param nsteps The number of steps.
     * @param evaluate_cast The evaluation cast parameter.
     * @param original_base_scenario The original base scenario name.
     */
    void execute_new(
        const std::string& base_scenario,
        const std::string& scenario,
        const std::string& out_path,
        int pollutant_idx, //0
        double ipopt_reduction, //0.30
        int nsteps,
        int evaluate_cast,
        std::string original_base_scenario
    );

    /**
     * @brief Gets the JSON scenario with the given parameters.
     * @param sinfo The scenario info size.
     * @param report_loads_path The path to report loads.
     * @param output_path_prefix The prefix for the output path.
     */
    void get_json_scenario(size_t sinfo, const std::string& report_loads_path, const std::string& output_path_prefix);

    /**
     * @brief Updates the output with the given parameters.
     * @param emo_uuid The UUID for the emotion data.
     * @param initial_cost The initial cost parameter.
     */
    void update_output(const std::string& emo_uuid, double initial_cost);

    /**
     * @brief Processes a file with the given parameters.
     * @param source_path The source path of the file.
     * @param destination_path The destination path of the file.
     * @param initial_cost The initial cost parameter.
     * @return True if the file was processed successfully.
     */
    bool process_file(const std::string& source_path, const std::string& destination_path, float initial_cost);
};
