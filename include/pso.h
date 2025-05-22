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

/*

 * Particle Swarm Optimization class
 */
//  struct BmpRowLand {
//   int32_t  BmpSubmittedId;
//   int32_t  AgencyId;
//   std::string StateUniqueIdentifier;
//   int32_t  StateId;
//   int32_t  BmpId;
//   int32_t  GeographyId;
//   int32_t  LoadSourceGroupId;
//   int32_t  UnitId;
//   double   Amount;
//   bool     IsValid;
//   std::string ErrorMessage;
//   int32_t  RowIndex;
// };

// declaration of your reader
class PSO {
public:

    PSO(int nparts, int nobjs, int max_iter, double w, double c1, double c2, double lb, double ub, const std::string& input_filename, const std::string& scenario_filename, const std::string& out_dir, bool is_ef_enabled, bool is_lc_enabled, bool is_animal_enabled, bool is_manure_enabled,
            const std::string& manure_nutrients_file, const std::string& base_land_bmp_file, const std::string& base_animal_bmp_file,const std::string& base_manure_bmp_file, const std::string& exec_uuid);
    ~PSO();
    PSO(const PSO &p);
    PSO& operator=(const PSO &p);
    void init();
    void optimize();
    void print();

    std::vector<std::string> generate_n_uuids(int n);

    void copy_parquet_files_for_ipopt(const std::string& path, const std::string& parent_uuid, const std::vector<std::string>& uuids);
    std::vector<std::vector<double>> get_gbest_x() {
        return gbest_x;
    }
    const std::vector<std::vector<double>>& get_gbest_x_reference() {
        return gbest_x;
    }
    std::vector<std::vector<double>> get_gbest_fx() {
        return gbest_fx;
    }
    const std::vector<std::vector<double>>& get_gbest_fx_reference() {
        return gbest_fx;
    }

    const std::vector<Particle>& get_gbest() const {
        return gbest_;
    }
    void save_gbest(std::string out_dir);
    

private:
    int dim;
    int nparts;
    int nobjs;
    int max_iter;
    double w;
    double c1;
    double c2;

    Scenario scenario_;
    std::vector<Particle> particles;
    std::vector<Particle> gbest_;
    std::vector<std::vector<double>> gbest_x;
    std::vector<std::vector<double>> gbest_fx;

    double lower_bound;
    double upper_bound;
    void update_gbest();
    // CAST
    void init_cast(const std::string& input_filename, const std::string& scenario_filename, const std::string& manure_nutrients_file);
    std::string emo_uuid_;
    std::string exec_uuid_;
    int ef_size_;
    int lc_size_;
    int animal_size_;
    int manure_size_;
    
    void evaluate();
    void update_pbest();
    bool is_ef_enabled_;
    bool is_lc_enabled_;
    bool is_animal_enabled_;
    bool is_manure_enabled_;
    std::string input_filename_;
    std::string scenario_filename_;
    std::string base_land_bmp_file_;
    std::string base_animal_bmp_file_;
    std::string base_manure_bmp_file_;
    std::string out_dir_;
    Execute execute;
    std::vector<std::vector<std::string>> exec_uuid_log_;
    std::vector<BmpRowLand> base_land_bmp_inputs_;
    std::vector<std::tuple<int, int, int, int, int, double>> base_animal_bmp_inputs_;
    std::vector<std::tuple<int, int, int, int, int, double>> base_manure_bmp_inputs_;

    void exec_ipopt();
    void exec_ipopt_all_sols();
    void delete_tmp_files();

    void evaluate_ipopt_sols(const std::string& sub_dir, const std::string& ipopt_uuid, double animal_cost, double manure_cost);

    double best_lc_cost_;
    double best_animal_cost_;
    //std::shared_ptr<spdlog::logger> logger_;
};

#endif // PSO_H
