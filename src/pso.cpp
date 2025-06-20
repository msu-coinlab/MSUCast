#include <stdio.h>
#include <iostream>


//#include "pso.h"

#include <iostream>
#include <random>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>
#include <functional> // For std::reference_wrapper
#include <fstream>


#include <range/v3/all.hpp>

#include "external_archive.h"
#include "particle.h"
#include "pso.h"
#include "scenario.h"
#include "misc_utilities.h"

#include <crossguid/guid.hpp>
#include <fmt/core.h>
#include <regex>
#include <filesystem>
#include <boost/algorithm/string.hpp>
#include <optional>
#include <fstream>
#include <algorithm>


#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/array.h>
#include <arrow/array/concatenate.h>
#include <arrow/io/file.h>
#include <arrow/status.h>
#include <arrow/table.h>
#include <arrow/type.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/exception.h>
#include <vector>
#include <tuple>
#include <string>
#include <memory>

#include <nlohmann/json.hpp>

//#include "spdlog/spdlog.h"
//#include "spdlog/sinks/stdout_color_sinks.h"

namespace fs = std::filesystem;

using json = nlohmann::json;

/***************************************************************************/
struct CostData {
    double objective1;
    double objective2;
    std::optional<double> objective3; // Optional third objective
    std::string file_index;
};

// Function to check if one solution dominates another
bool dominates(const CostData& a, const CostData& b, int num_objectives, double max_budget) {
    if(a.objective1<=max_budget && b.objective1>max_budget){
        return true;
    }
    if(a.objective1>max_budget && b.objective1<=max_budget){
        return false;
    }
    if(a.objective1>max_budget && b.objective1>max_budget){
        if(a.objective1<b.objective1){
            return true;
        }
        else{
            return false;
        }
    } 
    if (num_objectives == 2) {
        return (a.objective1 <= b.objective1 && a.objective2 <= b.objective2) &&
               (a.objective1 < b.objective1 || a.objective2 < b.objective2);
    } else if (num_objectives == 3 && a.objective3.has_value() && b.objective3.has_value()) {
        return (a.objective1 <= b.objective1 && a.objective2 <= b.objective2 && a.objective3.value() <= b.objective3.value()) &&
               (a.objective1 < b.objective1 || a.objective2 < b.objective2 || a.objective3.value() < b.objective3.value());
    }
    return false;
}

// Function to find non-dominated solutions
std::vector<std::string> find_pareto_front(const std::vector<CostData>& data, int num_objectives,double max_budget) {
    std::vector<std::string> pareto_front;

    for (const auto& current : data) {
        bool is_dominated = false;

        for (const auto& other : data) {
            std::cout << "Current Cost: " << current.objective1 << ", Other Cost: " << other.objective1 <<std::endl;
            std::cout << "Results: " <<  dominates(other, current, num_objectives, max_budget) << std::endl;
            if (dominates(other, current, num_objectives, max_budget)) {
                is_dominated = true;
                break;
            }
        }

        if (!is_dominated) {
            pareto_front.push_back(current.file_index);
        }
    }

    return pareto_front;
}

std::vector<CostData> readCostFiles(const std::vector<std::string>& objs, const std::string& directory) {
    std::vector<CostData> data;
    int num_objectives = objs.size();

    for (const auto& entry : fs::directory_iterator(directory)) {
        std::string filename = entry.path().filename().string();
        if (filename.ends_with("_costs.json")) {
            std::ifstream file(entry.path());
            json j;
            file >> j;

            CostData cost_data;
            cost_data.objective1 = j[objs[0]];
            cost_data.objective2 = j[objs[1]];
            if (num_objectives == 3) {
                cost_data.objective3 = j[objs[2]];
            } else {
                cost_data.objective3 = std::nullopt;
            }

            // Extract and store everything before the first underscore
            cost_data.file_index = filename.substr(0, filename.find('_'));

            data.push_back(cost_data);
        }
    }

    return data;
}


std::vector<std::string> findParetoFrontFiles(const std::vector<std::string>& objs, const std::string& directory, double max_budget) {

    int num_objectives = objs.size();
    std::vector<CostData> data;
    data = readCostFiles(objs, directory);
    return find_pareto_front(data, num_objectives, max_budget);
}

void writeCSV(const std::vector<CostData>& data, const std::string& output_file, const std::vector<std::string>& objs) {
    std::vector<CostData> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end(), [](const CostData& a, const CostData& b) {
        return std::stol(a.file_index) < std::stol(b.file_index);
    });
    std::ofstream csv_file(output_file);


    // Write the header
    //csv_file << "file_index," << objs[0] << "," << objs[1];
    /*
    if (objs.size() == 3) {
        csv_file << "," << objs[2];
    }
    csv_file << "\n";
    */

    // Write the data
    //
    csv_file << std::fixed << std::setprecision(6); // Set the precision to 10 decimal places
    for (const auto& entry : sorted_data) {
        csv_file << entry.objective1 << "," << entry.objective2;
        if (entry.objective3.has_value()) {
            csv_file << "," << entry.objective3.value();
        }
        csv_file << "\n";
    }

    csv_file.close();
}
/***************************************************************************/



namespace {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::string replace_ending(const std::string& str, const std::string& oldEnding, const std::string& newEnding) {
        if (str.ends_with(oldEnding)) {
            return str.substr(0, str.size() - oldEnding.size()) + newEnding;
        }
        return str;
    }
    
    void save(const std::vector<std::vector<double>>& data, const std::string& filename) {
        std::ofstream outFile(filename);
        
        // Check if the file opened successfully
        if (!outFile) {
            std::cerr << "Failed to open the file: "<<filename<<".\n";
            return;
        }
        
        for (const auto& row : data) {
            for (const auto& val : row) {
                outFile << val << ' ';
            }
            outFile << '\n';
        }
        
        outFile.close();
    }

    void save(const std::vector<Particle>& data, const std::string& filename, const std::string& filename_fx) {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Failed to open the file: "<<filename<<".\n";
            return;
        }
        
        for (const auto& row : data) {
            for (const auto& val : row.get_x()) {
                outFile << val << ' ';
            }
            outFile << '\n';
        }
        
        outFile.close();

        std::ofstream out_file_fx(filename_fx);
        if (!out_file_fx) {
            std::cerr << "Failed to open the file: "<<filename_fx<<".\n";
            return;
        }
        
        for (const auto& row : data) {
            for (const auto& val : row.get_fx()) {
                out_file_fx << val << ' ';
            }
            out_file_fx << '\n';
        }
        
