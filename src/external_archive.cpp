/**
 * @file external_archive.cpp
 * @brief Implements functions for maintaining an external archive of non-dominated solutions in multi-objective optimization.
 */

 #include <vector>
 #include <algorithm>
 #include <external_archive.h>
 #include <range/v3/all.hpp>
 #include "particle.h"
 
 /**
  * @brief Checks if one solution dominates another in a multi-objective optimization problem.
  * @param a The first solution vector.
  * @param b The second solution vector.
  * @return True if `a` dominates `b`, otherwise false.
  */
 bool is_dominated(const std::vector<double>& a, const std::vector<double>& b) {
     using namespace ranges;
     return all_of(view::zip(a, b), [](auto pair) { return pair.first >= pair.second; }) &&
            any_of(view::zip(a, b), [](auto pair) { return pair.first > pair.second; });
 }
 
 /**
  * @brief Checks if one Particle object dominates another in a multi-objective optimization problem.
  * @param part_a The first Particle object.
  * @param part_b The second Particle object.
  * @return True if `part_a` dominates `part_b`, otherwise false.
  */
 bool is_dominated(const Particle& part_a, const Particle& part_b) {
     using namespace ranges;
     const auto& a = part_a.get_fx();
     const auto& b = part_b.get_fx();
 
     return all_of(view::zip(a, b), [](auto pair) { return pair.first >= pair.second; }) &&
            any_of(view::zip(a, b), [](auto pair) { return pair.first > pair.second; });
 }
 
 /**
  * @brief Checks if two solutions are non-dominated with respect to each other.
  * @param a The first solution vector.
  * @param b The second solution vector.
  * @return True if neither `a` dominates `b` nor `b` dominates `a`, otherwise false.
  */
 bool is_non_dominated(const std::vector<double>& a, const std::vector<double>& b) {
     return !is_dominated(a, b) && !is_dominated(b, a);
 }
 
 /**
  * @brief Checks if two Particle objects are non-dominated with respect to each other.
  * @param part_a The first Particle object.
  * @param part_b The second Particle object.
  * @return True if neither `part_a` dominates `part_b` nor `part_b` dominates `part_a`, otherwise false.
  */
 bool is_non_dominated(const Particle& part_a, const Particle& part_b) { 
     const auto& a = part_a.get_fx();
     const auto& b = part_b.get_fx();
     return !is_dominated(a, b) && !is_dominated(b, a);
 }
 
 /**
  * @brief Updates the archive of non-dominated solutions by adding a new candidate solution.
  * @param archive_x Archive of solution vectors.
  * @param new_solution_x New solution vector to be considered.
  * @param archive_fx Archive of objective function values corresponding to `archive_x`.
  * @param new_solution_fx Objective function values of `new_solution_x`.
  */
 void update_non_dominated_solutions(
     std::vector<std::vector<double>>& archive_x, 
     const std::vector<double>& new_solution_x, 
     std::vector<std::vector<double>>& archive_fx, 
     const std::vector<double>& new_solution_fx) 
 {
     std::vector<std::vector<double>> new_archive_x;
     std::vector<std::vector<double>> new_archive_fx;
 
     bool new_solution_dominated = false;
 
     for (size_t i = 0; i < archive_fx.size(); ++i) {
         if (is_dominated(archive_fx[i], new_solution_fx)) {
             continue;
         }
         if (is_dominated(new_solution_fx, archive_fx[i]) || 
             new_solution_fx == archive_fx[i]) 
         {
             new_solution_dominated = true;
         }
 
         new_archive_x.push_back(archive_x[i]);
         new_archive_fx.push_back(archive_fx[i]);
     }
 
     if (!new_solution_dominated) {
         new_archive_x.push_back(new_solution_x);
         new_archive_fx.push_back(new_solution_fx);
     }
     std::swap(archive_x, new_archive_x);
     std::swap(archive_fx, new_archive_fx);
 }
 
 /**
  * @brief Updates the archive of non-dominated Particle solutions by adding a new candidate solution.
  * @param archive Archive of Particle objects.
  * @param new_solution_x New Particle object to be considered.
  */
 void update_non_dominated_solutions(
     std::vector<Particle>& archive, 
     const Particle& new_solution_x)
 {
     std::vector<Particle> new_archive;
 
     bool new_solution_dominated = false;
 
     for (size_t i = 0; i < archive.size(); ++i) {
         const auto& archive_fx = archive[i].get_fx();
         const auto& new_solution_fx = new_solution_x.get_fx();
 
         if (is_dominated(archive_fx, new_solution_fx)) {
             continue;
         }
         if (is_dominated(new_solution_fx, archive_fx) || 
             new_solution_fx == archive_fx) 
         {
             new_solution_dominated = true;
         }
 
         new_archive.push_back(archive[i]);
     }
 
     if (!new_solution_dominated) {
         new_archive.push_back(new_solution_x);
     }
     std::swap(archive, new_archive);
 }
 