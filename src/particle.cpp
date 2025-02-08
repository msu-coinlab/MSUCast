/**
 * @file particle.cpp
 * @brief Implements the Particle class used in a particle swarm optimization (PSO) algorithm.
 */

 #include <vector>
 #include <iostream>
 #include <random>
 #include <algorithm>
 #include <crossguid/guid.hpp>
 #include <fmt/core.h>
 #include "json.hpp"
 #include <fstream>
 #include "particle.h"
 #include "external_archive.h"
 
 using json = nlohmann::json;
 
 namespace {
     std::random_device rd;
     std::mt19937 gen(rd());
     std::uniform_real_distribution<> dis(0, 1);
 
     /**
      * @brief Generates a random double between the given bounds.
      * @param lower_bound The lower limit of the random value.
      * @param upper_bound The upper limit of the random value.
      * @return A randomly generated double within the specified range.
      */
     double rand_double(double lower_bound, double upper_bound) {
         return lower_bound + dis(gen) * (upper_bound - lower_bound);
     }
 }
 
 /**
  * @brief Constructs a Particle with the given parameters.
  * @param dim Number of dimensions.
  * @param nobjs Number of objective functions.
  * @param w Inertia weight.
  * @param c1 Cognitive coefficient.
  * @param c2 Social coefficient.
  * @param lb Lower bound for the position values.
  * @param ub Upper bound for the position values.
  */
 Particle::Particle(int dim, int nobjs, double w, double c1, double c2, double lb, double ub) {
     fmt::print("{}\n", dim);
     this->dim = dim;
     this->nobjs = nobjs;
     this->w = w;
     this->c1 = c1;
     this->c2 = c2;
 
     this->x = std::vector<double>(dim);
     this->v = std::vector<double>(dim);
     this->fx = std::vector<double>(nobjs);
     this->pbest_x = std::vector<double>(dim);
     this->pbest_fx = std::vector<double>(nobjs);
     this->lower_bound = lb;
     this->upper_bound = ub;
     this->uuid_ = xg::newGuid().str();
     this->lc_cost_ = 0.0;
     this->animal_cost_ = 0.0;
     this->manure_cost_ = 0.0;
 }
 
 /**
  * @brief Copy constructor for Particle.
  * @param p The Particle object to copy.
  */
 Particle::Particle(const Particle &p) {
     this->dim = p.dim;
     this->nobjs = p.nobjs;
     this->w = p.w;
     this->c1 = p.c1;
     this->c2 = p.c2;
     this->x = p.x;
     this->v = p.v;
     this->fx = p.fx;
     this->pbest_x = p.pbest_x;
     this->pbest_fx = p.pbest_fx;
     this->lower_bound = p.lower_bound;
     this->upper_bound = p.upper_bound;
     this->uuid_ = p.uuid_;
     this->lc_x_ = p.lc_x_;
     this->animal_x_ = p.animal_x_;
     this->manure_x_ = p.manure_x_;
     this->lc_cost_ = p.lc_cost_;
     this->animal_cost_ = p.animal_cost_;
     this->manure_cost_ = p.manure_cost_;
     this->amount_plus_ = p.amount_plus_;
     this->amount_minus_ = p.amount_minus_;
 }
 
 /**
  * @brief Assignment operator for Particle.
  * @param p The Particle object to assign.
  * @return A reference to the assigned Particle object.
  */
 Particle& Particle::operator=(const Particle &p) {
     if (this == &p) {
         return *this;
     }
     this->dim = p.dim;
     this->nobjs = p.nobjs;
     this->w = p.w;
     this->c1 = p.c1;
     this->c2 = p.c2;
     this->x = p.x;
     this->fx = p.fx;
     this->v = p.v;
     this->pbest_x = p.pbest_x;
     this->pbest_fx = p.pbest_fx;
     this->lower_bound = p.lower_bound;
     this->upper_bound = p.upper_bound;
     this->uuid_ = p.uuid_;
     this->lc_x_ = p.lc_x_;
     this->animal_x_ = p.animal_x_;
     this->manure_x_ = p.manure_x_;
     this->lc_cost_ = p.lc_cost_;
     this->animal_cost_ = p.animal_cost_;
     this->manure_cost_ = p.manure_cost_;
     this->amount_plus_ = p.amount_plus_;
     this->amount_minus_ = p.amount_minus_;
     return *this;
 }
 
 /**
  * @brief Initializes the particle's position and velocity randomly.
  */
 void Particle::init() {
     for (int i = 0; i < dim; i++) {
         x[i] = rand_double(lower_bound, upper_bound);
         v[i] = 0.0;
     }
 }
 
 /**
  * @brief Initializes the particle's position with a given vector.
  * @param xp A vector representing the initial position.
  */
 void Particle::init(const std::vector<double> &xp) {
     for (int i = 0; i < dim; i++) {
         x[i] = xp[i];
         v[i] = 0.0;
     }
 }
 
 /**
  * @brief Initializes the personal best values of the particle.
  */
 void Particle::init_pbest() {
     pbest_x = x;
     pbest_fx = fx;
 }
 
 /**
  * @brief Updates the particle's velocity and position based on personal and global bests.
  * @param gbest_x The global best position vector.
  */
 void Particle::update(const std::vector<double> &gbest_x) {
     for (int i = 0; i < dim; i++) {
         double inertia = w * v[i];
         double cognitive = c1 * rand_double(0.0, 1.0) * (pbest_x[i] - x[i]);
         double social = c2 * rand_double(0.0, 1.0) * (gbest_x[i] - x[i]);
         v[i] = inertia + cognitive + social;
 
         x[i] = x[i] + v[i];
 
         if (x[i] < lower_bound) {
             x[i] = lower_bound;
         }
         if (x[i] > upper_bound) {
             x[i] = upper_bound;
         }
     }
 }
 
 /**
  * @brief Stores additional particle amount data to a JSON file.
  * @param filename The name of the output file.
  */
 void Particle::store_amount_plus_minus(const std::string& filename) {
     json json_obj;
     json_obj["plus"] = amount_plus_;
     json_obj["minus"] = amount_minus_;
 
     std::ofstream file(filename);
     file << json_obj.dump();
     file.close();
 }
 
 /**
  * @brief Updates the personal best of the particle if the new position is better.
  */
 void Particle::update_pbest() {
     if (is_dominated(pbest_fx, fx) || !is_dominated(fx, pbest_fx)) {
         pbest_x = x;
         pbest_fx = fx;
     }
 }
 
 /**
  * @brief Evaluates the fitness of the particle using a sample objective function.
  */
 void Particle::evaluate() {
     double fx0 = 0.0;
     double fx1 = 0.0;
 
     for (int i = 0; i < dim; i++) {
         fx0 += x[i] * x[i];
         fx1 += (x[i] - 2.0) * (x[i] - 2.0);
     }
     fx[0] = fx0;
     fx[1] = fx1;
 }
 
 /**
  * @brief Sets the fitness values of the particle.
  * @param fx1 First objective function value.
  * @param fx2 Second objective function value.
  */
 void Particle::set_fx(double fx1, double fx2) {
     fx[0] = fx1;
     fx[1] = fx2;
 }
 