        out_file_fx.close();
    }


    std::vector<std::vector<std::tuple<int, int, int, int, double>>> read_scenarios_keyed_json2(std::string filename, const std::unordered_map<std::string, double>& alpha_dict) {
        std::vector<std::vector<std::tuple<int, int, int, int, double>>> scenarios_list;
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open the file: "<<filename<<".\n";
            exit(-1);
        }
    
        json json_obj = json::parse(file);
        for (const auto &scenario_list : json_obj){
            std::vector<std::tuple<int, int, int, int, double>> parcel_list;
            for(const auto& parcel : scenario_list){
                std::vector<std::string> result_vec;
                auto key = parcel["name"].get<std::string>();
                boost::split(result_vec, key, boost::is_any_of("_"));
                auto lrseg = result_vec[0];
                auto agency = result_vec[1];
                auto load_src = result_vec[2];
                auto bmp = result_vec[3];
                double alpha = 0.0;
                try {
                    alpha = 1.0;//alpha_dict.at(fmt::format("{}_{}_{}", lrseg, agency, load_src));
                }
                catch (const std::exception& e) {
                    std::cerr << "Key not found: " << fmt::format("{}_{}_{}", lrseg, agency, load_src) << '\n';
                    for (const auto& [key, val] : alpha_dict) {
                        std::cerr << key << '\n';
                    }
                    std::cerr << e.what() << '\n';
                    exit(-1);
                }

                auto amount = parcel["amount"].get<double>() * alpha;

                parcel_list.emplace_back(std::stoi(lrseg), std::stoi(agency), std::stoi(load_src), std::stoi(bmp), amount);
            }
            scenarios_list.emplace_back(parcel_list);
        }
        return scenarios_list;
    }
    std::vector<std::vector<std::tuple<int, int, int, int, double>>> read_scenarios_keyed_json(std::string filename, const std::unordered_map<std::string, double>& alpha_dict) {
        std::vector<std::vector<std::tuple<int, int, int, int, double>>> scenarios_list;
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open the file: "<<filename<<".\n";
            exit(-1);
        }
    
        json json_obj = json::parse(file);
        for (const auto &scenario_list : json_obj){
            std::vector<std::tuple<int, int, int, int, double>> parcel_list;
            for(const auto& parcel : scenario_list){
                std::vector<std::string> result_vec;
                auto key = parcel["name"].get<std::string>();
                boost::split(result_vec, key, boost::is_any_of("_"));
                auto lrseg = result_vec[0];
                auto agency = result_vec[1];
                auto load_src = result_vec[2];
                auto bmp = result_vec[3];
                double alpha = 0.0;
                try {
                    alpha = alpha_dict.at(fmt::format("{}_{}_{}", lrseg, agency, load_src));
                }
                catch (const std::exception& e) {
                    std::cerr << "Key not found: " << fmt::format("{}_{}_{}", lrseg, agency, load_src) << '\n';
                    for (const auto& [key, val] : alpha_dict) {
                        std::cerr << key << '\n';
                    }
                    std::cerr << e.what() << '\n';
                    exit(-1);
                }

                auto amount = parcel["amount"].get<double>() * alpha;

                parcel_list.emplace_back(std::stoi(lrseg), std::stoi(agency), std::stoi(load_src), std::stoi(bmp), amount);
            }
            scenarios_list.emplace_back(parcel_list);
        }
        return scenarios_list;
    }
    
    std::vector<std::tuple<int, int, int, int, double>> read_scenario_json(std::string filename) {
        std::vector<std::tuple<int, int, int, int, double>> parcel_list;
    
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open the file: "<<filename<<".\n";
            exit(-1);
        }
    
        // Parse the JSON file directly into a nlohmann::json object
        json json_obj = json::parse(file);
    
        auto scenario = json_obj.get<std::unordered_map<std::string, double>>();
        for (const auto& [key, amount] : scenario) {
            std::vector<std::string> result_vec;
            boost::split(result_vec, key, boost::is_any_of("_"));
            parcel_list.emplace_back(std::stoi(result_vec[0]), std::stoi(result_vec[1]), std::stoi(result_vec[2]), std::stoi(result_vec[3]), amount);
        }
    
        return parcel_list;
    }

}


// Define a plain‐old‐data struct matching your schema
// struct BmpRowLand {
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

std::vector<BmpRowLand> read_parquet_file_land(const std::string& file_name) {
  // 1) Open file
  arrow::MemoryPool* pool = arrow::default_memory_pool();
  std::shared_ptr<arrow::io::ReadableFile> infile;
  PARQUET_ASSIGN_OR_THROW(
    infile,
    arrow::io::ReadableFile::Open(file_name, pool)
  );

  // 2) Open Parquet reader
  std::unique_ptr<parquet::arrow::FileReader> reader;
  PARQUET_THROW_NOT_OK(
    parquet::arrow::OpenFile(infile, pool, &reader)
  );

  // 3) Read entire file into a Table
  std::shared_ptr<arrow::Table> table;
  PARQUET_THROW_NOT_OK(reader->ReadTable(&table));
  const int64_t num_rows = table->num_rows();
  if (num_rows == 0) return {};

  // Helper to get a single (possibly concatenated) Array by column name
  auto get_array = [&](const std::string& name) -> std::shared_ptr<arrow::Array> {
    int idx = table->schema()->GetFieldIndex(name);
    if (idx < 0) throw std::runtime_error("Column not found: " + name);
    auto col = table->column(idx);
    if (col->num_chunks() == 1) {
      return col->chunk(0);
    } else {
      std::shared_ptr<arrow::Array> concatenated;
      PARQUET_ASSIGN_OR_THROW(
        concatenated,
        arrow::Concatenate(col->chunks(), pool)
      );
      return concatenated;
    }
  };

  // 4) Extract each column
  auto arr0  = get_array("BmpSubmittedId");
  auto arr1  = get_array("AgencyId");
  auto arr2  = get_array("StateUniqueIdentifier");
  auto arr3  = get_array("StateId");
  auto arr4  = get_array("BmpId");
  auto arr5  = get_array("GeographyId");
  auto arr6  = get_array("LoadSourceGroupId");
  auto arr7  = get_array("UnitId");
  auto arr8  = get_array("Amount");
  auto arr9  = get_array("IsValid");
  auto arr10 = get_array("ErrorMessage");
  auto arr11 = get_array("RowIndex");

  // 5) Cast to the right Array types
  auto col0  = std::static_pointer_cast<arrow::Int32Array>(arr0);
  auto col1  = std::static_pointer_cast<arrow::Int32Array>(arr1);
  auto col2  = std::static_pointer_cast<arrow::StringArray>(arr2);
  auto col3  = std::static_pointer_cast<arrow::Int32Array>(arr3);
  auto col4  = std::static_pointer_cast<arrow::Int32Array>(arr4);
  auto col5  = std::static_pointer_cast<arrow::Int32Array>(arr5);
  auto col6  = std::static_pointer_cast<arrow::Int32Array>(arr6);
  auto col7  = std::static_pointer_cast<arrow::Int32Array>(arr7);
  auto col8  = std::static_pointer_cast<arrow::DoubleArray>(arr8);
  auto col9  = std::static_pointer_cast<arrow::BooleanArray>(arr9);
  auto col10 = std::static_pointer_cast<arrow::StringArray>(arr10);
  auto col11 = std::static_pointer_cast<arrow::Int32Array>(arr11);

  // 6) Pull out each row into your vector
  std::vector<BmpRowLand> result;
  result.reserve(num_rows);
  for (int64_t i = 0; i < num_rows; ++i) {
    BmpRowLand row;
    row.BmpSubmittedId       = col0->Value(i);
    row.AgencyId             = col1->Value(i);
    row.StateUniqueIdentifier = col2->GetString(i);
    row.StateId              = col3->Value(i);
    row.BmpId                = col4->Value(i);
    row.GeographyId          = col5->Value(i);
    row.LoadSourceGroupId    = col6->Value(i);
    row.UnitId               = col7->Value(i);
    row.Amount               = col8->Value(i);
    row.IsValid              = col9->Value(i);
    // handle nullable ErrorMessage
    row.ErrorMessage         = col10->IsNull(i) 
                               ? std::string("") 
                               : col10->GetString(i);
    row.RowIndex             = col11->Value(i);
    result.push_back(std::move(row));
  }

  return result;
}


