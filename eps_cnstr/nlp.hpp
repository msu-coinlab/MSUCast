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

/** C++ Example NLP for interfacing a problem with IPOPT.
 *
 */
class EPA_NLP: public TNLP
{

public:
    double max_constr;
    double total_cost;
    double total_acres;
    int status_result;
    int pollutant_idx;
    bool has_content;
   /** Default constructor */

    EPA_NLP(const json& base_scenario_json, const json& scenario_json, const std::string& path_out, int pollutant_idx);
   /** Default destructor */
   virtual ~EPA_NLP();

   /**@name Overloaded from TNLP */
   //@{
   /** Method to return some info about the NLP */
   virtual bool get_nlp_info(
      Index&          n,
      Index&          m,
      Index&          nnz_jac_g,
      Index&          nnz_h_lag,
      IndexStyleEnum& index_style
   );
    
    void write_files(
        Index                      n,
        const Number*              x,
        Index                      m,
        Number                     obj_value
        );

    void save_files(
        Index                      n,
        const Number*              x
    );

    void save_files2(
        Index                      n,
        const Number*              x
    );

    size_t write_land_json(
        const std::vector<std::tuple<int, int, int, int, double, int, int, int, int>>& lc_x,
        const std::string& out_filename
    ); 

   /** Method to return the bounds for my problem */
   virtual bool get_bounds_info(
      Index   n,
      Number* x_l,
      Number* x_u,
      Index   m,
      Number* g_l,
      Number* g_u
   );



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
   /** Method to return the objective value */
   virtual bool eval_f(
      Index         n,
      const Number* x,
      bool          new_x,
      Number&       obj_value
   );

   /** Method to return the gradient of the objective */
   virtual bool eval_grad_f(
      Index         n,
      const Number* x,
      bool          new_x,
      Number*       grad_f
   );

   /** Method to return the constraint residuals */
   virtual bool eval_g(
      Index         n,
      const Number* x,
      bool          new_x,
      Index         m,
      Number*       g
   );
   /** Method to return the constraint residuals */
   virtual bool eval_g_proxy(
      Index         n,
      const Number* x,
      bool          new_x,
      Index         m,
      Number*       g,
      bool          is_final
   );
   
   /** Method to return:
    *   1) The structure of the jacobian (if "values" is NULL)
    *   2) The values of the jacobian (if "values" is not NULL)
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

   /** Method to return:
    *   1) The structure of the hessian of the lagrangian (if "values" is NULL)
    *   2) The values of the hessian of the lagrangian (if "values" is not NULL)
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

   /** This method is called when the algorithm is complete so the TNLP can store/write the solution */
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
   //@}

   void update_reduction(double,int);

    std::string get_scenario_data();
    std::string get_uuid();
    void append_lc_x(const std::vector<std::tuple<int, int, int, int, double, int, int, int, int>>& lc_x);

    std::vector<std::tuple<int, int, int, int, int, int, double>> read_land(const std::string& filename);

    int write_land_barefoot(
        const std::vector<std::tuple<int, int, int, int, int, int, double>>& x, 
        const std::string& out_filename
    ); 
private:
   /**@name Methods to block default compiler methods.
    *
    * The compiler automatically generates the following three methods.
    *  Since the default compiler implementation is generally not what
    *  you want (for all but the most simple classes), we usually
    *  put the declarations of these methods in the private section
    *  and never implement them. This prevents the compiler from
    *  implementing an incorrect "default" behavior without us
    *  knowing. (See Scott Meyers book, "Effective C++")
    */
   //@{
   EPA_NLP(
      const EPA_NLP&
   );

   EPA_NLP& operator=(
      const EPA_NLP&
   );
   //Efficiency
   //
    void filter_efficiency_keys();
    void compute_efficiency_keys();
    void compute_efficiency_size();
    void normalize_efficiency();
    //int compute_efficiency();
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
    //a partir de aqui hay que revisar
    std::unordered_map<std::string, std::vector<int>> lrseg_;
    std::unordered_map<std::string, std::vector<double>> eta_dict_;
    std::unordered_map<std::string, std::vector<double>> phi_dict_;
    std::unordered_map<std::string, double> bmp_cost_;
    std::unordered_map<std::string, int> u_u_group_dict;
    std::vector<double> initial_x;

    //From misc.cpp
    std::unordered_map<int, std::vector<double>> limit_bmps_dict;
    std::unordered_map<int, std::vector<double> > limit_alpha_dict;
    std::unordered_map<int, std::vector<int> > limit_vars_dict;

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
