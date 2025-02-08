#pragma once

// Include necessary libraries for input/output, string manipulation, and vector operations
#include <iostream>
#include <string>
#include <vector>
// Include libraries for smart pointers and IPOPT application
#include <coin-or/IpSmartPtr.hpp>
#include <coin-or/IpIpoptApplication.hpp>
// Include custom NLP class
#include "nlp.hpp"
// Include JSON library for parsing and generating JSON data
#include <nlohmann/json.hpp>

// Define a macro to prevent multiple inclusions of this header file
#ifndef EPS_CNSTR_HPP
#define EPS_CNSTR_HPP

// Use the nlohmann namespace for JSON operations
using json = nlohmann::json;

/**
 * @brief Class for evaluating constraints in the optimization problem.
 */
class EpsConstraint {
    // Private member variables
    // Path for output files
    std::string path_out_;
    // Flag to evaluate cast
    bool evaluate_cast_;
    // Smart pointer to EPA_NLP object
    SmartPtr <EPA_NLP> mynlp;
    // Smart pointer to IpoptApplication object
    SmartPtr <IpoptApplication> app;

    // Public member functions
public:
    /**
     * @brief Constructor for EpsConstraint class.
     * 
     * @param base_scenario_json The base scenario JSON object.
     * @param scenario_json The scenario JSON object.
     * @param path_out The output path.
     * @param pollutant_idx The pollutant index.
     * @param evaluate_cast Whether to evaluate the cast.
     */
    EpsConstraint(const json& base_scenario_json, const json& scenario_json, const std::string& path_out, int pollutant_idx, bool evaluate_cast);

    /**
     * @brief Evaluates the constraints.
     * 
     * @param reduction The reduction value.
     * @param nsteps The number of steps.
     * @param uuids The list of UUIDs.
     * @param parent_uuid_path The parent UUID path.
     * @return bool Whether the evaluation was successful.
     */
    bool constr_eval(double reduction, int nsteps, const std::vector<std::string>& uuids, const std::string& parent_uuid_path);

    /**
     * @brief Evaluates a single value.
     * 
     * @param value The value to evaluate.
     * @param int The integer parameter.
     * @return bool Whether the evaluation was successful.
     */
    bool evaluate(double, int);

    /**
     * @brief Sends files to the RabbitMQ server.
     * 
     * @param scenario_data The scenario data.
     * @param uuid The UUID.
     * @param uuids The list of UUIDs.
     * @return std::vector<std::string> The list of output strings.
     */
    std::vector<std::string> send_files(const std::string& scenario_data, const std::string& uuid, const std::vector<std::string>& uuids);
};

#endif
