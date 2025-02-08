// Copyright (C) 2005, 2007 International Business Machines and others.
// All Rights Reserved.
// This code is published under the Eclipse Public License.
//
// Authors:  Carl Laird, Andreas Waechter     IBM    2005-08-09

#ifndef __EPA_NLP_HPP__
#define __EPA_NLP_HPP__

#include <unordered_map>
#include <sw/redis++/redis++.h>
#include "IpTNLP.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace Ipopt;

/**
 * @class EPA_NLP
 * @brief C++ Example NLP for interfacing a problem with IPOPT.
 *
 * This class provides an interface to IPOPT for solving a nonlinear programming problem.
 * It defines the problem's objective function, constraints, and bounds.
 */
class EPA_NLP: public TNLP
{

public:
    /**
     * @brief Maximum constraint value.
     */
    double max_constr;

    /**
     * @brief Total cost of the problem.
     */
    double total_cost;

    /**
     * @brief Total acres of the problem.
     */
    double total_acres;

    /**
     * @brief Status result of the problem.
     */
    int status_result;

    /**
     * @brief Index of the pollutant.
     */
    int pollutant_idx;

    /**
     * @brief Flag indicating whether the problem has content.
     */
    bool has_content;

    /**
     * @brief Default constructor.
     *
     * @param base_scenario_json Base scenario JSON data.
     * @param scenario_json Scenario JSON data.
     * @param path_out Output path.
     * @param pollutant_idx Index of the pollutant.
     */
    EPA_NLP(const json& base_scenario_json, const json& scenario_json, const std::string& path_out, int pollutant_idx);

    /**
     * @brief Default destructor.
     */
    virtual ~EPA_NLP();

    /**
     * @brief Method to return some info about the NLP.
     *
     * @param n Number of variables.
     * @param m Number of constraints.
     * @param nnz_jac_g Number of non-zero elements in the Jacobian of the constraints.
     * @param nnz_h_lag Number of non-zero elements in the Hessian of the Lagrangian.
     * @param index_style Index style.
     * @return True if successful, false otherwise.
     */
    virtual bool get_nlp_info(
        Index&          n,
        Index&          m,
        Index&          nnz_jac_g,
        Index&          nnz_h_lag,
        IndexStyleEnum& index_style
    );

    /**
     * @brief Method to write files.
     *
     * @param n Number of variables.
     * @param x Variable values.
     * @param m Number of constraints.
     * @param obj_value Objective value.
     */
    void write_files(
        Index                      n,
        const Number*              x,
        Index                      m,
        Number                     obj_value
    );

    /**
     * @brief Method to save files.
     *
     * @param n Number of variables.
     * @param x Variable values.
     */
    void save_files(
        Index                      n,
        const Number*              x
    );

    /**
     * @brief Method to save files (alternative).
     *
     * @param n Number of variables.
     * @param x Variable values.
     */
    void save_files2(
        Index                      n,
        const Number*              x
    );

    /**
     * @brief Method to write land JSON data.
     *
     * @param lc_x Land data.
     * @param out_filename Output file name.
     * @return Number of bytes written.
     */
    size_t write_land_json(
        const std::vector<std::tuple<int, int, int, int, double, int, int, int, int>>& lc_x,
        const std::string& out_filename
    );

    /**
     * @brief Method to return the bounds for my problem.
     *
     * @param n Number of variables.
     * @param x_l Lower bounds for variables.
     * @param x_u Upper bounds for variables.
     * @param m Number of constraints.
     * @param g_l Lower bounds for constraints.
     * @param g_u Upper bounds for constraints.
     * @return True if successful, false otherwise.
     */
    virtual bool get_bounds_info(
        Index   n,
        Number* x_l,
        Number* x_u,
        Index   m,
        Number* g_l,
        Number* g_u
    );

