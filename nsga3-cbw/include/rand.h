/* Declaration for random number related variables and routines */

# ifndef _RAND_H_
# define _RAND_H_

/* Variable declarations for the random number generator */
extern double seed; ///< The seed for the random number generator.
extern double oldrand[55]; ///< Array to store old random values.
extern int jrand; ///< Index for the random number generator.

/* Function declarations for the random number generator */

/**
 * @brief Randomizes the random number generator.
 */
void randomize(void);

/**
 * @brief Warms up the random number generator with a given seed.
 * @param seed The seed value to initialize the generator.
 */
void warmup_random(double seed);

/**
 * @brief Advances the state of the random number generator.
 */
void advance_random(void);

/**
 * @brief Generates a random percentage.
 * @return A random percentage value.
 */
double randomperc(void);

/**
 * @brief Generates a random integer within a given range.
 * @param low The lower bound of the range.
 * @param high The upper bound of the range.
 * @return A random integer within the range [low, high].
 */
int rnd(int low, int high);

/**
 * @brief Generates a random double within a given range.
 * @param low The lower bound of the range.
 * @param high The upper bound of the range.
 * @return A random double within the range [low, high].
 */
double rndreal(double low, double high);

# endif