std::vector<std::tuple<int, int, int, int, int, double>> read_parquet_file(std::string file_name) {
    using arrow::default_memory_pool;
    using parquet::arrow::FileReader;
  
    // 1) Open the file
    std::shared_ptr<arrow::io::ReadableFile> infile;
    PARQUET_ASSIGN_OR_THROW(
      infile,
      arrow::io::ReadableFile::Open(file_name, default_memory_pool())
    );
  
    // 2) Open a ParquetFileReader
    std::unique_ptr<FileReader> reader;
    PARQUET_THROW_NOT_OK(
      parquet::arrow::OpenFile(infile, default_memory_pool(), &reader)
    );
  
    // 3) Read whole file into a Table
    std::shared_ptr<arrow::Table> table;
    PARQUET_THROW_NOT_OK(reader->ReadTable(&table));
  
    int64_t nrows = table->num_rows();
    if (nrows == 0) return {};
  
    // 4) Get column arrays (now five Int32 and one Double)
    auto col0 = std::static_pointer_cast<arrow::Int32Array>(table->column(0)->chunk(0));
    auto col1 = std::static_pointer_cast<arrow::Int32Array>(table->column(1)->chunk(0));
    auto col2 = std::static_pointer_cast<arrow::Int32Array>(table->column(2)->chunk(0));
    auto col3 = std::static_pointer_cast<arrow::Int32Array>(table->column(3)->chunk(0));
    auto col4 = std::static_pointer_cast<arrow::Int32Array>(table->column(4)->chunk(0));
    auto col5 = std::static_pointer_cast<arrow::DoubleArray>(table->column(5)->chunk(0));
  
    // Handle multiple chunks if necessary
    if (table->column(0)->num_chunks() > 1 ||
        table->column(1)->num_chunks() > 1 ||
        table->column(2)->num_chunks() > 1 ||
        table->column(3)->num_chunks() > 1 ||
        table->column(4)->num_chunks() > 1 ||
        table->column(5)->num_chunks() > 1) {
      
      std::shared_ptr<arrow::Array> raw0, raw1, raw2, raw3, raw4, raw5;
      PARQUET_ASSIGN_OR_THROW(raw0, arrow::Concatenate(table->column(0)->chunks(), default_memory_pool()));
      PARQUET_ASSIGN_OR_THROW(raw1, arrow::Concatenate(table->column(1)->chunks(), default_memory_pool()));
      PARQUET_ASSIGN_OR_THROW(raw2, arrow::Concatenate(table->column(2)->chunks(), default_memory_pool()));
      PARQUET_ASSIGN_OR_THROW(raw3, arrow::Concatenate(table->column(3)->chunks(), default_memory_pool()));
      PARQUET_ASSIGN_OR_THROW(raw4, arrow::Concatenate(table->column(4)->chunks(), default_memory_pool()));
      PARQUET_ASSIGN_OR_THROW(raw5, arrow::Concatenate(table->column(5)->chunks(), default_memory_pool()));
      
      col0 = std::static_pointer_cast<arrow::Int32Array>(raw0);
      col1 = std::static_pointer_cast<arrow::Int32Array>(raw1);
      col2 = std::static_pointer_cast<arrow::Int32Array>(raw2);
      col3 = std::static_pointer_cast<arrow::Int32Array>(raw3);
      col4 = std::static_pointer_cast<arrow::Int32Array>(raw4);
      col5 = std::static_pointer_cast<arrow::DoubleArray>(raw5);
    }
  
    // 5) Pull out row values into your vector of tuples
    std::vector<std::tuple<int, int, int, int, int, double>> result;
    result.reserve(nrows);
    for (int64_t i = 0; i < nrows; ++i) {
      result.emplace_back(
        col0->Value(i),
        col1->Value(i),
        col2->Value(i),
        col3->Value(i),
        col4->Value(i),
        col5->Value(i)
      );
    }
  
    return result;
}


// add the file and dont forget the header 
PSO::PSO(int nparts, int nobjs, int max_iter, double w, double c1, double c2, double lb, double ub, const std::string& input_filename, const std::string& scenario_filename, const std::string& out_dir, bool is_ef_enabled, bool is_lc_enabled, bool is_animal_enabled, bool is_manure_enabled, 
        const std::string& manure_nutrients_file, const std::string& base_land_bmp_file, const std::string& base_animal_bmp_file, const std::string& base_manure_bmp_file, const std::string& exec_uuid, const std::string& base_scenario_uuid) {
    out_dir_= out_dir;
    is_ef_enabled_ = is_ef_enabled;
    is_lc_enabled_ = is_lc_enabled;
    is_animal_enabled_ = is_animal_enabled;
    is_manure_enabled_ = is_manure_enabled;
    exec_uuid_ = exec_uuid;
    emo_uuid_ = base_scenario_uuid;
    ef_size_ = 0;
    lc_size_ = 0;
    animal_size_ = 0;
    manure_size_ = 0;
    init_cast(input_filename, scenario_filename, manure_nutrients_file);
    input_filename_ = input_filename;
    scenario_filename_ = scenario_filename;
    this->nparts = nparts;
    this->nobjs= nobjs;
    this->max_iter = max_iter;
    this->w = w;
    this->c1 = c1;
    this->c2 = c2;
    this->lower_bound = lb; 
    this->upper_bound = ub; 
    //logger_ = spdlog::stdout_color_mt("PSO");

    // Store the base file path 
    base_land_bmp_file_ = base_land_bmp_file;
    base_animal_bmp_file_ = base_animal_bmp_file;
    base_manure_bmp_file_ = base_manure_bmp_file;

    // unpack the parquet file and store them as vectore tuples 
    base_land_bmp_inputs_ = read_parquet_file_land(base_land_bmp_file);
    base_animal_bmp_inputs_ = read_parquet_file(base_animal_bmp_file);
    base_manure_bmp_inputs_ = read_parquet_file(base_manure_bmp_file);

     // Read in the scecario file to get the constraint
     std::ifstream in(scenario_filename_);
     json scenario;
     in >> scenario;
     max_budget_ = scenario["total_budget"].get<double>(); 
}

