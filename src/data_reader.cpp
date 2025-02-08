/**
 * @file data_reader.cpp
 * @brief Implementation of the DataReader class.
 *
 * This file implements the DataReader class that is responsible for reading various CSV
 * and JSON data files. It reads scenario, cost, geographic, animal, and BMP related data
 * using the csv.hpp library, nlohmann/json, and fmt for formatting.
 *
 * Created by gtoscano on 4/1/23.
 */

 #include "data_reader.h"
 #include "misc_utilities.h"
 
 #include <string>
 #include <unordered_map>
 
 #include <fmt/core.h>
 
 #include "csv.hpp"
 
 #include <nlohmann/json.hpp>
 using json = nlohmann::json;
 
 /**
  * @brief Default constructor.
  *
  * Constructs a DataReader using the environment variable "MSU_CBPO_PATH" (or defaulting
  * to "/opt/opt4cast") and sets the CSVs path to that directory appended with "/csvs".
  */
 DataReader::DataReader() {
     std::string msu_cbpo_path = misc_utilities::get_env_var("MSU_CBPO_PATH", "/opt/opt4cast");
     csvs_path = fmt::format("{}/csvs", msu_cbpo_path);
 }
 
 /**
  * @brief Constructor with explicit CSV path.
  *
  * Constructs a DataReader with a given path.
  *
  * @param path The directory path where the CSV files are stored.
  */
 DataReader::DataReader(std::string path) {
     csvs_path = path;
 }
 
 /**
  * @brief Reads all data files.
  *
  * Calls all the individual read functions to load data for:
  * - U to U group
  * - Land River Segment to Geography mapping
  * - Load Source to BMP list
  * - Animal population data
  * - Animal group BMPs
  * - Land cost, animal cost, land BMP cost, and animal BMP cost data
  * - Land river segment, scenario, and geography county data
  */
 void DataReader::read_all() {
     read_u_to_u_group();
     read_lrseg_to_geography();
     read_load_src_to_bmp_list();
     read_animal_population();
     read_animal_grp_bmps();
     read_land_cost();
     read_animal_cost();
     read_land_bmp_cost();
     read_animal_bmp_cost();
     read_lrseg();
     read_scenario();
     read_geography_county();
 }
 
 /**
  * @brief Reads scenario data from a CSV file.
  *
  * Reads "TblScenario.csv" from the CSV path and parses each row into a JSON object.
  * The resulting JSON string is stored in the internal maps scenario_data_ and
  * scenario_data2_ keyed by ScenarioId.
  */
 void DataReader::read_scenario() {
     int counter = 0;
     auto filename = fmt::format("{}/TblScenario.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             auto ScenarioId = row["ScenarioId"].get<int>();
             json scenario_data;
             scenario_data["AtmDepDataSetId"] = row["AtmDepDataSetId"].get<int>();
             scenario_data["BackoutScenarioId"] = row["BackoutScenarioId"].get<int>();
             scenario_data["BaseConditionId"] = row["BaseConditionId"].get<int>();
             scenario_data["BaseLoadId"] = row["BaseLoadId"].get<int>();
             scenario_data["CostProfileId"] = row["CostProfileId"].get<int>();
             scenario_data["ClimateChangeDataSetId"] = row["ClimateChangeDataSetId"].get<int>();
             scenario_data["HistoricalCropNeedScenario"]= 6608;
             scenario_data["PointSourceDataSetId"] = row["PointSourceDataSetId"].get<int>();
             scenario_data["ScenarioTypeId"] = row["ScenarioTypeId"].get<int>();
             scenario_data["SoilPDataSetId"] = row["SoilPDataSetId"].get<int>();
             scenario_data["SourceDataRevisionId"]= row["SourceDataRevisionId"].get<int>();
             scenario_data_[ScenarioId] = scenario_data.dump();
 
             std::string scenario_name = "";
             int atm_dep_data_set = row["AtmDepDataSetId"].get<int>();
             int back_out_scenario = row["BackoutScenarioId"].get<int>();
             int base_condition = row["BaseConditionId"].get<int>();
             int base_load = row["BaseLoadId"].get<int>();
             int cost_profile = row["CostProfileId"].get<int>();
             int climate_change_data_set = row["ClimateChangeDataSetId"].get<int>();
             int historical_crop_need_scenario = 6608;
             int point_source_data_set = row["PointSourceDataSetId"].get<int>();
             int scenario_type = row["ScenarioTypeId"].get<int>();
             int soil_p_data_set = row["SoilPDataSetId"].get<int>();
             int source_data_revision = row["SourceDataRevisionId"].get<int>();
             std::string scenario_data2 = fmt::format("{}_{}_{}_{}_{}_{}_{}_{}_{}_{}_{}_{}",
                                       scenario_name,
                                       atm_dep_data_set, back_out_scenario, base_condition,
                                       base_load, cost_profile, climate_change_data_set,
                                       historical_crop_need_scenario, point_source_data_set,
                                       scenario_type, soil_p_data_set, source_data_revision);
 
             scenario_data2_[ScenarioId] = scenario_data2;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Land River Segument Read {}\n", counter);
 }
 
 /**
  * @brief Returns the scenario data for a given scenario ID.
  *
  * @param scenario_id The scenario ID.
  * @return A JSON string with the scenario data.
  */
 std::string DataReader::get_scenario_data(int scenario_id) {
     return scenario_data_.at(scenario_id);
 }
 
 /**
  * @brief Returns secondary scenario data for a given scenario ID.
  *
  * @param scenario_id The scenario ID.
  * @return A formatted string representing additional scenario data.
  */
 std::string DataReader::get_scenario_data2(int scenario_id) {
     return scenario_data2_.at(scenario_id);
 }
 
 /**
  * @brief Reads land river segment data from CSV.
  *
  * Reads "TblLandRiverSegment.csv" and stores each row’s LrsegId, LandSegmentGeographyId,
  * FIPS, StateId, and CountyId in the internal vector lrseg_.
  */
 void DataReader::read_lrseg() {
     int counter = 0;
     auto filename = fmt::format("{}/TblLandRiverSegment.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int lrseg_id = row["LrsegId"].get<int>();
             int geography = row["LandSegmentGeographyId"].get<int>();
             int fips = row["FIPS"].get<int>();
             int state = row["StateId"].get<int>();
             int county = row["CountyId"].get<int>();
             lrseg_.push_back({lrseg_id, fips, state, county});
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Land River Segument Read {}\n", counter);
 }
 
 /**
  * @brief Returns the land river segment data.
  *
  * @return A reference to the vector of lrseg data.
  */
 const std::vector<std::vector<int>>& DataReader::get_lrseg() const {
     return lrseg_;
 }
 
 /**
  * @brief Reads geography county data from CSV.
  *
  * Reads "TblGeographyCounty.csv" and populates the geography_county_ map with
  * CountyId as key and a tuple of GeographyId, GeographyType2Id, FIPS, CountyName,
  * and StateAbbreviation as value.
  */
 void DataReader::read_geography_county() {
     int counter = 0;
     auto filename = fmt::format("{}/TblGeographyCounty.csv", csvs_path);
 
     csv::CSVReader reader(filename);
     for(auto& row: reader) {
         try {
             int county = row["CountyId"].get<int>();
             int geography = row["GeographyId"].get<int>();
             int geography2 = row["GeographyType2Id"].get<int>();
             std::string fips = row["FIPS"].get<std::string>();
             std::string county_name = row["CountyName"].get<std::string>();
             std::string state = row["StateAbbreviation"].get<std::string>();
             geography_county_[county] = {geography, geography2, fips, county_name, state};
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Geography County Read {}\n", counter);
 }
 
 /**
  * @brief Returns the geography county mapping.
  *
  * @return A constant reference to the geography_county_ map.
  */
 const std::unordered_map<int, std::tuple<int, int, std::string, std::string, std::string> >& DataReader::get_geography_county() const {
     return geography_county_;
 }
 
 /**
  * @brief Reads BMP cost data from a CSV file.
  *
  * Reads the CSV file specified by @p filename and populates the bmp_cost_ map.
  * Keys are in the format "profile_bmp" and the corresponding cost is stored.
  * Also populates the bmp_cost_idx_ map with lists of keys per cost profile.
  *
  * @param filename The full path to the BMP cost CSV file.
  * @return The number of records read.
  */
 int DataReader::read_bmp_cost(const std::string& filename) {
     int counter = 0;
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int profile = row["CostProfileId"].get<int>();
             int bmp = row["BmpId"].get<int>();
             double cost = row["TotalCostPerUnit"].get<double>();
             auto key = fmt::format("{}_{}", profile, bmp);
             bmp_cost_[key] = cost;
             auto profile_str = std::to_string(profile);
             if (bmp_cost_idx_.contains(profile_str)) {
                 auto tmp_lst = bmp_cost_idx_.at(profile_str);
                 tmp_lst.push_back(key);
                 bmp_cost_idx_[profile_str] = tmp_lst;
             }
             else {
                 bmp_cost_idx_[profile_str] = {key};
             }
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Bmp Cost read {}\n", counter);
     return counter;
 }
 
 /**
  * @brief Reads land cost data.
  *
  * Invokes read_bmp_cost() on the "TblCostBmpLand.csv" file.
  */
 void DataReader::read_land_cost() {
     auto filename = fmt::format("{}/TblCostBmpLand.csv", csvs_path);
     read_bmp_cost(filename);
 }
 
 /**
  * @brief Reads animal cost data.
  *
  * Invokes read_bmp_cost() on the "TblCostBmpAnimal-reduced.csv" file.
  */
 void DataReader::read_animal_cost() {
     auto filename = fmt::format("{}/TblCostBmpAnimal-reduced.csv", csvs_path);
     read_bmp_cost(filename);
 }
 
 /**
  * @brief Returns the BMP cost for a given key.
  *
  * @param key A string key in the format "profile_bmp".
  * @return The cost value.
  */
 double DataReader::get_bmp_cost(std::string key) const {
     auto cost = bmp_cost_.at(key);
     return cost;
 }
 
 /**
  * @brief Returns the entire BMP cost map.
  *
  * @return A constant reference to the bmp_cost_ map.
  */
 const std::unordered_map<std::string, double>& DataReader::get_bmp_cost() const {
     return bmp_cost_;
 }
 
 /**
  * @brief Returns the list of BMP cost keys for a given cost profile.
  *
  * @param profile The cost profile (as a string).
  * @return A constant reference to a vector of keys.
  */
 const std::vector<std::string>& DataReader::get_bmp_cost_idx(const std::string& profile) const {
     return bmp_cost_idx_.at(profile);
 }
 
 /**
  * @brief Reads animal group BMP mappings.
  *
  * Reads "TblAnimalGrpBmp.csv" and populates the animal_grp_bmps_ map, mapping each animal group
  * to a list of BMPs.
  */
 void DataReader::read_animal_grp_bmps() {
     int counter = 0;
     auto filename = fmt::format("{}/TblAnimalGrpBmp.csv", csvs_path);
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int animal_grp = row["AnimalGrp"].get<int>();
             int bmp = row["Bmp"].get<int>();
 
             if (animal_grp_bmps_.contains(animal_grp)) {
                 auto tmp_lst = animal_grp_bmps_.at(animal_grp);
                 tmp_lst.push_back(bmp);
                 animal_grp_bmps_[animal_grp] = tmp_lst;
             }
             else {
                 animal_grp_bmps_[animal_grp] = {bmp};
             }
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal Grp Bmp read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal group BMP mapping.
  *
  * @return A constant reference to the animal_grp_bmps_ map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_animal_grp_bmps() const {
     return animal_grp_bmps_;
 }
 
 /**
  * @brief Reads animal population data.
  *
  * Reads "TblAnimalPopulation-filtered.csv" and populates the animal_ map (mapping a unique key
  * to animal units) as well as the animal_idx_ map that groups animal keys by base condition and county.
  */
 void DataReader::read_animal_population() {
     int counter = 0;
     auto filename = fmt::format("{}/TblAnimalPopulation-filtered.csv", csvs_path);
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int base_condition = row["BaseConditionId"].get<int>();
             int county = row["CountyId"].get<int>();
             int load_source = row["LoadSourceId"].get<int>();
             int animal_id = row["AnimalId"].get<int>();
             double animal_units = row["AnimalUnits"].get<double>();
 
             auto key = fmt::format("{}_{}_{}_{}", base_condition, county, load_source, animal_id);
             auto key_base_county = fmt::format("{}_{}", base_condition, county);
             animal_[key] = animal_units;
 
             if (animal_idx_.contains(key_base_county)) {
                 auto tmp_lst = animal_idx_.at(key_base_county);
                 tmp_lst.push_back(key);
                 animal_idx_[key_base_county] = tmp_lst;
             }
             else {
                 animal_idx_[key_base_county] = {key};
             }
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal population map.
  *
  * @return A constant reference to the animal_ map.
  */
 const std::unordered_map<std::string, double>& DataReader::get_animal() const {
     return animal_;
 }
 
 /**
  * @brief Returns the animal unit value for a given key.
  *
  * @param key The unique animal key.
  * @return The animal unit value.
  */
 double DataReader::get_animal(const std::string& key) {
     return animal_.at(key);
 }
 
 /**
  * @brief Returns the animal index list for a given base condition and county key.
  *
  * @param key The key in the format "BaseCondition_County".
  * @return A constant reference to the vector of animal keys.
  */
 const std::vector<std::string>& DataReader::get_animal_idx(std::string key) const {
     return animal_idx_.at(key);
 }
 
 /**
  * @brief Reads load source to BMP list mappings.
  *
  * Reads "TblBmpLoadSourceFromTo.csv" and populates two maps: lc_bmp_from_to (mapping a key
  * of "BmpId_FromLoadSourceId" to the ToLoadSourceId) and load_src_to_bmp_list (mapping a FromLoadSourceId
  * to a list of BMP Ids).
  */
 void DataReader::read_load_src_to_bmp_list() {
     int counter = 0;
     auto filename = fmt::format("{}/TblBmpLoadSourceFromTo.csv", csvs_path);
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int bmp_id = row["BmpId"].get<int>();
             int load_src_from = row["FromLoadSourceId"].get<int>();
             int load_src_to = row["ToLoadSourceId"].get<int>();
             auto key = fmt::format("{}_{}", bmp_id, load_src_from);
             lc_bmp_from_to[key] = load_src_to;
             if (load_src_to_bmp_list.contains(load_src_from)) {
                 auto tmp_list = load_src_to_bmp_list[load_src_from];
                 tmp_list.push_back(bmp_id);
                 load_src_to_bmp_list[load_src_from] = tmp_list;
             }
             else {
                 load_src_to_bmp_list[load_src_from] = {bmp_id};
             }
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to BMP list.
  *
  * @return A constant reference to the load_src_to_bmp_list map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_load_src_to_bmp_dict() const {
     return load_src_to_bmp_list;
 }
 
 /**
  * @brief Returns the BMP list for a specific load source.
  *
  * @param load_src The load source ID.
  * @return A constant reference to the vector of BMP IDs.
  */
 const std::vector<int>& DataReader::get_load_src_to_bmp_list(int load_src) const {
     return load_src_to_bmp_list.at(load_src);
 }
 
 /**
  * @brief Returns the mapping of land conversion BMP from-to.
  *
  * @return A constant reference to the lc_bmp_from_to map.
  */
 const std::unordered_map<std::string, int>& DataReader::get_lc_bmp_from_to() const {
     return lc_bmp_from_to;
 }
 
 /**
  * @brief Returns the load source mapping for a given key.
  *
  * @param lc_bmp_to The key in the format "BmpId_FromLoadSourceId".
  * @return The corresponding ToLoadSourceId.
  */
 int DataReader::get_lc_bmp_from_to(std::string lc_bmp_to) const {
     return lc_bmp_from_to.at(lc_bmp_to);
 }
 
 /**
  * @brief Reads the mapping from load source to load source group.
  *
  * Reads "load_src_to_load_src_grp.csv" and populates the u_u_group_dict map.
  */
 void DataReader::read_u_to_u_group() {
     int counter = 0;
     auto filename = fmt::format("{}/load_src_to_load_src_grp.csv", csvs_path);
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int load_src = row["LoadSrcId"].get<int>();
             int load_src_grp = row["LoadSrcGrpId"].get<int>();
             u_u_group_dict[load_src] = load_src_grp;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to load source group.
  *
  * @return A constant reference to the u_u_group_dict map.
  */
 const std::unordered_map<int, int>& DataReader::get_u_u_groups() const{
     return u_u_group_dict;
 }
 
 /**
  * @brief Returns the load source group for a given load source.
  *
  * @param load_source The load source ID.
  * @return The corresponding load source group if found; otherwise, -1.
  */
 int DataReader::get_u_u_group(int load_source) const {
     if (u_u_group_dict.contains(load_source)) {
         int ret_val = u_u_group_dict.at(load_source);
         return ret_val;
     }
     else
         return -1;
 }
 
 /**
  * @brief Reads the mapping from land river segment to geography and state.
  *
  * Reads "lrseg_geo.csv" and populates the s_geography_dict and s_state_dict maps.
  */
 void DataReader::read_lrseg_to_geography() {
     auto filename = fmt::format("{}/{}", csvs_path, "lrseg_geo.csv");
     csv::CSVReader reader(filename);
     int counter = 0;
     for(auto& row: reader) {
         try {
             int lrseg = row["LrsegId"].get<int>();
             int geography = row["GeographyId"].get<int>();
             int state = row["StateId"].get<int>();
             s_geography_dict[lrseg] = geography;
             s_state_dict[lrseg] = state;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Geographies read counter {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping of lrseg to geography.
  *
  * @return A constant reference to the s_geography_dict map.
  */
 const std::unordered_map<int, int>& DataReader::get_geographies() const {
     return s_geography_dict;
 }
 
 /**
  * @brief Returns the geography for a specific lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The geography if available; otherwise, -1.
  */
 int DataReader::get_geography(int lrseg) const {
     if (s_geography_dict.contains(lrseg))
         return s_geography_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Returns the mapping of lrseg to state.
  *
  * @return A constant reference to the s_state_dict map.
  */
 const std::unordered_map<int, int>& DataReader::get_states() const {
     return s_state_dict;
 }
 
 /**
  * @brief Returns the state for a specific lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The state if available; otherwise, -1.
  */
 int DataReader::get_state(int lrseg) const {
     if (s_state_dict.contains(lrseg))
         return s_state_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Reads BMP cost data into a nested dictionary.
  *
  * Reads the CSV file specified by @p filename and populates bmp_cost_dict_ such that
  * for each profile a mapping from BMP to cost is stored.
  *
  * @param filename The full path to the CSV file.
  * @return The number of records read.
  */
 int DataReader::read_bmp_cost2(const std::string& filename) {
     int counter = 0;
     csv::CSVReader reader(filename);
 
     for(auto& row: reader) {
         try {
             int profile = row["CostProfileId"].get<int>();
             int bmp = row["BmpId"].get<int>();
             double cost = row["TotalCostPerUnit"].get<double>();
             bmp_cost_dict_[profile][bmp] = cost;
             ++counter;
         } catch (std::exception& e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
     }
     return counter;
 }
 
 /**
  * @brief Reads land BMP cost data into a nested dictionary.
  *
  * Invokes read_bmp_cost2 on "TblCostBmpLand.csv".
  */
 void DataReader::read_land_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpLand.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Reads animal BMP cost data into a nested dictionary.
  *
  * Invokes read_bmp_cost2 on "TblCostBmpAnimal-reduced.csv".
  */
 void DataReader::read_animal_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpAnimal-reduced.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Returns the nested dictionary of land BMP costs.
  *
  * @return A constant reference to bmp_cost_dict_.
  */
 const std::unordered_map<int, std::unordered_map<int, double>>& DataReader::get_land_bmp_costs() const{
     return bmp_cost_dict_;
 }
 
 /**
  * @brief Returns the BMP cost for a given profile and BMP.
  *
  * @param profile The cost profile ID.
  * @param bmp The BMP ID.
  * @return The cost if found; otherwise, -1.
  */
 double DataReader::get_bmp_cost(int profile, int bmp) const {
     if (bmp_cost_dict_.contains(profile)){
         if (bmp_cost_dict_.at(profile).contains(bmp)) {
             return bmp_cost_dict_.at(profile).at(bmp);
         }
     }
     return -1;
 }
 
 /**
  * @brief Reads animal group BMP cost data.
  *
  * Reads "TblAnimalGrpBmp.csv" and populates the animal_grp_bmps_ map.
  */
 void DataReader::read_animal_grp_bmps() {
     int counter = 0;
     auto filename = fmt::format("{}/TblAnimalGrpBmp.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int animal_grp = row["AnimalGrp"].get<int>();
             int bmp = row["Bmp"].get<int>();
 
             if (animal_grp_bmps_.contains(animal_grp)) {
                 auto tmp_lst = animal_grp_bmps_.at(animal_grp);
                 tmp_lst.push_back(bmp);
                 animal_grp_bmps_[animal_grp] = tmp_lst;
             }
             else {
                 animal_grp_bmps_[animal_grp] = {bmp};
             }
 
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal Grp Bmp read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal group BMP mapping.
  *
  * @return A constant reference to the animal_grp_bmps_ map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_animal_grp_bmps() const {
     return animal_grp_bmps_;
 }
 
 /**
  * @brief Reads animal population data from CSV.
  *
  * Reads "TblAnimalPopulation-filtered.csv" and populates the animal_ map and animal_idx_ map.
  */
 void DataReader::read_animal_population() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblAnimalPopulation-filtered.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int base_condition = row["BaseConditionId"].get<int>();
             int county = row["CountyId"].get<int>();
             int load_source = row["LoadSourceId"].get<int>();
             int animal_id = row["AnimalId"].get<int>();
             double animal_units = row["AnimalUnits"].get<double>();
 
             auto key = fmt::format("{}_{}_{}_{}", base_condition, county, load_source, animal_id);
             auto key_base_county = fmt::format("{}_{}", base_condition, county);
             animal_[key] = animal_units;
 
             if (animal_idx_.contains(key_base_county)) {
                 auto tmp_lst = animal_idx_.at(key_base_county);
                 tmp_lst.push_back(key);
                 animal_idx_[key_base_county] = tmp_lst;
             }
             else {
                 animal_idx_[key_base_county] = {key};
             }
 
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal population data.
  *
  * @return A constant reference to the animal_ map.
  */
 const std::unordered_map<std::string, double>& DataReader::get_animal() const {
     return animal_;
 }
 
 /**
  * @brief Returns the animal unit value for a given key.
  *
  * @param key The unique animal key.
  * @return The animal unit value.
  */
 double DataReader::get_animal(const std::string& key) {
     return animal_.at(key);
 }
 
 /**
  * @brief Returns the animal index list for a given key.
  *
  * @param key The key in the format "BaseCondition_County".
  * @return A constant reference to the vector of animal keys.
  */
 const std::vector<std::string>& DataReader::get_animal_idx(std::string key) const {
     return animal_idx_.at(key);
 }
 
 /**
  * @brief Reads load source to BMP list mappings.
  *
  * Reads "TblBmpLoadSourceFromTo.csv" and populates the lc_bmp_from_to and load_src_to_bmp_list maps.
  */
 void DataReader::read_load_src_to_bmp_list() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblBmpLoadSourceFromTo.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int bmp_id = row["BmpId"].get<int>();
             int load_src_from = row["FromLoadSourceId"].get<int>();
             int load_src_to = row["ToLoadSourceId"].get<int>();
             auto key = fmt::format("{}_{}", bmp_id, load_src_from);
             lc_bmp_from_to[key] = load_src_to;
             if (load_src_to_bmp_list.contains(load_src_from)) {
                 auto tmp_list = load_src_to_bmp_list[load_src_from];
                 tmp_list.push_back(bmp_id);
                 load_src_to_bmp_list[load_src_from] = tmp_list;
             }
             else {
                 load_src_to_bmp_list[load_src_from] = {bmp_id};
             }
 
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to BMP list.
  *
  * @return A constant reference to the load_src_to_bmp_list map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_load_src_to_bmp_dict() const {
     return load_src_to_bmp_list;
 }
 
 /**
  * @brief Returns the BMP list for a specific load source.
  *
  * @param load_src The load source ID.
  * @return A constant reference to the vector of BMP IDs.
  */
 const std::vector<int>& DataReader::get_load_src_to_bmp_list(int load_src) const {
     return load_src_to_bmp_list.at(load_src);
 }
 
 /**
  * @brief Returns the land conversion BMP from-to mapping.
  *
  * @return A constant reference to the lc_bmp_from_to map.
  */
 const std::unordered_map<std::string, int>& DataReader::get_lc_bmp_from_to() const {
     return lc_bmp_from_to;
 }
 
 /**
  * @brief Returns the load source mapping for a given key.
  *
  * @param lc_bmp_to The key (format "BmpId_FromLoadSourceId").
  * @return The corresponding ToLoadSourceId.
  */
 int DataReader::get_lc_bmp_from_to(std::string lc_bmp_to) const {
     return lc_bmp_from_to.at(lc_bmp_to);
 }
 
 /**
  * @brief Reads the mapping from load source to load source group.
  *
  * Reads "load_src_to_load_src_grp.csv" and populates the u_u_group_dict map.
  */
 void DataReader::read_u_to_u_group() {
     int counter = 0;
 
     auto filename = fmt::format("{}/load_src_to_load_src_grp.csv", csvs_path);
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int load_src = row["LoadSrcId"].get<int>();
             int load_src_grp = row["LoadSrcGrpId"].get<int>();
             u_u_group_dict[load_src] = load_src_grp;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to load source group.
  *
  * @return A constant reference to the u_u_group_dict map.
  */
 const std::unordered_map<int, int>& DataReader::get_u_u_groups() const{
     return u_u_group_dict;
 }
 
 /**
  * @brief Returns the load source group for a given load source.
  *
  * @param load_source The load source ID.
  * @return The group if found; otherwise, -1.
  */
 int DataReader::get_u_u_group(int load_source) const {
     if (u_u_group_dict.contains(load_source)) {
         int ret_val = u_u_group_dict.at(load_source);
         return ret_val;
     }
     else
         return -1;
 }
 
 /**
  * @brief Reads the mapping from land river segment to geography and state.
  *
  * Reads "lrseg_geo.csv" and populates s_geography_dict and s_state_dict.
  */
 void DataReader::read_lrseg_to_geography() {
     auto filename = fmt::format("{}/{}", csvs_path, "lrseg_geo.csv");
     csv::CSVReader reader(filename);
     int counter = 0;
     for(auto& row: reader) {
         try {
             int lrseg = row["LrsegId"].get<int>();
             int geography = row["GeographyId"].get<int>();
             int state = row["StateId"].get<int>();
             s_geography_dict[lrseg] = geography;
             s_state_dict[lrseg] = state;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Geographies read counter {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping of lrseg to geography.
  *
  * @return A constant reference to s_geography_dict.
  */
 const std::unordered_map<int, int>& DataReader::get_geographies() const {
     return s_geography_dict;
 }
 
 /**
  * @brief Returns the geography for a given lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The geography if found; otherwise, -1.
  */
 int DataReader::get_geography(int lrseg) const {
     if (s_geography_dict.contains(lrseg))
         return s_geography_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Returns the mapping of lrseg to state.
  *
  * @return A constant reference to s_state_dict.
  */
 const std::unordered_map<int, int>& DataReader::get_states() const {
     return s_state_dict;
 }
 
 /**
  * @brief Returns the state for a given lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The state if found; otherwise, -1.
  */
 int DataReader::get_state(int lrseg) const {
     if (s_state_dict.contains(lrseg))
         return s_state_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Reads BMP cost data into a nested dictionary.
  *
  * Reads a CSV file and populates bmp_cost_dict_ with profile as the key and
  * a mapping from BMP to cost as its value.
  *
  * @param filename The CSV file path.
  * @return The number of records read.
  */
 int DataReader::read_bmp_cost2(const std::string& filename) {
     int counter = 0;
     csv::CSVReader reader(filename);
 
     for(auto& row: reader) {
         try {
 
             int profile = row["CostProfileId"].get<int>();
             int bmp = row["BmpId"].get<int>();
             double cost = row["TotalCostPerUnit"].get<double>();
             bmp_cost_dict_[profile][bmp] = cost;
             ++counter;
         } catch (std::exception& e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
     }
     return counter;
 }
 
 /**
  * @brief Reads land BMP cost data.
  *
  * Reads "TblCostBmpLand.csv" using read_bmp_cost2.
  */
 void DataReader::read_land_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpLand.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Reads animal BMP cost data.
  *
  * Reads "TblCostBmpAnimal-reduced.csv" using read_bmp_cost2.
  */
 void DataReader::read_animal_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpAnimal-reduced.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Returns the nested dictionary of land BMP costs.
  *
  * @return A constant reference to bmp_cost_dict_.
  */
 const std::unordered_map<int, std::unordered_map<int, double>>& DataReader::get_land_bmp_costs() const{
     return bmp_cost_dict_;
 }
 
 /**
  * @brief Returns the BMP cost for a given profile and BMP.
  *
  * @param profile The cost profile ID.
  * @param bmp The BMP ID.
  * @return The cost if found; otherwise, -1.
  */
 double DataReader::get_bmp_cost(int profile, int bmp) const {
     if (bmp_cost_dict_.contains(profile)){
         if (bmp_cost_dict_.at(profile).contains(bmp)) {
             return bmp_cost_dict_.at(profile).at(bmp);
         }
     }
     return -1;
 }
 
 /**
  * @brief Reads animal group BMP cost data.
  *
  * Reads "TblAnimalGrpBmp.csv" and populates the animal_grp_bmps_ map.
  */
 void DataReader::read_animal_grp_bmps() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblAnimalGrpBmp.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int animal_grp = row["AnimalGrp"].get<int>();
             int bmp = row["Bmp"].get<int>();
 
             if (animal_grp_bmps_.contains(animal_grp)) {
                 auto tmp_lst = animal_grp_bmps_.at(animal_grp);
                 tmp_lst.push_back(bmp);
                 animal_grp_bmps_[animal_grp] = tmp_lst;
             }
             else {
                 animal_grp_bmps_[animal_grp] = {bmp};
             }
 
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal Grp Bmp read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal group BMP mapping.
  *
  * @return A constant reference to the animal_grp_bmps_ map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_animal_grp_bmps() const {
     return animal_grp_bmps_;
 }
 
 /**
  * @brief Reads animal population data from CSV.
  *
  * Reads "TblAnimalPopulation-filtered.csv" and populates both the animal_ and animal_idx_ maps.
  */
 void DataReader::read_animal_population() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblAnimalPopulation-filtered.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int base_condition = row["BaseConditionId"].get<int>();
             int county = row["CountyId"].get<int>();
             int load_source = row["LoadSourceId"].get<int>();
             int animal_id = row["AnimalId"].get<int>();
             double animal_units = row["AnimalUnits"].get<double>();
 
             auto key = fmt::format("{}_{}_{}_{}", base_condition, county, load_source, animal_id);
             auto key_base_county = fmt::format("{}_{}", base_condition, county);
             animal_[key] = animal_units;
 
             if (animal_idx_.contains(key_base_county)) {
                 auto tmp_lst = animal_idx_.at(key_base_county);
                 tmp_lst.push_back(key);
                 animal_idx_[key_base_county] = tmp_lst;
             }
             else {
                 animal_idx_[key_base_county] = {key};
             }
 
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal population data.
  *
  * @return A constant reference to the animal_ map.
  */
 const std::unordered_map<std::string, double>& DataReader::get_animal() const {
     return animal_;
 }
 
 /**
  * @brief Returns the animal unit value for a given key.
  *
  * @param key The unique animal key.
  * @return The animal unit value.
  */
 double DataReader::get_animal(const std::string& key) {
     return animal_.at(key);
 }
 
 /**
  * @brief Returns the animal index list for a given key.
  *
  * @param key The key in the format "BaseCondition_County".
  * @return A constant reference to the vector of animal keys.
  */
 const std::vector<std::string>& DataReader::get_animal_idx(std::string key) const {
     return animal_idx_.at(key);
 }
 
 /**
  * @brief Reads load source to BMP list mappings.
  *
  * Reads "TblBmpLoadSourceFromTo.csv" and populates both the lc_bmp_from_to map and
  * the load_src_to_bmp_list map.
  */
 void DataReader::read_load_src_to_bmp_list() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblBmpLoadSourceFromTo.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int bmp_id = row["BmpId"].get<int>();
             int load_src_from = row["FromLoadSourceId"].get<int>();
             int load_src_to = row["ToLoadSourceId"].get<int>();
             auto key = fmt::format("{}_{}", bmp_id, load_src_from);
             lc_bmp_from_to[key] = load_src_to;
             if (load_src_to_bmp_list.contains(load_src_from)) {
                 auto tmp_list = load_src_to_bmp_list[load_src_from];
                 tmp_list.push_back(bmp_id);
                 load_src_to_bmp_list[load_src_from] = tmp_list;
             }
             else {
                 load_src_to_bmp_list[load_src_from] = {bmp_id};
             }
 
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to BMP list.
  *
  * @return A constant reference to the load_src_to_bmp_list map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_load_src_to_bmp_dict() const {
     return load_src_to_bmp_list;
 }
 
 /**
  * @brief Returns the BMP list for a given load source.
  *
  * @param load_src The load source ID.
  * @return A constant reference to the vector of BMP IDs.
  */
 const std::vector<int>& DataReader::get_load_src_to_bmp_list(int load_src) const {
     return load_src_to_bmp_list.at(load_src);
 }
 
 /**
  * @brief Returns the mapping of land conversion BMP from-to.
  *
  * @return A constant reference to the lc_bmp_from_to map.
  */
 const std::unordered_map<std::string, int>& DataReader::get_lc_bmp_from_to() const {
     return lc_bmp_from_to;
 }
 
 /**
  * @brief Returns the load source mapping for a given key.
  *
  * @param lc_bmp_to The key in the format "BmpId_FromLoadSourceId".
  * @return The corresponding ToLoadSourceId.
  */
 int DataReader::get_lc_bmp_from_to(std::string lc_bmp_to) const {
     return lc_bmp_from_to.at(lc_bmp_to);
 }
 
 /**
  * @brief Reads the mapping from load source to load source group.
  *
  * Reads "load_src_to_load_src_grp.csv" and populates the u_u_group_dict map.
  */
 void DataReader::read_u_to_u_group() {
     int counter = 0;
 
     auto filename = fmt::format("{}/load_src_to_load_src_grp.csv", csvs_path);
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int load_src = row["LoadSrcId"].get<int>();
             int load_src_grp = row["LoadSrcGrpId"].get<int>();
             u_u_group_dict[load_src] = load_src_grp;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to load source group.
  *
  * @return A constant reference to the u_u_group_dict map.
  */
 const std::unordered_map<int, int>& DataReader::get_u_u_groups() const{
     return u_u_group_dict;
 }
 
 /**
  * @brief Returns the load source group for a given load source.
  *
  * @param load_source The load source ID.
  * @return The corresponding group if found; otherwise, -1.
  */
 int DataReader::get_u_u_group(int load_source) const {
     if (u_u_group_dict.contains(load_source)) {
         int ret_val = u_u_group_dict.at(load_source);
         return ret_val;
     }
     else
         return -1;
 }
 
 /**
  * @brief Reads the mapping from land river segment to geography and state.
  *
  * Reads "lrseg_geo.csv" and populates the s_geography_dict and s_state_dict maps.
  */
 void DataReader::read_lrseg_to_geography() {
     auto filename = fmt::format("{}/{}", csvs_path, "lrseg_geo.csv");
     csv::CSVReader reader(filename);
     int counter = 0;
     for(auto& row: reader) {
         try {
             int lrseg = row["LrsegId"].get<int>();
             int geography = row["GeographyId"].get<int>();
             int state = row["StateId"].get<int>();
             s_geography_dict[lrseg] = geography;
             s_state_dict[lrseg] = state;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Geographies read counter {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping of lrseg to geography.
  *
  * @return A constant reference to s_geography_dict.
  */
 const std::unordered_map<int, int>& DataReader::get_geographies() const {
     return s_geography_dict;
 }
 
 /**
  * @brief Returns the geography for a given lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The geography if found; otherwise, -1.
  */
 int DataReader::get_geography(int lrseg) const {
     if (s_geography_dict.contains(lrseg))
         return s_geography_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Returns the mapping of lrseg to state.
  *
  * @return A constant reference to s_state_dict.
  */
 const std::unordered_map<int, int>& DataReader::get_states() const {
     return s_state_dict;
 }
 
 /**
  * @brief Returns the state for a given lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The state if found; otherwise, -1.
  */
 int DataReader::get_state(int lrseg) const {
     if (s_state_dict.contains(lrseg))
         return s_state_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Reads BMP cost data into a nested dictionary.
  *
  * Reads a CSV file and populates bmp_cost_dict_ such that for each profile, a mapping from BMP to cost is stored.
  *
  * @param filename The full path to the CSV file.
  * @return The number of records read.
  */
 int DataReader::read_bmp_cost2(const std::string& filename) {
     int counter = 0;
     csv::CSVReader reader(filename);
 
     for(auto& row: reader) {
         try {
 
             int profile = row["CostProfileId"].get<int>();
             int bmp = row["BmpId"].get<int>();
             double cost = row["TotalCostPerUnit"].get<double>();
             bmp_cost_dict_[profile][bmp]=cost;
             ++counter;
         } catch (std::exception& e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
     }
     return counter;
 }
 
 /**
  * @brief Reads land BMP cost data.
  *
  * Invokes read_bmp_cost2 on the "TblCostBmpLand.csv" file.
  */
 void DataReader::read_land_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpLand.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Reads animal BMP cost data.
  *
  * Invokes read_bmp_cost2 on the "TblCostBmpAnimal-reduced.csv" file.
  */
 void DataReader::read_animal_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpAnimal-reduced.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Returns the nested dictionary of land BMP costs.
  *
  * @return A constant reference to bmp_cost_dict_.
  */
 const std::unordered_map<int, std::unordered_map<int, double>>& DataReader::get_land_bmp_costs() const{
     return bmp_cost_dict_;
 }
 
 /**
  * @brief Returns the BMP cost for a given profile and BMP.
  *
  * @param profile The cost profile ID.
  * @param bmp The BMP ID.
  * @return The cost if found; otherwise, -1.
  */
 double DataReader::get_bmp_cost(int profile, int bmp) const {
     if (bmp_cost_dict_.contains(profile)){
         if (bmp_cost_dict_.at(profile).contains(bmp)) {
             return bmp_cost_dict_.at(profile).at(bmp);
         }
     }
     return -1;
 }
 
 /**
  * @brief Reads animal group BMP cost data.
  *
  * Reads "TblAnimalGrpBmp.csv" and populates the animal_grp_bmps_ map.
  */
 void DataReader::read_animal_grp_bmps() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblAnimalGrpBmp.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int animal_grp = row["AnimalGrp"].get<int>();
             int bmp = row["Bmp"].get<int>();
 
             if (animal_grp_bmps_.contains(animal_grp)) {
                 auto tmp_lst = animal_grp_bmps_.at(animal_grp);
                 tmp_lst.push_back(bmp);
                 animal_grp_bmps_[animal_grp] = tmp_lst;
             }
             else {
                 animal_grp_bmps_[animal_grp] = {bmp};
             }
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal Grp Bmp read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal group BMP mapping.
  *
  * @return A constant reference to the animal_grp_bmps_ map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_animal_grp_bmps() const {
     return animal_grp_bmps_;
 }
 
 /**
  * @brief Reads animal population data.
  *
  * Reads "TblAnimalPopulation-filtered.csv" and populates the animal_ and animal_idx_ maps.
  */
 void DataReader::read_animal_population() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblAnimalPopulation-filtered.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int base_condition = row["BaseConditionId"].get<int>();
             int county = row["CountyId"].get<int>();
             int load_source = row["LoadSourceId"].get<int>();
             int animal_id = row["AnimalId"].get<int>();
             double animal_units = row["AnimalUnits"].get<double>();
             auto key = fmt::format("{}_{}_{}_{}", base_condition, county, load_source, animal_id);
             auto key_base_county = fmt::format("{}_{}", base_condition, county);
             animal_[key] = animal_units;
 
             if (animal_idx_.contains(key_base_county)) {
                 auto tmp_lst = animal_idx_.at(key_base_county);
                 tmp_lst.push_back(key);
                 animal_idx_[key_base_county] = tmp_lst;
             }
             else {
                 animal_idx_[key_base_county] = {key};
             }
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Animal read {}\n", counter);
 }
 
 /**
  * @brief Returns the animal population data.
  *
  * @return A constant reference to the animal_ map.
  */
 const std::unordered_map<std::string, double>& DataReader::get_animal() const {
     return animal_;
 }
 
 /**
  * @brief Returns the animal unit value for a given key.
  *
  * @param key The unique animal key.
  * @return The animal unit value.
  */
 double DataReader::get_animal(const std::string& key) {
     return animal_.at(key);
 }
 
 /**
  * @brief Returns the animal index list for a given key.
  *
  * @param key The key in the format "BaseCondition_County".
  * @return A constant reference to the vector of animal keys.
  */
 const std::vector<std::string>& DataReader::get_animal_idx(std::string key) const {
     return animal_idx_.at(key);
 }
 
 /**
  * @brief Reads load source to BMP list mappings.
  *
  * Reads "TblBmpLoadSourceFromTo.csv" and populates the lc_bmp_from_to and load_src_to_bmp_list maps.
  */
 void DataReader::read_load_src_to_bmp_list() {
     int counter = 0;
 
     auto filename = fmt::format("{}/TblBmpLoadSourceFromTo.csv", csvs_path);
 
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int bmp_id = row["BmpId"].get<int>();
             int load_src_from = row["FromLoadSourceId"].get<int>();
             int load_src_to = row["ToLoadSourceId"].get<int>();
             auto key = fmt::format("{}_{}", bmp_id, load_src_from);
             lc_bmp_from_to[key] = load_src_to;
             if (load_src_to_bmp_list.contains(load_src_from)) {
                 auto tmp_list = load_src_to_bmp_list[load_src_from];
                 tmp_list.push_back(bmp_id);
                 load_src_to_bmp_list[load_src_from] = tmp_list;
             }
             else {
                 load_src_to_bmp_list[load_src_from] = {bmp_id};
             }
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to BMP list.
  *
  * @return A constant reference to the load_src_to_bmp_list map.
  */
 const std::unordered_map<int, std::vector<int>>& DataReader::get_load_src_to_bmp_dict() const {
     return load_src_to_bmp_list;
 }
 
 /**
  * @brief Returns the BMP list for a given load source.
  *
  * @param load_src The load source ID.
  * @return A constant reference to the vector of BMP IDs.
  */
 const std::vector<int>& DataReader::get_load_src_to_bmp_list(int load_src) const {
     return load_src_to_bmp_list.at(load_src);
 }
 
 /**
  * @brief Returns the mapping of land conversion BMP from-to.
  *
  * @return A constant reference to the lc_bmp_from_to map.
  */
 const std::unordered_map<std::string, int>& DataReader::get_lc_bmp_from_to() const {
     return lc_bmp_from_to;
 }
 
 /**
  * @brief Returns the load source mapping for a given key.
  *
  * @param lc_bmp_to The key in the format "BmpId_FromLoadSourceId".
  * @return The corresponding ToLoadSourceId.
  */
 int DataReader::get_lc_bmp_from_to(std::string lc_bmp_to) const {
     return lc_bmp_from_to.at(lc_bmp_to);
 }
 
 /**
  * @brief Reads the mapping from load source to load source group.
  *
  * Reads "load_src_to_load_src_grp.csv" and populates the u_u_group_dict map.
  */
 void DataReader::read_u_to_u_group() {
     int counter = 0;
 
     auto filename = fmt::format("{}/load_src_to_load_src_grp.csv", csvs_path);
     csv::CSVReader reader2(filename);
     for(auto& row: reader2) {
         try {
             int load_src = row["LoadSrcId"].get<int>();
             int load_src_grp= row["LoadSrcGrpId"].get<int>();
             u_u_group_dict[load_src] = load_src_grp;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Load sources to load sources read {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping from load source to load source group.
  *
  * @return A constant reference to the u_u_group_dict map.
  */
 const std::unordered_map<int, int>& DataReader::get_u_u_groups() const{
     return u_u_group_dict;
 }
 
 /**
  * @brief Returns the load source group for a given load source.
  *
  * @param load_source The load source ID.
  * @return The corresponding group if found; otherwise, -1.
  */
 int DataReader::get_u_u_group(int load_source) const {
     if (u_u_group_dict.contains(load_source)) {
         int ret_val = u_u_group_dict.at(load_source);
         return ret_val;
     }
     else
         return -1;
 }
 
 /**
  * @brief Reads the mapping from land river segment to geography and state.
  *
  * Reads "lrseg_geo.csv" and populates s_geography_dict and s_state_dict.
  */
 void DataReader::read_lrseg_to_geography() {
     auto filename = fmt::format("{}/{}", csvs_path, "lrseg_geo.csv");
     csv::CSVReader reader(filename);
     int counter = 0;
     for(auto& row: reader) {
         try {
             int lrseg = row["LrsegId"].get<int>();
             int geography = row["GeographyId"].get<int>();
             int state = row["StateId"].get<int>();
             s_geography_dict[lrseg] = geography;
             s_state_dict[lrseg] = state;
         } catch (std::exception &e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
         counter++;
     }
     fmt::print("Geographies read counter {}\n", counter);
 }
 
 /**
  * @brief Returns the mapping of lrseg to geography.
  *
  * @return A constant reference to s_geography_dict.
  */
 const std::unordered_map<int, int>& DataReader::get_geographies() const {
     return s_geography_dict;
 }
 
 /**
  * @brief Returns the geography for a given lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The geography if found; otherwise, -1.
  */
 int DataReader::get_geography(int lrseg) const {
     if (s_geography_dict.contains(lrseg))
         return s_geography_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Returns the mapping of lrseg to state.
  *
  * @return A constant reference to s_state_dict.
  */
 const std::unordered_map<int, int>& DataReader::get_states() const {
     return s_state_dict;
 }
 
 /**
  * @brief Returns the state for a given lrseg.
  *
  * @param lrseg The land river segment ID.
  * @return The state if found; otherwise, -1.
  */
 int DataReader::get_state(int lrseg) const {
     if (s_state_dict.contains(lrseg))
         return s_state_dict.at(lrseg);
     else
         return -1;
 }
 
 /**
  * @brief Reads BMP cost data into a nested dictionary.
  *
  * Reads a CSV file and populates bmp_cost_dict_ such that for each profile a mapping
  * from BMP to cost is stored.
  *
  * @param filename The full path to the CSV file.
  * @return The number of records read.
  */
 int DataReader::read_bmp_cost2(const std::string& filename) {
     int counter = 0;
     csv::CSVReader reader(filename);
 
     for(auto& row: reader) {
         try {
 
             int profile = row["CostProfileId"].get<int>();
             int bmp = row["BmpId"].get<int>();
             double cost = row["TotalCostPerUnit"].get<double>();
             bmp_cost_dict_[profile][bmp]=cost;
             ++counter;
         } catch (std::exception& e) {
             std::cerr << "Caught exception: " << e.what() << std::endl;
         } catch (...) {
             std::cerr << "Caught unknown exception." << std::endl;
         }
     }
     return counter;
 }
 
 /**
  * @brief Reads land BMP cost data.
  *
  * Invokes read_bmp_cost2 on "TblCostBmpLand.csv".
  */
 void DataReader::read_land_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpLand.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Reads animal BMP cost data.
  *
  * Invokes read_bmp_cost2 on "TblCostBmpAnimal-reduced.csv".
  */
 void DataReader::read_animal_bmp_cost() {
     auto filename = fmt::format("{}/TblCostBmpAnimal-reduced.csv", csvs_path);
     read_bmp_cost2(filename);
 }
 
 /**
  * @brief Returns the nested dictionary of land BMP costs.
  *
  * @return A constant reference to bmp_cost_dict_.
  */
 const std::unordered_map<int, std::unordered_map<int, double>>& DataReader::get_land_bmp_costs() const{
     return bmp_cost_dict_;
 }
 
 /**
  * @brief Returns the BMP cost for a given profile and BMP.
  *
  * @param profile The cost profile ID.
  * @param bmp The BMP ID.
  * @return The cost if found; otherwise, -1.
  */
 double DataReader::get_bmp_cost(int profile, int bmp) const {
     if (bmp_cost_dict_.contains(profile)){
         if (bmp_cost_dict_.at(profile).contains(bmp)) {
             return bmp_cost_dict_.at(profile).at(bmp);
         }
     }
     return -1;
 }
 