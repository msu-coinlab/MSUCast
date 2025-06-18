#ifndef EXTERNAL_ARCHIVE_H
#define EXTERNAL_ARCHIVE_H
#include <vector>
#include <string>
#include "particle.h"


bool is_dominated(const std::vector<double>& a, const std::vector<double>& b, const double g1, const double g2);

//bool is_dominated(const std::vector<double>& a, const std::vector<double>& b);

bool is_non_dominated(const std::vector<double>& a, const std::vector<double>& b, const double a_g, const double b_g);

void update_non_dominated_solutions(
    std::vector<std::vector<double>>& archive_x, 
    const std::vector<double>& new_solution_x, 
    std::vector<std::vector<double>>& archive_fx, 
    const std::vector<double>& new_solution_fx,
    std::vector<double>& archive_gx,
    const double new_solution_gx);
    
void update_non_dominated_solutions(
    std::vector<Particle>& archive, 
    const Particle& new_solution_x);

#endif