    /**
     * @brief Method to return the starting point for my problem.
     *
     * @param n Number of variables.
     * @param init_x Flag indicating whether to initialize variables.
     * @param x Variable values.
     * @param init_z Flag indicating whether to initialize bounds.
     * @param z_L Lower bounds for variables.
     * @param z_U Upper bounds for variables.
     * @param m Number of constraints.
     * @param init_lambda Flag indicating whether to initialize Lagrange multipliers.
     * @param lambda Lagrange multipliers.
     * @return True if successful, false otherwise.
     */
    virtual bool get_starting_point(
        Index   n,
        bool    init_x,
        Number* x,
        bool    init_z,
        Number* z_L,
        Number* z_U,
        Index   m,
        bool    init_lambda,
        Number* lambda
    );

    /**
     * @brief Method to return the objective value.
     *
     * @param n Number of variables.
     * @param x Variable values.
     * @param new_x Flag indicating whether the variables have changed.
     * @param obj_value Objective value.
     * @return True if successful, false otherwise.
     */
    virtual bool eval_f(
        Index         n,
        const Number* x,
        bool          new_x,
        Number&       obj_value
    );

    /**
     * @brief Method to return the gradient of the objective.
     *
     * @param n Number of variables.
     * @param x Variable values.
     * @param new_x Flag indicating whether the variables have changed.
     * @param grad_f Gradient of the objective.
     * @return True if successful, false otherwise.
     */
    virtual bool eval_grad_f(
        Index         n,
        const Number* x,
        bool          new_x,
        Number*       grad_f
    );

    /**
     * @brief Method to return the constraint residuals.
     *
     * @param n Number of variables.
     * @param x Variable values.
     * @param new_x Flag indicating whether the variables have changed.
     * @param m Number of constraints.
     * @param g Constraint residuals.
     * @return True if successful, false otherwise.
     */
    virtual bool eval_g(
        Index         n,
        const Number* x,
        bool          new_x,
        Index         m,
        Number*       g
    );

    /**
     * @brief Method to return the constraint residuals (alternative).
     *
     * @param n Number of variables.
     * @param x Variable values.
     * @param new_x Flag indicating whether the variables have changed.
     * @param m Number of constraints.
     * @param g Constraint residuals.
     * @param is_final Flag indicating whether this is the final evaluation.
     * @return True if successful, false otherwise.
     */
    virtual bool eval_g_proxy(
        Index         n,
        const Number* x,
        bool          new_x,
        Index         m,
        Number*       g,
        bool          is_final
    );

    /**
     * @brief Method to return the structure of the Jacobian of the constraints.
     *
     * @param n Number of variables.
     * @param x Variable values.
     * @param new_x Flag indicating whether the variables have changed.
     * @param m Number of constraints.
     * @param nele_jac Number of non-zero elements in the Jacobian.
     * @param iRow Row indices of the non-zero elements.
     * @param jCol Column indices of the non-zero elements.
     * @param values Values of the non-zero elements.
     * @return True if successful, false otherwise.
     */
    virtual bool eval_jac_g(
        Index         n,
        const Number* x,
        bool          new_x,
        Index         m,
        Index         nele_jac,
        Index*        iRow,
        Index*        jCol,
        Number*       values
    );

    /**
     * @brief Method to return the structure of the Hessian of the Lagrangian.
     *
     * @param n Number of variables.
     * @param x Variable values.
     * @param new_x Flag indicating whether the variables have changed.
     * @param obj_factor Objective function factor.
     * @param m Number of constraints.
     * @param lambda Lagrange multipliers.
     * @param new_lambda Flag indicating whether the Lagrange multipliers have changed.
     * @param nele_hess Number of non-zero elements in the Hessian.
     * @param iRow Row indices of the non-zero elements.
     * @param jCol Column indices of the non-zero elements.
     * @param values Values of the non-zero elements.
     * @return True if successful, false otherwise.
     */
    virtual bool eval_h(
        Index         n,
        const Number* x,
        bool          new_x,
        Number        obj_factor,
        Index         m,
        const Number* lambda,
        bool          new_lambda,
        Index         nele_hess,
        Index*        iRow,
        Index*        jCol,
        Number*       values
    );

