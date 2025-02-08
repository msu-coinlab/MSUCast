#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

double seed;
double oldrand[55];
int jrand;

/**
 * @brief Initializes the random number generator.
 *
 * This function sets the initial state of the random number generator by
 * resetting the old random numbers and calling the warmup function.
 */
void randomize()
{
    int j1;
    for (j1 = 0; j1 <= 54; j1++)
    {
        oldrand[j1] = 0.0;
    }
    jrand = 0;
    warmup_random(seed);
    return;
}

/**
 * @brief Prepares the random number generator with a seed.
 *
 * This function initializes the random number generator using the provided seed
 * and generates the first batch of random numbers.
 *
 * @param seed The seed value to initialize the random number generator.
 */
void warmup_random(double seed)
{
    int j1, ii;
    double new_random, prev_random;
    oldrand[54] = seed;
    new_random = 0.000000001;
    prev_random = seed;
    for (j1 = 1; j1 <= 54; j1++)
    {
        ii = (21 * j1) % 54;
        oldrand[ii] = new_random;
        new_random = prev_random - new_random;
        if (new_random < 0.0)
        {
            new_random += 1.0;
        }
        prev_random = oldrand[ii];
    }
    advance_random();
    advance_random();
    advance_random();
    jrand = 0;
    return;
}

/**
 * @brief Advances the random number generator to create the next batch of random numbers.
 *
 * This function updates the state of the random number generator by generating
 * the next set of random numbers based on the current state.
 */
void advance_random()
{
    int j1;
    double new_random;
    for (j1 = 0; j1 < 24; j1++)
    {
        new_random = oldrand[j1] - oldrand[j1 + 31];
        if (new_random < 0.0)
        {
            new_random = new_random + 1.0;
        }
        oldrand[j1] = new_random;
    }
    for (j1 = 24; j1 < 55; j1++)
    {
        new_random = oldrand[j1] - oldrand[j1 - 24];
        if (new_random < 0.0)
        {
            new_random = new_random + 1.0;
        }
        oldrand[j1] = new_random;
    }
}

/**
 * @brief Fetches a single random number between 0.0 and 1.0.
 *
 * This function returns a random number from the uniform distribution in the range [0.0, 1.0].
 *
 * @return A random number between 0.0 and 1.0.
 */
double randomperc()
{
    jrand++;
    if (jrand >= 55)
    {
        jrand = 1;
        advance_random();
    }
    return ((double)oldrand[jrand]);
}

/**
 * @brief Fetches a single random integer between low and high (inclusive).
 *
 * This function returns a random integer within the specified bounds.
 *
 * @param low The lower bound (inclusive).
 * @param high The upper bound (inclusive).
 * @return A random integer between low and high.
 */
int rnd(int low, int high)
{
    int res;
    if (low >= high)
    {
        res = low;
    }
    else
    {
        res = low + (randomperc() * (high - low + 1));
        if (res > high)
        {
            res = high;
        }
    }
    return (res);
}

/**
 * @brief Fetches a single random real number between low and high (inclusive).
 *
 * This function returns a random real number within the specified bounds.
 *
 * @param low The lower bound (inclusive).
 * @param high The upper bound (inclusive).
 * @return A random real number between low and high.
 */
double rndreal(double low, double high)
{
    return (low + (high - low) * randomperc());
}