PSO::PSO(const PSO &p) {
    this->dim = p.dim;
    this->nparts = p.nparts;
    this->nobjs= p.nobjs;
    this->max_iter = p.max_iter;
    this->w = p.w;
    this->c1 = p.c1;
    this->c2 = p.c2;
    this->particles = p.particles;
    this->gbest_x = p.gbest_x;
    this->gbest_fx = p.gbest_fx;
    this->lower_bound = p.lower_bound;
    this->upper_bound = p.upper_bound;

    this->is_ef_enabled_ = p.is_ef_enabled_;
    this->is_lc_enabled_ = p.is_lc_enabled_;
    this->is_animal_enabled_ = p.is_animal_enabled_;
    this->is_manure_enabled_ = p.is_manure_enabled_;
    this->input_filename_ = p.input_filename_;
    this->out_dir_ = p.out_dir_;
    this->emo_uuid_ = p.emo_uuid_;
    this->lc_size_ = p.lc_size_;
    this->animal_size_ = p.animal_size_;
    this->manure_size_ = p.manure_size_;
    this->exec_uuid_log_ = p.exec_uuid_log_;
    this->scenario_ = p.scenario_;
    this->execute = p.execute;
    this->gbest_ = p.gbest_;
    //this->logger_ = p.logger_;
}

PSO& PSO::operator=(const PSO &p) {

  // Protect against self-assignment
    if (this == &p) {
        return *this;
    }

    this->dim = p.dim;
    this->nparts = p.nparts;
    this->nobjs= p.nobjs;
    this->max_iter = p.max_iter;
    this->w = p.w;
    this->c1 = p.c1;
    this->c2 = p.c2;
    this->particles = p.particles;
    this->gbest_x = p.gbest_x;
    this->gbest_fx = p.gbest_fx;
    this->lower_bound = p.lower_bound;
    this->upper_bound = p.upper_bound;


    return *this;
}

PSO::~PSO() {
    //delete_tmp_eiles();
}
void PSO::delete_tmp_files(){
    int counter = 0;
    std::string directory = fmt::format("/opt/opt4cast/output/nsga3/{}", exec_uuid_);
    for (const auto& exec_uuid_vec : exec_uuid_log_) {
        for (const auto& exec_uuid: exec_uuid_vec) {
            auto list_files = misc_utilities::find_files(directory, exec_uuid);
            for (const auto &file : list_files) {
                auto full_path = fmt::format("{}/{}", directory, file);
                try {
                    if (fs::exists(full_path)) {
                        fs::remove(full_path);
                        //std::cout << "\t\tDeleted: " << full_path << std::endl;
                    } else {
                        //logger_->error("File not found: {}", full_path);
                        std::cout << "\t\tFile not found: " << full_path << std::endl;
                    }
                } catch (const std::exception &e) {
                    std::cerr << "\tError deleting full_path " << full_path << ": " << e.what() << std::endl;
                }
            }
        }
    }
}


void PSO::init_cast(const std::string& input_filename, const std::string& scenario_filename, const std::string& manure_nutrients_file) {
    fmt::print("exec_uuid: {}\n", exec_uuid_);
    std::string exec_path = fmt::format("/opt/opt4cast/output/nsga3/{}/", exec_uuid_);
    std::unordered_map<std::string, std::tuple<int, double, double , double, double>> generation_fx;//key: UID, tuple: [idx, Nitrogen, Phosphorus, Sediments]
    std::unordered_map<std::string, int> generation_uuid_idx;
    misc_utilities::mkdir(exec_path);

    scenario_.init(input_filename, scenario_filename, is_ef_enabled_, is_lc_enabled_, is_animal_enabled_, is_manure_enabled_, manure_nutrients_file);
    ef_size_ = scenario_.get_ef_size();
    fmt::print("ef_size: {}\n", ef_size_);
    lc_size_ = scenario_.get_lc_size();
    fmt::print("lc_size: {}\n", lc_size_);
    animal_size_ = scenario_.get_animal_size();
    fmt::print("animal_size: {}\n", animal_size_);
    manure_size_ = scenario_.get_manure_size();
    fmt::print("manure_size: {}\n", manure_size_);
    dim = lc_size_ + animal_size_ + manure_size_ + ef_size_;
    fmt::print("dim: {}\n", dim);
    nobjs = 2;
}

void PSO::init() {
    particles.reserve(nparts);
    for (int i = 0; i < nparts; i++) {
        particles.emplace_back(dim, nobjs, w, c1, c2, lower_bound, upper_bound);

        //std::vector<double > x(lc_size + animal_size);
        auto x = particles[i].get_x();

        scenario_.initialize_vector(x);
        //particles[i].init();
        particles[i].init(x);
    }
    evaluate();
    for (int i = 0; i < nparts; i++) {
        particles[i].init_pbest();
    }
    update_gbest();
}

void PSO::optimize() {
    init();

    for (int i = 0; i < max_iter; i++) {
        fmt::print(" =================================================================\n                      iteration: {}\n=================================================================\n", i);
        for (int j = 0; j < nparts; j++) {
            std::uniform_int_distribution<> dis(0, gbest_.size() - 1);
            int index = dis(gen);
            const auto& curr_gbest = gbest_[index].get_x();
            particles[j].update(curr_gbest);
        }
        evaluate();
        update_pbest();
        update_gbest();
    }

    //exec_ipopt();
    fmt::print("======================Finaliza Optimize===========================================\n");

    fmt::print("======================Gx values before Ipopt===========================================\n");
    //Print all of the gx values 
    for(int i = 0; i < nparts; i++){
        fmt::print("gx_{} = {}\n",i,particles[i].get_gx());
    }

    fmt::print("======================Ipopt===========================================\n");

    exec_ipopt_all_sols();
    //evaluate_ipopt_sols();
}


