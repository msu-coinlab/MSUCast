#include <vector>
#include <algorithm>
#include <external_archive.h>

#include <range/v3/all.hpp>

#include "particle.h"

bool is_dominated(const std::vector<double>& a, const std::vector<double>& b, const double a_g, const double b_g){
    using namespace ranges;
    if ( a_g > 0 && b_g <= 0){
        return true;
    }
    if( a_g <= 0 && b_g > 0){
        return false; 
    }
    if( a_g > 0 && b_g > 0){
        if(a_g < b_g){
            return false;
        }
        else{
            return true;
        }
    }
    return all_of(view::zip(a, b), [](auto pair) { return pair.first >= pair.second; }) &&
           any_of(view::zip(a, b), [](auto pair) { return pair.first > pair.second; });
}

// This just used to make it were it doesn't crash on is_non_dominated since that never get calleed
// bool is_dominated(const std::vector<double>& a, const std::vector<double>& b){
//     using namespace ranges;
//     return all_of(view::zip(a, b), [](auto pair) { return pair.first >= pair.second; }) &&
//            any_of(view::zip(a, b), [](auto pair) { return pair.first > pair.second; });
// }

bool is_dominated(const Particle& part_a, const Particle& part_b) {
    using namespace ranges;
    const auto& a = part_a.get_fx();
    const auto& b = part_b.get_fx();
    const auto& a_g = part_a.get_gx();
    const auto& b_g = part_b.get_gx();

    if(a_g > 0 && b_g <= 0){
        return true;
    }
    if(a_g <= 0 && b_g > 0){
        return false;
    }
    if(a_g > 0 && b_g > 0){
        if(a_g < b_g){
            return false;
        }
        else{
            return true;
        }
    }
    return all_of(view::zip(a, b), [](auto pair) { return pair.first >= pair.second; }) &&
           any_of(view::zip(a, b), [](auto pair) { return pair.first > pair.second; });
}


bool is_non_dominated(const std::vector<double>& a, const std::vector<double>& b, const double a_g, const double b_g) {
    return !is_dominated(a, b, a_g, b_g) && !is_dominated(b, a, b_g, a_g);
}

bool is_non_dominated(const Particle& part_a, const Particle& part_b) { 
    const auto& a = part_a.get_fx();
    const auto& b = part_b.get_fx();
    const auto& a_g = part_a.get_gx();
    const auto& b_g = part_b.get_gx();
    return !is_dominated(a, b, a_g, b_g) && !is_dominated(b, a, b_g, a_g);
}

void update_non_dominated_solutions(
    std::vector<std::vector<double>>& archive_x, 
    const std::vector<double>& new_solution_x, 
    std::vector<std::vector<double>>& archive_fx, 
    const std::vector<double>& new_solution_fx,
    std::vector<double>& archive_gx,
    double new_solution_gx
    ) 
{
    std::vector<std::vector<double>> new_archive_x;
    std::vector<std::vector<double>> new_archive_fx;
    std::vector<double> new_archive_gx; 

    bool new_solution_dominated = false;

    for (size_t i = 0; i < archive_fx.size(); ++i) {
        if (is_dominated(archive_fx[i], new_solution_fx, archive_gx[i], new_solution_gx)) {
            continue;
        }
        if (is_dominated(new_solution_fx, archive_fx[i], new_solution_gx, archive_gx[i])) 
        {
            new_solution_dominated = true;
        }

        new_archive_x.push_back(archive_x[i]);
        new_archive_fx.push_back(archive_fx[i]);
        new_archive_gx.push_back(archive_gx[i]); 
    }

    if (!new_solution_dominated) {
        new_archive_x.push_back(new_solution_x);
        new_archive_fx.push_back(new_solution_fx);
        new_archive_gx.push_back(new_solution_gx); 
    }
    std::swap( archive_x, new_archive_x);
    std::swap( archive_fx, new_archive_fx);
    std::swap(archive_gx, new_archive_gx);
}

void update_non_dominated_solutions(
    std::vector<Particle>& archive, 
    const Particle& new_solution_x)
{
    std::vector<Particle> new_archive;

    bool new_solution_dominated = false;

    for (size_t i = 0; i < archive.size(); ++i) {
        const auto& archive_fx = archive[i].get_fx();
        const auto& archive_gx = archive[i].get_gx();
        const auto& new_solution_fx = new_solution_x.get_fx();
        const auto& new_soulution_gx = new_solution_x.get_gx();

        if (is_dominated(archive_fx, new_solution_fx, archive_gx, new_soulution_gx)) {
            continue;
        }

        if (is_dominated(new_solution_fx, archive_fx, new_soulution_gx, archive_gx) ||  new_solution_fx == archive_fx){
            new_solution_dominated = true;
        }

        new_archive.push_back(archive[i]);
    }

    if (!new_solution_dominated) {
        new_archive.push_back(new_solution_x);
    }
    std::swap( archive, new_archive);
}


