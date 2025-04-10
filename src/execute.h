class Execute {
public:
    void execute_local(
        const std::string& in_path,
        const std::string& out_path,
        int pollutant_idx,
        double ipopt_reduction,
        int ipopt_popsize,
        const std::string& scenario_name
    );
}; 