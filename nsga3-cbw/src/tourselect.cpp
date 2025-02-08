/**
 * @file tourselect.cpp
 * @brief Tournament selection routines for genetic algorithms.
 * 
 * This file contains functions for performing tournament selection
 * and crossover between individuals in a population.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/**
 * @brief Performs tournament selection to create a new population.
 * 
 * This function creates a new population from the old population
 * by performing tournament selection and crossover.
 * 
 * @param old_pop Pointer to the old population.
 * @param new_pop Pointer to the new population to be created.
 */
void selection(population *old_pop, population *new_pop)
{
    int *a1, *a2;
    int temp;
    int i;
    int rand;
    individual *parent1, *parent2;
    a1 = (int *)malloc(popsize * sizeof(int));
    a2 = (int *)malloc(popsize * sizeof(int));
    for (i = 0; i < popsize; i++)
    {
        a1[i] = a2[i] = i;
    }
    for (i = 0; i < popsize; i++)
    {
        rand = rnd(i, popsize - 1);
        temp = a1[rand];
        a1[rand] = a1[i];
        a1[i] = temp;
        rand = rnd(i, popsize - 1);
        temp = a2[rand];
        a2[rand] = a2[i];
        a2[i] = temp;
    }
    for (i = 0; i < popsize; i += 4)
    {
        parent1 = tournament(&old_pop->ind[a1[i]], &old_pop->ind[a1[i + 1]]);
        parent2 = tournament(&old_pop->ind[a1[i + 2]], &old_pop->ind[a1[i + 3]]);
        crossover(parent1, parent2, &new_pop->ind[i], &new_pop->ind[i + 1]);
        parent1 = tournament(&old_pop->ind[a2[i]], &old_pop->ind[a2[i + 1]]);
        parent2 = tournament(&old_pop->ind[a2[i + 2]], &old_pop->ind[a2[i + 3]]);
        crossover(parent1, parent2, &new_pop->ind[i + 2], &new_pop->ind[i + 3]);
    }
    free(a1);
    free(a2);
    return;
}

/**
 * @brief Performs a binary tournament between two individuals.
 * 
 * This function selects one of the two individuals based on their
 * constraint violations and a random percentage.
 * 
 * @param ind1 Pointer to the first individual.
 * @param ind2 Pointer to the second individual.
 * @return Pointer to the selected individual.
 */
individual *tournament(individual *ind1, individual *ind2)
{
    if (ncon != 0)
    {
        if (ind1->constr_violation < 0 && ind2->constr_violation < 0)
        {
            if (ind1->constr_violation < ind2->constr_violation)
                return (ind2);
            if (ind1->constr_violation > ind2->constr_violation)
                return (ind1);

            if ((randomperc()) <= 0.5)
            {
                return (ind1);
            }
            else
            {
                return (ind2);
            }
        }
        else if (ind1->constr_violation >= 0 && ind2->constr_violation < 0)
            return (ind1);
        else if (ind1->constr_violation < 0 && ind2->constr_violation >= 0)
            return (ind2);
        else
        {
            if ((randomperc()) <= 0.5)
            {
                return (ind1);
            }
            else
            {
                return (ind2);
            }
        }
    }
    else
    {
        if ((randomperc()) <= 0.5)
        {
            return (ind1);
        }
        else
        {
            return (ind2);
        }
    }
}