    /**
     * @brief Method to finalize the solution.
     *
     * @param status Solver return status.
     * @param n Number of variables.
     * @param x Variable values.
     * @param z_L Lower bounds for variables.
     * @param z_U Upper bounds for variables.
     * @param m Number of constraints.
     * @param g Constraint residuals.
     * @param lambda Lagrange multipliers.
     * @param obj_value Objective value.
     * @param ip_data IPOPT data.
     * @param ip_cq IPOPT calculated quantities.
     */
    virtual void finalize_solution(
        SolverReturn               status,
        Index                      n,
        const Number*              x,
        const Number*              z_L,
        const Number*              z_U,
        Index                      m,
        const Number*              g,
        const Number*              lambda,
        Number                     obj_value,
        const IpoptData*           ip_data,
        IpoptCalculatedQuantities* ip_cq
    );

    /**
     * @brief Method to update the reduction.
     *
     * @param reduction Reduction value.
     * @param iteration Current iteration.
     */
    void update_reduction(double reduction, int iteration);

    /**
     * @brief Method to get the scenario data.
     *
     * @return Scenario data as a string.
     */
    std::string get_scenario_data();

    /**
     * @brief Method to get the UUID.
     *
     * @return UUID as a string.
     */
    std::string get_uuid();

    /**
     * @brief Method to append land data.
     *
     * @param lc_x Land data.
     */
    void append_lc_x(const std::vector<std::tuple<int, int, int, int, double, int, int, int, int>>& lc_x);

    /**
     * @brief Method to read land data from a file.
     *
     * @param filename File name.
     * @return Land data as a vector of tuples.
     */
    std::vector<std::tuple<int, int, int, int, int, int, double>> read_land(const std::string& filename);

    /**
     * @brief Method to write land data to a file.
     *
     * @param x Land data.
     * @param out_filename Output file name.
     * @return Number of bytes written.
     */
    int write_land_barefoot(
        const std::vector<std::tuple<int , int, int, int, int, int, double>>& x, 
        const std::string& out_filename
    ); 

private:
    /**@name Methods to block default compiler methods.
     *
     * The compiler automatically generates the following three methods.
     * Since the default compiler implementation is generally not what
     * you want (for all but the most simple classes), we usually
     * put the declarations of these methods in the private section
     * and never implement them. This prevents the compiler from
     * implementing an incorrect "default" behavior without us
     * knowing. (See Scott Meyers book, "Effective C++")
     */
    //@{
    EPA_NLP(
        const EPA_NLP&
    );

    EPA_NLP& operator=(
        const EPA_NLP&
    );

    // Efficiency methods
    void filter_efficiency_keys();
    void compute_efficiency_keys();
    void compute_efficiency_size();
    void normalize_efficiency();
    void compute_eta();

    void load(const json& base_scenario_json, const json& scenario_json);

    size_t ef_size_;
    size_t nvars_;
    size_t ncons_;

    std::vector<double> sum_load_invalid_;
    std::vector<double> sum_load_valid_;

    std::vector<std::string> ef_keys_;

    std::unordered_map<std::string, double> amount_;
    std::unordered_map<std::string, std::vector<std::vector<int>>> efficiency_;
    std::unordered_map<std::string, std::vector<int>> lrseg_;
    std::unordered_map<std::string, std::vector<double>> eta_dict_;
    std::unordered_map<std::string, std::vector<double>> phi_dict_;
    std::unordered_map<std::string, double> bmp_cost_;
    std::unordered_map<std::string, int> u_u_group_dict;
    std::vector<double> initial_x;

    // From misc.cpp
    std::unordered_map<int, std::vector<double>> limit_bmps_dict;
    std::unordered_map<int, std::vector<double>> limit_alpha_dict;
    std::unordered_map<int, std::vector<int>> limit_vars_dict;

    std::vector<std::tuple<int, int, int, int, double, int, int, int, int>> ef_x_;
    std::vector<std::tuple<int, int, int, int, double, int, int, int, int>> lc_x_;
    std::string uuid_;

    int write_land(
        const std::vector<std::tuple<int, int, int, int, double, int, int, int, int>>& lc_x,
        const std::string& out_filename
    ); 

    int current_iteration_;
    std::string scenario_data_;
    std::string path_out_;
    //@}
};

#endif