void PSO::update_pbest() {
    for (int j = 0; j < nparts; j++) {
        particles[j].update_pbest();
    }
}

std::vector<std::string> PSO::generate_n_uuids(int n) {
    std::vector<std::string> uuids;
    for (int i = 0; i < n; i++) {
        uuids.push_back(xg::newGuid().str());
    }
    //json uuids_json;
    //uuids_json["uuids"] = uuids;
    return uuids;
}

void PSO::copy_parquet_files_for_ipopt(const std::string& path, const std::string& parent_uuid, const std::vector<std::string>& uuids) {
    for (const auto& uuid : uuids) {
        /*
        auto land_src = fmt::format("{}/{}_impbmpsubmittedland.parquet", path, parent_uuid);
        if(fs::exists(land_src)){
            auto land_dst = fmt::format("{}/{}_impbmpsubmittedland.parquet", path, uuid);
            misc_utilities::copy_file(land_src, land_dst);
        }
        */

        auto animal_src = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", path, parent_uuid);
        if(fs::exists(animal_src)){
            auto animal_dst = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", path, uuid);
            misc_utilities::copy_file(animal_src, animal_dst);
        }
        auto manure_src = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", path, parent_uuid);
        if (fs::exists(manure_src)) {
            auto manure_dst = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", path, uuid);
            misc_utilities::copy_file(manure_src, manure_dst);
        }
    }
} 


std::vector<Particle> PSO::get_min_mid_max_ipopt_position() {
    std::vector<Particle> values;
    std::vector<Particle> rejects;
    
    // Find all of valid options
    for (auto& particle : gbest_) {
        particle.set_gx(particle.get_fx()[0] - max_budget_); // total cost - upper limit 
        std::cout << "Particles gx: " << particle.get_gx() << std::endl;
        if (particle.get_gx() <= 0) {
            values.emplace_back(particle);
        } else {
            rejects.emplace_back(particle);
        }
    }

    // if values does not have 3 values in it add the values 
    if (values.size() < 3) {
        std::sort(rejects.begin(), rejects.end(),
            [](const auto& a, const auto& b) {
                return a.get_fx()[0] < b.get_fx()[0];
            });
        int i = 0;
        while (values.size() < 3 && i < rejects.size()) {
            values.emplace_back(rejects[i]);
            i++;
        }
    }

    for(auto value:values){
        std::cout << "Values Cost: " << value.get_fx()[0] << std::endl;
    }


    std::sort(values.begin(), values.end(),
        [](const auto& a, const auto& b) {
            return a.get_fx()[0] < b.get_fx()[0];
        });

    // Get the calues to be used in ipopt 
    Particle min_val = values[0];
    Particle max_val = values[values.size() - 1];
    Particle mid_val = values[(values.size()) / 2];
    std::cout << "Min_val gx: " << min_val.get_gx() << " Mid value gx: " << mid_val.get_gx() << "Max Value gx: " << max_val.get_gx() << std::endl;
    std::cout << "Min_val Cost: " << min_val.get_fx()[0] << " Mid value Cost: " << mid_val.get_fx()[0] << "Max Value Cost: " << max_val.get_fx()[0] << std::endl;
    return {min_val, mid_val, max_val};
}

