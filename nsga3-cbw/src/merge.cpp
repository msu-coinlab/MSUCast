/* Routine for mergeing two populations */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/**
 * @brief Merges two populations into a third population.
 *
 * This function combines two populations (pop1 and pop2) into a third population (pop3).
 * The first population is copied to the beginning of pop3, followed by the second population.
 *
 * @param pop1 Pointer to the first population to be merged.
 * @param pop2 Pointer to the second population to be merged.
 * @param pop3 Pointer to the destination population where the merged result will be stored.
 */
void merge(population *pop1, population *pop2, population *pop3)
{
    int i, k;
    for (i = 0; i < popsize; i++)
    {
        copy_ind(&(pop1->ind[i]), &(pop3->ind[i]));
    }
    for (i = 0, k = popsize; i < popsize; i++, k++)
    {
        copy_ind(&(pop2->ind[i]), &(pop3->ind[k]));
    }
    return;
}

/**
 * @brief Copies one individual to another.
 *
 * This function performs a deep copy of an individual's properties including:
 * - Rank and constraint violation
 * - Real-valued variables
 * - Binary variables and their gene representations
 * - Objective function values
 * - Constraint values
 *
 * @param ind1 Pointer to the source individual to be copied.
 * @param ind2 Pointer to the destination individual where the copy will be stored.
 */
void copy_ind(individual *ind1, individual *ind2)
{
    int i, j;
    ind2->rank = ind1->rank;
    ind2->constr_violation = ind1->constr_violation;
    if (nreal != 0)
    {
        for (i = 0; i < nreal; i++)
        {
            ind2->xreal[i] = ind1->xreal[i];
        }
    }
    if (nbin != 0)
    {
        for (i = 0; i < nbin; i++)
        {
            ind2->xbin[i] = ind1->xbin[i];
            for (j = 0; j < nbits[i]; j++)
            {
                ind2->gene[i][j] = ind1->gene[i][j];
            }
        }
    }
    for (i = 0; i < nobj; i++)
    {
        ind2->obj[i] = ind1->obj[i];
    }
    if (ncon != 0)
    {
        for (i = 0; i < ncon; i++)
        {
            ind2->constr[i] = ind1->constr[i];
        }
    }
    return;
}