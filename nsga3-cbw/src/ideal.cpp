#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>

#include "global.h"
#include "rand.h"

/**
 * @brief Finds the ideal point in a population.
 *
 * This function determines the ideal point by finding the minimum value for each objective
 * among all feasible solutions in the population. A solution is considered feasible if its
 * constraint violation is non-negative.
 *
 * @param pop Pointer to the population where the ideal point will be found.
 */
void find_ideal_point(population *pop)
{
    int i, j;
    double temp;

    for (i = 0; i < nobj; i++)
    {
        ideal_point[i] = INF;
        for (j = 0; j < popsize; j++)
        {
            if (pop->ind[j].constr_violation >= 0.0)
            {
                temp = pop->ind[j].obj[i];
                if (temp < ideal_point[i])
                    ideal_point[i] = temp;
            }
        }
    }
}

/**
 * @brief Updates the ideal point using a population.
 *
 * This function updates the existing ideal point by checking if any objective values
 * in the current population are better (lower) than the current ideal point values.
 * Only feasible solutions (constraint violation >= 0.0) are considered.
 *
 * @param pop Pointer to the population used to update the ideal point.
 */
void update_ideal_point(population *pop)
{
    int i, j;
    double temp;
    for (i = 0; i < nobj; i++)
    {
        for (j = 0; j < popsize; j++)
        {
            if (pop->ind[j].constr_violation >= 0.0)
            {
                temp = pop->ind[j].obj[i];
                if (temp < ideal_point[i])
                    ideal_point[i] = temp;
            }
        }
    }
}