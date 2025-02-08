#ifndef EXTERNAL_ARCHIVE_H
#define EXTERNAL_ARCHIVE_H

#include <vector>
#include <string>
#include "particle.h"

/**
 * @brief Checks if the solution a is dominated by the solution b.
 * @param a The first solution vector.
 * @param b The second solution vector.
 * @return True if a is dominated by b.
 */
bool is_dominated(const std::vector<double>& a, const std::vector<double>& b);

/**
 * @brief Checks if the solution a is non-dominated by the solution b.
 * @param a The first solution vector.
 * @param b The second solution vector.
 * @return True if a is non-dominated by b.
 */
bool is_non_dominated(const std::vector<double>& a, const std::vector<double>& b);

/**
 * @brief Updates the list of non-dominated solutions.
 * 
 * @param solutions_x The current set of solutions.
 * @param new_solution_x The new solution to be considered.
 * @param solutions_fx The current set of objective function values.
 * @param new_solution_fx The objective function values of the new solution.
 */
void update_non_dominated_solutions(
    std::vector<std::vector<double>>& solutions_x, 
    const std::vector<double>& new_solution_x, 
    std::vector<std::vector<double>>& solutions_fx, 
    const std::vector<double>& new_solution_fx);

/**
 * @brief Updates the list of non-dominated solutions in the archive.
 * 
 * @param archive The archive containing the current non-dominated solutions.
 * @param new_solution_x The new solution to be considered.
 */
void update_non_dominated_solutions(
    std::vector<Particle>& archive, 
    const Particle& new_solution_x);

#endif // EXTERNAL_ARCHIVE_H