void PSO::exec_ipopt_all_sols(){
    Execute execute;
    // int min_idx = 0;
    // int max_idx = 0; 
    // int mid_idx;
    // std::vector<double> values;

    // for (const auto& particle : gbest_) {
    //     values.emplace_back(particle.get_fx()[0]);
    // }

    // // Step 1: Initialize a vector with indices 0 to values.size() - 1
    // std::vector<size_t> indices(values.size());
    // std::iota(indices.begin(), indices.end(), 0);

    // // Step 2: Sort the indices based on comparing values from the values vector
    // std::sort(indices.begin(), indices.end(),
    //     [&values](size_t i1, size_t i2) { return values[i1] < values[i2]; });
    // //for (const auto& particle : gbest_) {
    // // min_idx is fine
    // min_idx = indices[0];
    // // max_idx should be the (first infeasible idx - 1), but if there are not enough feasible solution then consider infeasible as well
    // max_idx = indices[indices.size() - 1];
    // mid_idx = (min_idx + max_idx)/2;



    std::vector<Particle> particle_vec = get_min_mid_max_ipopt_position();
    std::string path = fmt::format("/opt/opt4cast/output/nsga3/{}", exec_uuid_);
    std::string ipopt_path = fmt::format("{}/ipopt", path);
    int counter = 0;
    //json scenario_json = misc_utilities::read_json_file(fmt::format("{}/scenario.json", path));
   for (const auto& particle : particle_vec) { 
        auto lc_cost = particle.get_lc_cost();
        auto animal_cost = particle.get_animal_cost();
        auto manure_cost = particle.get_manure_cost();
        auto parent_uuid = particle.get_uuid();
        auto parent_uuid_path = fmt::format("{}/{}", path, parent_uuid);
        misc_utilities::mkdir(parent_uuid_path);


        // Make the cost.json file and move to parent uuid path 
        json costs_json_file;
        costs_json_file["lc_cost"] = lc_cost;
        costs_json_file["animal_cost"] = animal_cost;
        costs_json_file["manure_cost"] = manure_cost;
        misc_utilities::write_json_file(fmt::format("{}_costs.json", parent_uuid_path), costs_json_file);

        //execute.set_files(exec_uuid_, in_file);
        //execute.execute(exec_uuid_, 0.50, 6, 20);

        fmt::print("Particle Selected Cost: {}\n",particle.get_fx()[0]);
        //execute.update_output(exec_uuid_, gbest_[idx].get_fx()[0]);
        fmt::print("======================== best_lc_cost_: {}\n", lc_cost);
        fmt::print("======================== best_animal_cost_: {}\n", animal_cost);
        fmt::print("======================== best_manure_cost_: {}\n", manure_cost);
       
       // Not used was probs used for logging
        // std::string postfix;
        // if (idx == min_idx) postfix = "min";
        // else if (idx == max_idx) postfix = "max";
        // else postfix = "median";

        std::string report_loads_path = fmt::format("{}_reportloads.csv", parent_uuid_path);
        fmt::print("===================================================================================== Scenario_id: {}\n", scenario_.get_scenario_id());

        execute.get_json_scenario(scenario_.get_scenario_id(), report_loads_path, parent_uuid_path);

        auto base_scenario_filename = fmt::format("{}_reportloads_processed.json", parent_uuid_path);
        fmt::print("base_scenario_filename: {}\n", base_scenario_filename);
        //std::cout << "base_scenario_filename: " <<  base_scenario_filename << std::endl;


        //misc_utilities::ls_path(path);
        //json base_scenario_json = misc_utilities::read_json_file(base_scenario_filename);
        
        json uuids_json;
        //auto uuids = generate_n_uuids(ipopt_popsize);
        //uuids_json["uuids"] = uuids;
        //copy_parquet_files_for_ipopt(path, parent_uuid, uuids);
        int pollutant_idx = 0;
        double ipopt_reduction = 0.30;  
        int nsteps = 10;

        //OPT4CAST_RUN_EPS_CNSTR_PATH = os.environ.get('OPT4CAST_RUN_EPS_CNSTR_PATH', '/home/gtoscano/projects/MSUCast/build/eps_cnstr/eps_cnstr')
        auto reportloads_json_path = base_scenario_filename;
        auto scenario_json_path = scenario_filename_;
        fmt::print("parent_uuid_path: {}", parent_uuid_path);
        //std::cout << "parent_uuid_path:" << parent_uuid_path << std:endl;
        auto base_scenario_uuid = emo_uuid_;
        auto base_scenario_str = scenario_.get_scenario_string();

        execute.execute_new(
                base_scenario_filename,
                scenario_filename_,
                parent_uuid_path,
                pollutant_idx, //0
                ipopt_reduction, //0.30
                nsteps,//10
                1,
                parent_uuid_path,
                path,
                base_scenario_uuid,
                base_scenario_str,
                exec_uuid_
            );

        fmt::print("before move_files\n");
        std::cout << "Parent uuid path: " << parent_uuid_path << '\n' << "ipop_path " << ipopt_path << '\n' <<std::endl;
        misc_utilities::move_files(parent_uuid_path, ipopt_path, nsteps, counter*nsteps);


        /*
        for (int i(0); i< nsteps; ++i) {
            auto src_parent_land = fmt::format("{}_impbmpsubmittedland.parquet", parent_uuid_path);
            auto src_land_file = fmt::format("{}/{}_{}",path, i,"impbmpsubmittedland.parquet");
            auto dst_land_file = fmt::format("{}/{}_impbmpsubmittedland.parquet", parent_uuid_path, uuids[i]);

            auto src_parent_animal = fmt::format("{}_impbmpsubmittedanimal.parquet", parent_uuid_path);
            auto dst_animal_file = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", parent_uuid_path, uuids[i]);
        }

        */
        //args = [OPT4CAST_RUN_EPS_CNSTR_PATH, reportloads_json_path, scenario_json_path,  str(sel_pollutant), str(target_pct), str(niterations)]

        //evaluate ipopt solutions


        //evaluate_ipopt_sols(sub_dir, ipopt_uuid, animal_cost, manure_cost);
        //EpsConstraint eps_constr(base_scenario_json, scenario_json, uuids_json, path_out, pollutant_idx, evaluate_cast);
        //
        //update_non_dominated_solutions(gbest_, particles[j]);
        ++counter;
    }


    std::vector<std::string> objectives = {"cost", "EoS-N"};  // Example: change as needed
    std::string directory = ipopt_path; 
    std::string pf_path = fmt::format("{}/front", path);
    std::string csv_path = fmt::format("{}/front/pareto_front.txt", path);

    fmt::print("before find pareto  \n");
    std::cout << "Max Budget: " << max_budget_ << std::endl;
    std::vector<std::string> pf_files = findParetoFrontFiles(objectives, directory, max_budget_);
    misc_utilities::move_pf(ipopt_path, pf_path, pf_files);

    std::vector<CostData> pf_data = readCostFiles(objectives, pf_path);

    writeCSV(pf_data, csv_path, objectives);

}


