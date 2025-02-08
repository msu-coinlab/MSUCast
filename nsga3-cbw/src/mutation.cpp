/* Mutation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "misc.hpp"
#include "global.h"
#include "rand.h"

/**
 * @brief Performs mutation operations on an entire population.
 *
 * This function applies mutation to each individual in the population.
 *
 * @param pop Pointer to the population structure where mutation will be performed.
 */
void mutation_pop(population *pop)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        mutation_ind(&(pop->ind[i]));
    }
    return;
}

/**
 * @brief Performs mutation on a single individual.
 *
 * This function applies both real and binary mutation operations if applicable.
 *
 * @param ind Pointer to the individual to be mutated.
 */
void mutation_ind(individual *ind)
{
    if (nreal != 0)
    {
        real_mutate_ind(ind);
    }
    if (nbin != 0)
    {
        bin_mutate_ind(ind);
    }
    return;
}

/**
 * @brief Performs binary mutation on an individual.
 *
 * This function applies bit-flip mutation to the binary genes of an individual
 * based on the binary mutation probability (pmut_bin).
 *
 * @param ind Pointer to the individual whose binary genes will be mutated.
 */
void bin_mutate_ind(individual *ind)
{
    int j, k;
    double prob;
    for (j = 0; j < nbin; j++)
    {
        for (k = 0; k < nbits[j]; k++)
        {
            prob = randomperc();
            if (prob <= pmut_bin)
            {
                if (ind->gene[j][k] == 0)
                {
                    ind->gene[j][k] = 1;
                }
                else
                {
                    ind->gene[j][k] = 0;
                }
                nbinmut += 1;
            }
        }
    }
    return;
}

/**
 * @brief Performs polynomial mutation on real-valued variables.
 *
 * This function applies polynomial mutation to the real-valued variables of an individual.
 * The mutation is performed using a polynomial probability distribution and ensures the
 * mutated values stay within their specified bounds.
 *
 * @param ind Pointer to the individual whose real variables will be mutated.
 */
void real_mutate_ind(individual *ind)
{
    int j;
    double rnd, delta1, delta2, mut_pow, deltaq;
    double y, yl, yu, val, xy;
    for (j = 0; j < nreal; j++)
    {
        if (randomperc() <= pmut_real)
        {
            y = ind->xreal[j];
            yl = min_realvar[j];
            yu = max_realvar[j];
            delta1 = (y - yl) / (yu - yl);
            delta2 = (yu - y) / (yu - yl);
            rnd = randomperc();
            mut_pow = 1.0 / (eta_m + 1.0);
            if (rnd <= 0.5)
            {
                xy = 1.0 - delta1;
                val = 2.0 * rnd + (1.0 - 2.0 * rnd) * (pow(xy, (eta_m + 1.0)));
                deltaq = pow(val, mut_pow) - 1.0;
            }
            else
            {
                xy = 1.0 - delta2;
                val = 2.0 * (1.0 - rnd) + 2.0 * (rnd - 0.5) * (pow(xy, (eta_m + 1.0)));
                deltaq = 1.0 - (pow(val, mut_pow));
            }
            y = y + deltaq * (yu - yl);
            if (y < yl)
                y = yl;
            if (y > yu)
                y = yu;
            ind->xreal[j] = y;
            nrealmut += 1;
        }
    }
    return;
}