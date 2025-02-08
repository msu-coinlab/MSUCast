#ifndef PARTICLE_H
#define PARTICLE_H

#include <iostream>
#include <vector>
#include "scenario.h"

/**
 * @class Particle
 * @brief A class representing a particle for optimization algorithms.
 */
class Particle {
public:
    /**
     * @brief Constructor to initialize a particle.
     * @param dim Dimension of the particle.
     * @param nobjs Number of objectives.
     * @param w Inertia weight.
     * @param c1 Cognitive coefficient.
     * @param c2 Social coefficient.
     * @param lb Lower bound.
     * @param ub Upper bound.
     */
    Particle(int dim, int nobjs, double w, double c1, double c2, double lb, double ub);

    /**
     * @brief Default constructor.
     */
    Particle() = default;

    /**
     * @brief Copy constructor.
     * @param p The particle to copy.
     */
    Particle(const Particle &p);

    /**
     * @brief Default destructor.
     */
    ~Particle() = default;

    /**
     * @brief Assignment operator.
     * @param p The particle to assign from.
     * @return Reference to the assigned particle.
     */
    Particle& operator=(const Particle &p);

    /**
     * @brief Initializes the particle.
     */
    void init();

    /**
     * @brief Initializes the particle with a given position.
     * @param xp The position vector.
     */
    void init(const std::vector<double> &xp);

    /**
     * @brief Updates the particle's position and velocity.
     * @param gbest_x The global best position.
     */
    void update(const std::vector<double> &gbest_x);

    /**
     * @brief Evaluates the particle's fitness.
     */
    void evaluate();

    /**
     * @brief Gets the current position of the particle.
     * @return The position vector.
     */
    const std::vector<double>& get_x() const { return x; }

    /**
     * @brief Gets the personal best position of the particle.
     * @return The personal best position vector.
     */
    const std::vector<double>& get_pbest() const { return pbest_x; }

    /**
     * @brief Gets the fitness values of the particle.
     * @return The fitness values vector.
     */
    const std::vector<double>& get_fx() const { return fx; }

    /**
     * @brief Sets the fitness values of the particle.
     * @param fx1 The first fitness value.
     * @param fx2 The second fitness value.
     */
    void set_fx(double fx1, double fx2);

    /**
     * @brief Sets the UUID of the particle.
     * @param uuid The UUID string.
     */
    void set_uuid(const std::string& uuid) { uuid_ = uuid; }

    /**
     * @brief Gets the UUID of the particle.
     * @return The UUID string.
     */
    const std::string& get_uuid() const { return uuid_; }

    /**
     * @brief Initializes the personal best position.
     */
    void init_pbest();

    /**
     * @brief Updates the personal best position.
     */
    void update_pbest();

    /**
     * @brief Gets the land cover data of the particle.
     * @return The land cover data vector.
     */
    const std::vector<std::tuple<int, int, int, int, double>> get_lc_x() const { return lc_x_; }

    /**
     * @brief Gets the animal data of the particle.
     * @return The animal data vector.
     */
    const std::vector<std::tuple<int, int, int, int, int, double>> get_animal_x() const { return animal_x_; }

    /**
     * @brief Gets the manure data of the particle.
     * @return The manure data vector.
     */
    const std::vector<std::tuple<int, int, int, int, int, double>> get_manure_x() const { return manure_x_; }

    /**
     * @brief Sets the land cover data of the particle.
     * @param lc_x The land cover data vector.
     */
    void set_lc_x(const std::vector<std::tuple<int, int, int, int, double>>& lc_x) { lc_x_ = lc_x; }

    /**
     * @brief Sets the animal data of the particle.
     * @param animal_x The animal data vector.
     */
    void set_animal_x(const std::vector<std::tuple<int, int, int, int, int, double>>& animal_x) { animal_x_ = animal_x; }

    /**
     * @brief Sets the manure data of the particle.
     * @param manure_x The manure data vector.
     */
    void set_manure_x(const std::vector<std::tuple<int, int, int, int, int, double>>& manure_x) { manure_x_ = manure_x; }

    /**
     * @brief Sets the land cover cost of the particle.
     * @param lc_cost The land cover cost.
     */
    void set_lc_cost(double lc_cost) { lc_cost_ = lc_cost; }

    /**
     * @brief Gets the land cover cost of the particle.
     * @return The land cover cost.
     */
    const double get_lc_cost() const { return lc_cost_; }

    /**
     * @brief Sets the animal cost of the particle.
     * @param animal_cost The animal cost.
     */
    void set_animal_cost(double animal_cost) { animal_cost_ = animal_cost; }

    /**
     * @brief Sets the manure cost of the particle.
     * @param manure_cost The manure cost.
     */
    void set_manure_cost(double manure_cost) { manure_cost_ = manure_cost; }

    /**
     * @brief Gets the animal cost of the particle.
     * @return The animal cost.
     */
    const double get_animal_cost() const { return animal_cost_; }

    /**
     * @brief Gets the manure cost of the particle.
     * @return The manure cost.
     */
    const double get_manure_cost() const { return manure_cost_; }

    /**
     * @brief Sets the amount plus map of the particle.
     * @param amount_plus The amount plus map.
     */
    void set_amount_plus(const std::unordered_map<std::string, double>& amount_plus) { amount_plus_ = amount_plus; }

    /**
     * @brief Gets the amount plus map of the particle.
     * @return The amount plus map.
     */
    const std::unordered_map<std::string, double>& get_amount_plus() const { return amount_plus_; }

    /**
     * @brief Sets the amount minus map of the particle.
     * @param amount_minus The amount minus map.
     */
    void set_amount_minus(const std::unordered_map<std::string, double>& amount_minus) { amount_minus_ = amount_minus; }

    /**
     * @brief Gets the amount minus map of the particle.
     * @return The amount minus map.
     */
    const std::unordered_map<std::string, double>& get_amount_minus() const { return amount_minus_; }

    /**
     * @brief Stores the amount plus and minus data to a file.
     * @param filename The name of the file.
     */
    void store_amount_plus_minus(const std::string& filename);

private:
    int dim; ///< The dimension of the particle.
    int nobjs; ///< The number of objectives.
    std::vector<double> x; ///< The current position of the particle.
    std::vector<double> fx; ///< The fitness values of the particle.
    std::vector<double> v; ///< The velocity of the particle.
    std::string uuid_; ///< The UUID of the particle.
    double w, c1, c2; ///< The weights and coefficients for the particle's velocity update.
    std::vector<double> pbest_x; ///< The personal best position of the particle.
    std::vector<double> pbest_fx; ///< The personal best fitness values of the particle.
    double lower_bound; ///< The lower bound for the particle's position.
    double upper_bound; ///< The upper bound for the particle's position.
    std::vector<std::tuple<int, int, int, int, double>> lc_x_; ///< The land cover data.
    std::vector<std::tuple<int, int, int, int, int, double>> animal_x_; ///< The animal data.
    std::vector<std::tuple<int, int, int, int, int, double>> manure_x_; ///< The manure data.

    std::unordered_map<std::string, double> amount_minus_; ///< The amount minus map.
    std::unordered_map<std::string, double> amount_plus_; ///< The amount plus map.
    double lc_cost_; ///< The land cover cost.
    double animal_cost_; ///< The animal cost.
    double manure_cost_; ///< The manure cost.
};

#endif // PARTICLE_H
