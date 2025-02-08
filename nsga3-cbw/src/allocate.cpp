/* Memory allocation and deallocation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "global.h"

/**
 * @brief Allocates memory to a population.
 * @param pop The population pointer.
 * @param size The size of the population.
 */
void allocate_memory_pop(population *pop, int size)
{
    int i;
    pop->ind = (individual *)malloc(size * sizeof(individual));
    for (i = 0; i < size; i++)
    {
        allocate_memory_ind(&(pop->ind[i]));
    }
    return;
}

/**
 * @brief Allocates memory to an individual.
 * @param ind The individual pointer.
 */
void allocate_memory_ind(individual *ind)
{
    int j;
    if (nreal != 0)
    {
        ind->xreal = (double *)malloc(nreal * sizeof(double));
    }
    if (nbin != 0)
    {
        ind->xbin = (double *)malloc(nbin * sizeof(double));
        ind->gene = (int **)malloc(nbin * sizeof(int));
        for (j = 0; j < nbin; j++)
        {
            ind->gene[j] = (int *)malloc(nbits[j] * sizeof(int));
        }
    }
    ind->obj = (double *)malloc(nobj * sizeof(double));
    if (ncon != 0)
    {
        ind->constr = (double *)malloc(ncon * sizeof(double));
    }
    return;
}

/**
 * @brief Deallocates memory for a population.
 * @param pop The population pointer.
 * @param size The size of the population.
 */
void deallocate_memory_pop(population *pop, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        deallocate_memory_ind(&(pop->ind[i]));
    }
    free(pop->ind);
    return;
}

/**
 * @brief Deallocates memory for an individual.
 * @param ind The individual pointer.
 */
void deallocate_memory_ind(individual *ind)
{
    int j;
    if (nreal != 0)
    {
        free(ind->xreal);
    }
    if (nbin != 0)
    {
        for (j = 0; j < nbin; j++)
        {
            free(ind->gene[j]);
        }
        free(ind->xbin);
        free(ind->gene);
    }
    free(ind->obj);
    if (ncon != 0)
    {
        free(ind->constr);
    }
    return;
}
