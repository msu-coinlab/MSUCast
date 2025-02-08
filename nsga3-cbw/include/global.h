/* This file contains the variable and function declarations */

# ifndef _GLOBAL_H_
# define _GLOBAL_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>

#define INF 1.0e14
#define EPS 1.0e-14
const double E = 2.71828182845905;
#define PI 3.14159265358979
#define GNUPLOT_COMMAND "cat"
//"gnuplot -persist"
#define NMAX 5000

extern std::vector<double> true_pf_lb;
extern std::vector<double> true_pf_ub;
extern double hv_raw_pf, pct_hv_to_exit;
extern int n_injected_points;
extern std::string injected_points_filename;
extern std::string opt4cast_evaluation;
extern std::string emo_uuid;
extern bool send_message(std::string routing_name, std::string msg);
extern std::map<std::string, std::string> uuid_registry;
extern std::map<std::string, std::string> uuid_surviving_pop_registry;
extern std::map<std::string, std::string> uuid_mixed_registry;

/**
 * @struct individual
 * @brief A structure representing an individual in the population.
 */
typedef struct
{
    int rank; ///< The rank of the individual.
    double constr_violation; ///< The constraint violation value.
    double *xreal; ///< The real-valued variables.
    int **gene; ///< The binary gene representation.
    double *xbin; ///< The binary variables.
    double *obj; ///< The objective values.
    double *constr; ///< The constraint values.
    double crowd_dist; ///< The crowding distance.
}
individual;

/**
 * @struct population
 * @brief A structure representing a population of individuals.
 */
typedef struct
{
    individual *ind; ///< The array of individuals.
}
population;

/**
 * @struct list
 * @brief A structure representing a linked list.
 */
typedef struct lists
{
    int index; ///< The index value.
    struct lists *parent; ///< The parent node.
    struct lists *child; ///< The child node.
}
list;

extern int nreal;
extern int nbin;
extern int nobj;
extern int ncon;
extern int popsize;
extern double pcross_real;
extern double pcross_bin;
extern double pmut_real;
extern double pmut_bin;
extern double eta_c;
extern double eta_m;
extern int ngen;
extern int nbinmut;
extern int nrealmut;
extern int nbincross;
extern int nrealcross;
extern int *nbits;
extern double *min_realvar;
extern double *max_realvar;
extern double *min_binvar;
extern double *max_binvar;
extern int bitlength;
extern int choice;
extern int obj1;
extern int obj2;
extern int obj3;
extern int angle1;
extern int angle2;

/* Extra variables for MNSGA-II */
extern int nref;
extern int curr_gen;
extern double *ref_pt[NMAX];
extern double *ideal_point;
extern double **plane_point;
extern int steps;
extern int onref;
extern int adaptive_increment;
extern int created_around[NMAX];
extern double scaling;
extern int active_ref_pts[10];
extern int start_incr;
extern std::string prefix_init_file;
extern int sols_init_file;

extern std::vector<double> hv_rp;

/**
 * @brief Allocates memory for a population.
 * @param pop The population pointer.
 * @param size The size of the population.
 */
void allocate_memory_pop(population *pop, int size);

/**
 * @brief Allocates memory for an individual.
 * @param ind The individual pointer.
 */
void allocate_memory_ind(individual *ind);

/**
 * @brief Deallocates memory for a population.
 * @param pop The population pointer.
 * @param size The size of the population.
 */
void deallocate_memory_pop(population *pop, int size);

/**
 * @brief Deallocates memory for an individual.
 * @param ind The individual pointer.
 */
void deallocate_memory_ind(individual *ind);

double maximum(double a, double b);
double minimum(double a, double b);

void crossover(individual *parent1, individual *parent2, individual *child1, individual *child2);
void realcross(individual *parent1, individual *parent2, individual *child1, individual *child2);
void bincross(individual *parent1, individual *parent2, individual *child1, individual *child2);

void decode_pop(population *pop);
void decode_ind(individual *ind);

void onthefly_display(population *pop, FILE *gp, int ii);
void onthefly_display2(population *pop, int ii);
int onthefly_display2(population *pop);

int check_dominance(individual *a, individual *b);

void evaluate_pop(population *pop, int curr_gen, int ngen, int corecast_gen, bool is_mixed_pop, int mixed_gen);
int evaluate_ind(individual *ind, std::string emo_uuid, std::string exec_uuid);

void initialize_pop(population *pop);
void initialize_ind(individual *ind, int);

void insert(list *node, int x);
list* del(list *node);

void merge(population *pop1, population *pop2, population *pop3);
void copy_ind(individual *ind1, individual *ind2);

void mutation_pop(population *pop);
void mutation_ind(individual *ind);
void bin_mutate_ind(individual *ind);
void real_mutate_ind(individual *ind);

int test_problem2(double *xreal, double *xbin, int **gene, double *obj, double *constr, std::string emo_uuid, std::string exec_uuid);
void test_problem(double *xreal, double *xbin, int **gene, double *obj, double *constr);

void report_pop(population *pop, FILE *fpt);
void report_feasible(population *pop, FILE *fpt);
void report_ind(individual *ind, FILE *fpt);

void selection(population *old_pop, population *new_pop);

/* Extra functions for MNSGA-II */
void create_ref_points(int p);
int generate_ref_points(int p, double **pts);
void assign_rank_mixedpop(population *new_pop);
void assign_rank(population *new_pop);
void find_ideal_point(population *pop);
void update_ideal_point(population *pop);
void elitist_sorting(population *mixed_pop, population *new_pop);
int Gaussian_Elimination(double *A, int n, double *B);

individual* tournament(individual *ind1, individual *ind2);

void fill_nondominated_sort(population *mixed_pop, population *new_pop);
void crowding_fill(population *mixed_pop, population *new_pop, int count, int front_size, list *cur);
void assign_crowding_distance_list(population *pop, list *lst, int front_size);
void assign_crowding_distance_indices(population *pop, int c1, int c2);
void assign_crowding_distance(population *pop, int *dist, int **obj_array, int front_size);
void quicksort_front_obj(population *pop, int objcount, int obj_array[], int obj_array_size);
void q_sort_front_obj(population *pop, int objcount, int obj_array[], int left, int right);
void quicksort_dist(population *pop, int *dist, int front_size);
void q_sort_dist(population *pop, int *dist, int left, int right);

#endif // _GLOBAL_H_