void PSO::evaluate_ipopt_sols(const std::string& sub_dir, const std::string& ipopt_uuid, double animal_cost, double manure_cost) {
    
    std::vector<std::string> exec_uuid_vec;
    std::unordered_map<std::string, double> total_cost_map;
    //get_parent_solution

    std::string emo_path = fmt::format("/opt/opt4cast/output/nsga3/{}", exec_uuid_);
    auto ipopt_in_filename = fmt::format("{}/{}_impbmpsubmittedland.json", emo_path, ipopt_uuid);
    fmt::print("ipopt_in_filename: {}\n", ipopt_in_filename);
    std::vector<std::tuple<int, int, int, int, double>> parent_list;
    if(is_lc_enabled_){
        parent_list = read_scenario_json(ipopt_in_filename);
    }
    //get_ipopt_solutions
    std::string filename_ipopt_out = fmt::format("{}/config/ipopt.json", emo_path);
    std::string filename_ipopt_out2 = fmt::format("{}/config/ipopt2.json", emo_path);
    fmt::print("filename_ipopt_out: {}\n", filename_ipopt_out);
    auto alpha_dict = scenario_.get_alpha();
    auto ipopt_lists = read_scenarios_keyed_json2(filename_ipopt_out2, alpha_dict);

    for (const auto& parcel_list : ipopt_lists) {

        std::vector<std::tuple<int, int, int, int, double>> combined;
        if(is_lc_enabled_){
            combined = parent_list;
        }
        combined.insert(combined.end(), parcel_list.begin(), parcel_list.end());

        std::string exec_uuid =xg::newGuid().str();
        exec_uuid_vec.emplace_back(exec_uuid);
        auto land_filename = fmt::format("{}/{}_impbmpsubmittedland.parquet", emo_path, exec_uuid);
        scenario_.write_land(combined, land_filename, base_land_bmp_inputs_);
        scenario_.write_land_json(combined, replace_ending(land_filename, ".parquet", ".json"));
        // Just doe write land for now 

        if(is_animal_enabled_){
            auto animal_dst = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", emo_path, exec_uuid);
            misc_utilities::copy_file(fmt::format("{}/{}_impbmpsubmittedanimal.parquet", emo_path, ipopt_uuid), animal_dst );
        }

        if(is_manure_enabled_){
            auto manure_dst = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", emo_path, exec_uuid);
            misc_utilities::copy_file(fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", emo_path, ipopt_uuid), manure_dst );
        }

        total_cost_map[exec_uuid] = scenario_.compute_cost(combined) + animal_cost + manure_cost;
    }

    auto results = scenario_.send_files(exec_uuid_, exec_uuid_vec);

    //ipopt_results
    auto dir_path = fmt::format("{}/config/{}", emo_path, sub_dir);
    misc_utilities::mkdir(dir_path);
    misc_utilities::copy_file(fmt::format("{}/config/ipopt.json", emo_path), fmt::format("{}/ipopt.json", dir_path));
    misc_utilities::copy_file(fmt::format("{}/config/ipopt2.json", emo_path), fmt::format("{}/ipopt2.json", dir_path));
    
    std::vector<std::vector<double>> result_fx;
    int counter = 0;
    for (const auto& result : results) {
        std::vector<std::string> result_vec;
        misc_utilities::split_str(result, '_', result_vec);
        auto exec_uuid = result_vec[0];
        result_fx.push_back({total_cost_map[exec_uuid], std::stod(result_vec[1])});

        std::regex pattern (exec_uuid);
        auto str_replacement = std::to_string(counter);
        auto found_files =  misc_utilities::find_files(emo_path, exec_uuid);
        for (const auto& filename : found_files) {
            fmt::print("filename: {}\n", filename);
            auto filename_dst = std::regex_replace(filename, pattern, str_replacement);
            misc_utilities::copy_file(fmt::format("{}/{}", emo_path, filename), fmt::format("{}/{}", dir_path, filename_dst));
        }
        counter++;
    } 

    for(const auto& result :result_fx) {
        fmt::print("ipopt solution : [{}, {}]\n", result[0], result[1]);
    }
    exec_uuid_log_.push_back(exec_uuid_vec);
    save(result_fx, fmt::format("{}/config/{}/pareto_front_ipopt.txt", emo_path, sub_dir));
    fmt::print("end\n");
}


//std::vector<Particle> gbest_;
void PSO::exec_ipopt(){
    Execute execute;
    Particle particle_selected;
    bool flag = true;
    for (const auto& particle : gbest_) {
        //select particle with lowest  particle.get_fx()[1]
        if (flag || particle_selected.get_fx()[0] > particle.get_fx()[0]) {
            particle_selected = particle;
            flag = false;
        }
    }
    auto ipopt_uuid = particle_selected.get_uuid();
    fmt::print("ipopt_uuid: {}\n", ipopt_uuid);
    auto in_file = fmt::format("/opt/opt4cast/output/nsga3/{}/{}_reportloads.csv", exec_uuid_, ipopt_uuid);

    auto lc_cost = particle_selected.get_lc_cost();
    auto animal_cost = particle_selected.get_animal_cost();
    auto manure_cost = particle_selected.get_manure_cost();
    execute.set_files(exec_uuid_, in_file);
    execute.execute(exec_uuid_, 0.50, 6, 20);
    std::string exec_str = "/home/gtoscano/projects/MSUCast/build/eps_cnstr/eps_cnstr";

    //OPT4CAST_RUN_EPS_CNSTR_PATH = os.environ.get('OPT4CAST_RUN_EPS_CNSTR_PATH', '/home/gtoscano/projects/MSUCast/build/eps_cnstr/eps_cnstr')


    std::string in_path = "/opt/opt4cast/output/nsga3/592e98d5-2d52-4d25-99cb-76f88a6d4e09/config/reportloads_processed.json"; 
    std::string out_path = "/opt/opt4cast/output/nsga3/592e98d5-2d52-4d25-99cb-76f88a6d4e09/config/scenario.json";
    std::string uuids = "/opt/opt4cast/output/nsga3/592e98d5-2d52-4d25-99cb-76f88a6d4e09/config/uuids.json";
    "/opt/opt4cast/output/nsga3/592e98d5-2d52-4d25-99cb-76f88a6d4e09/front";
    int pollutant_idx = 0;
    double ipopt_reduction = 0.7;
    int ipopt_popsize = 20;
    execute.execute_local(
        in_path,
        out_path,
        pollutant_idx, //0
        ipopt_reduction, //0.30
        ipopt_popsize //10
        ); 
    fmt::print("Particle Selected Cost: {}\n", particle_selected.get_fx()[0]);
    execute.update_output(exec_uuid_, particle_selected.get_fx()[0]);
    fmt::print("======================== best_lc_cost_: {}\n", lc_cost);
    fmt::print("======================== best_animal_cost_: {}\n", animal_cost);
    fmt::print("======================== best_manure_cost_: {}\n", manure_cost);
    std::string sub_dir = "ipopt_results";
    evaluate_ipopt_sols(sub_dir, ipopt_uuid, animal_cost, manure_cost);

}

/*
void PSO::update_gbest() {
    for (int j = 0; j < nparts; j++) {
        const auto& new_solution_x = particles[j].get_x();
        const auto& new_solution_fx = particles[j].get_fx();
        update_non_dominated_solutions( gbest_x, new_solution_x, gbest_fx, new_solution_fx);
    } 
}
*/

void PSO::update_gbest() {
    for (int j = 0; j < nparts; j++) {
        update_non_dominated_solutions(gbest_, particles[j]);
    } 
}

void PSO::print() {
    std::cout << "gbest_fx: ";
    for(auto& row : gbest_fx) {
        for(auto& val : row) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    std::cout << "gbest_x: ";
    for(auto& row : gbest_x) {
        for(auto& val : row) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout<<"--------------------------\n";
}

void PSO::evaluate() {


    //std::cout << " This is a test to see if one continaer can be build with out doing the whole thing" << std::endl; 

    std::vector<std::string> exec_uuid_vec;
    std::vector<double> total_cost_vec(nparts, 0.0);
    std::unordered_map<std::string, int> generation_uuid_idx;
    std::string emo_path = fmt::format("/opt/opt4cast/output/nsga3/{}/", emo_uuid_);
    std::string exec_path = fmt::format("/opt/opt4cast/output/nsga3/{}/", exec_uuid_);

    for (int i = 0; i < nparts; i++) {
        std::vector<std::tuple<int, int, int, int, double>> lc_x;
        std::vector<std::tuple<int, int, int, int, int, double>> animal_x;
        std::vector<std::tuple<int, int, int, int, int, double>> manure_x;
        std::unordered_map<std::string, double> amount_minus;
        std::unordered_map<std::string, double> amount_plus;
        double total_cost = 0.0;

        const auto& x = particles[i].get_x();
        // std::string exec_uuid = std::string("PSO-exec-uuid-") + xg::newGuid().str();
        std::string exec_uuid = xg::newGuid().str();
        particles[i].set_uuid(exec_uuid);
        std::cout << "=========================================PSO emo_uuid_ and exec_uuid , " << emo_uuid_ << " " << exec_uuid << std::endl; 
        bool flag = true;
        if(is_ef_enabled_){
            //total_cost += scenario_.normalize_ef(x, ef_x);
            //particles[i].set_ef_x(lc_x);
        }
        
        if(is_lc_enabled_){
            double lc_cost  = scenario_.normalize_lc(x, lc_x, amount_minus, amount_plus);
            particles[i].set_amount_minus(amount_minus);
            particles[i].set_amount_plus(amount_plus);
            particles[i].set_lc_cost(lc_cost);
            //fmt::print("lc_cost: {}\n", lc_cost);
            total_cost += lc_cost;
            particles[i].set_lc_x(lc_x);
            //fmt::print("exec_uuid: {}\n", exec_uuid);  
            auto land_filename = fmt::format("{}/{}_impbmpsubmittedland.parquet", exec_path, exec_uuid);
            std::cout << "Writing the land file" << std::endl;
            scenario_.write_land(lc_x, land_filename, base_land_bmp_inputs_);
            if (!std::filesystem::exists(land_filename)) {
                total_cost = 9999999999999.99;
                particles[i].set_lc_cost(lc_cost);
                particles[i].set_fx(total_cost, total_cost);
                particles[i].set_gx(total_cost); 
                flag = false;
                //continue;
            }

            scenario_.write_land_json(lc_x, replace_ending(land_filename, ".parquet", ".json"));
        }else {
            auto land_filename = fmt::format("{}/{}_impbmpsubmittedland.parquet", exec_path, exec_uuid);
            std::filesystem::copy(base_land_bmp_file_,land_filename, std::filesystem::copy_options::overwrite_existing);
            std::cout << "Land Disabled file path:" << land_filename << std::endl;
        }
        if(is_animal_enabled_){
            auto animal_cost = scenario_.normalize_animal(x, animal_x); 
            //fmt::print("animal_cost: {}\n", animal_cost);
            particles[i].set_animal_cost(animal_cost);
            total_cost += animal_cost;

            particles[i].set_animal_x(animal_x);
            auto animal_filename = fmt::format("{}/{}_impbmpsubmittedanimal.parquet", exec_path, exec_uuid);
            scenario_.write_animal(animal_x, animal_filename);
            if (!std::filesystem::exists(animal_filename)) {
                total_cost = 9999999999999.99;
                particles[i].set_animal_cost(animal_cost);
                particles[i].set_fx(total_cost, total_cost);
                particles[i].set_gx(total_cost); 
                flag = false;
                //continue;
            }
            scenario_.write_animal_json(animal_x, replace_ending(animal_filename, ".parquet", ".json"));
        }else{ 
            std::filesystem::path exec_path_obj(exec_path);
            std::filesystem::path exec_uuid_str(exec_uuid);

            // Handle animal files
            std::filesystem::path animal_filename = exec_path_obj / (exec_uuid_str.string() + "_impbmpsubmittedanimal.parquet");
            std::filesystem::copy(base_animal_bmp_file_, animal_filename, std::filesystem::copy_options::overwrite_existing);
            std::cout << "Animal Disabled file path:" << animal_filename << std::endl;
        }
        if(is_manure_enabled_){
            auto manure_cost = scenario_.normalize_manure(x, manure_x); 
            //fmt::print("manure_cost: {}\n", manure_cost);
            particles[i].set_manure_cost(manure_cost);
            total_cost += manure_cost;

            particles[i].set_manure_x(manure_x);
            auto manure_filename = fmt::format("{}/{}_impbmpsubmittedmanuretransport.parquet", exec_path, exec_uuid);
            scenario_.write_manure(manure_x, manure_filename);
            if (!std::filesystem::exists(manure_filename)) {
                total_cost = 9999999999999.99;
                particles[i].set_manure_cost(manure_cost);
                particles[i].set_fx(total_cost, total_cost);
                particles[i].set_gx(total_cost); 
                flag = false;
                //continue;
            }
            scenario_.write_manure_json(manure_x, replace_ending(manure_filename, ".parquet", ".json"));
        }else{
           
            std::filesystem::path exec_path_obj(exec_path);
            std::filesystem::path exec_uuid_str(exec_uuid);
            std::filesystem::path manure_filename = exec_path_obj / (exec_uuid_str.string() + "_impbmpsubmittedmanuretransport.parquet");

            std::filesystem::copy(base_manure_bmp_file_, manure_filename, std::filesystem::copy_options::overwrite_existing);
            std::cout << "Manure Disabled file path:" << manure_filename << std::endl;
        }

        if(flag){
            generation_uuid_idx[exec_uuid] = i;
            total_cost_vec[i] = total_cost;
            exec_uuid_vec.push_back(exec_uuid);
        }
    }

    //send files and wait for them
    auto results = scenario_.send_files(exec_uuid_, exec_uuid_vec);

    for (auto const& key : results) {
        std::vector<std::string> result_vec;
        misc_utilities::split_str(key, '_', result_vec);
        auto stored_idx = generation_uuid_idx[result_vec[0]];
        particles[stored_idx].set_gx(total_cost_vec[stored_idx] - max_budget_); // total cost - upper limit 
        particles[stored_idx].set_fx(total_cost_vec[stored_idx], std::stod(result_vec[1]));
    } 

    for (int i = 0; i < nparts; i++) {
        const auto& new_solution_fx = particles[i].get_fx();
        if (new_solution_fx[1] >= 9999999999999.0) {
            fmt::print("New solution fx[{}]: [{}, {}]\n", particles[i].get_uuid(), new_solution_fx[0], new_solution_fx[1]);

        }
        else {
            fmt::print("new_solution_fx[{}]: [{}, {}]\n", i, new_solution_fx[0], new_solution_fx[1]);
        }
    }
    exec_uuid_log_.push_back(exec_uuid_vec);
}

void PSO::save_gbest(std::string out_dir) {
    //create directory: out_dir
    //misc_utilities::mkdir(out_dir);

    auto pf_path = fmt::format("{}", out_dir);
    std::string exec_path = fmt::format("/opt/opt4cast/output/nsga3/{}/front", exec_uuid_);

    misc_utilities::copy_full_directory(exec_path, pf_path);

    /*
    int counter = 0;
    misc_utilities::mkdir(pf_path);
    for(const auto& particle : gbest_) {
        const auto& uuid = particle.get_uuid();
        std::regex pattern (uuid);
        std::string str_replacement = std::to_string(counter);

        auto found_files =  misc_utilities::find_files(emo_path, uuid);
         
        for (const auto& filename : found_files) {
            //copy_file(file, fmt::format("{}/{}", pfront_path, );
            auto filename_dst = std::regex_replace(filename, pattern, str_replacement);
            misc_utilities::copy_file(fmt::format("{}/{}", emo_path, filename), fmt::format("{}/{}", pfront_path, filename_dst));
        }

        counter++;
    }
    */
    /*
    auto x_filename = fmt::format("{}/pareto_set.txt", pfront_path);
    auto fx_filename = fmt::format("{}/pareto_front.txt", pfront_path);
    save(gbest_, x_filename, fx_filename);

    execute.get_files(emo_uuid_, fmt::format("{}/ipopt", out_dir));
    misc_utilities::copy_file(fmt::format("{}/ipopt/pfront_ef.txt", out_dir), fmt::format("{}/front/pfront_ef.txt", out_dir));
    */

}


