/* Routine for evaluating population members  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <filesystem>
#include <fstream>
#include "global.h"
#include "rand.h"
#include <unordered_map>
#include <sw/redis++/redis++.h>
#include <fmt/core.h>
#include <chrono>
#include <thread>
#include <misc.hpp>
#include <iostream>
#include "crossguid/guid.hpp"
#include <SimpleAmqpClient/SimpleAmqpClient.h>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

// Global variables
std::unordered_map<std::string, int> uuid2idx; /**< Map to associate UUIDs with indices */
std::string emo_uuid; /**< UUID for the current evaluation */
constexpr auto EXCHANGE_NAME = "opt4cast_exchange"; /**< Name of the AMQP exchange */

std::string AMQP_HOST = getEnvVar("AMQP_HOST");
std::string AMQP_USERNAME = getEnvVar("AMQP_USERNAME");
std::string AMQP_PASSWORD = getEnvVar("AMQP_PASSWORD");
std::string AMQP_PORT = getEnvVar("AMQP_PORT");

std::string REDIS_HOST = getEnvVar("REDIS_HOST");
std::string REDIS_PORT = getEnvVar("REDIS_PORT");
std::string REDIS_DB = getEnvVar("REDIS_DB");
std::string REDIS_DB_OPT = getEnvVar("REDIS_DB_OPT");
std::string REDIS_SERVER_DATA = fmt::format("tcp://{}:{}/{}", REDIS_HOST, REDIS_PORT, REDIS_DB_OPT);
std::map<std::string, std::string> uuid_registry; /**< Registry for UUIDs */
std::map<std::string, std::string> uuid_mixed_registry; /**< Registry for mixed UUIDs */
std::map<std::string, std::string> uuid_surviving_pop_registry; /**< Registry for surviving population UUIDs */

auto redis = sw::redis::Redis(REDIS_SERVER_DATA); /**< Redis connection */
std::string opt4cast_evaluation; /**< Evaluation string for opt4cast */

/**
 * @brief Splits a string by a delimiter and stores the results in a vector.
 * 
 * @param str The string to split.
 * @param delim The delimiter character.
 * @param out The vector to store the split strings.
 */
void split_str(std::string const &str, const char delim, std::vector<std::string> &out) {
    std::stringstream s(str);
    std::string s2;

    while (std::getline(s, s2, delim)) {
        out.push_back(s2); // store the string in s2
    }
}

/**
 * @brief Gets the current time in milliseconds since epoch.
 * 
 * @return Current time in milliseconds.
 */
long get_time() {
    namespace sc = std::chrono;
    auto time = sc::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = sc::duration_cast<sc::milliseconds>(since_epoch);
    long now = millis.count(); // just like java (new Date()).getTime();
    return now;
}

/**
 * @brief Maintains the non-dominated solutions (NDS) for a given UUID.
 * 
 * @param emo_uuid The UUID for the evaluation.
 * @param cost The cost of the solution.
 * @param load The load of the solution.
 */
void maintain_nds(std::string emo_uuid, double cost, double load) {
    std::string current_nds_str = *redis.hget("nds", emo_uuid);
    std::vector<std::string> nds_list;
    split_str(current_nds_str, '_', nds_list);
    std::vector<std::vector<double>> new_nds;
    std::string new_nds_str = "";
    bool flag = true;
    for (int i(0); i < (int)nds_list.size(); i += 2) {
        double cost_nds = std::stod(nds_list[i]);
        double load_nds = std::stod(nds_list[i + 1]);
        if (!(cost <= cost_nds && load <= load_nds)) {
            new_nds.push_back({cost_nds, load_nds});
            new_nds_str = fmt::format("{}{}_{}_", new_nds_str, cost_nds, load_nds);
        } else if (cost >= cost_nds && load >= load_nds) {
            flag = false;
        }
    }
    if (flag) {
        new_nds_str = fmt::format("{}{}_{}_", new_nds_str, cost, load);
    }
    new_nds_str = new_nds_str.substr(0, new_nds_str.size() - 1);
}

/**
 * @brief Gets the AMQP connection options.
 * 
 * @return The AMQP connection options.
 */
AmqpClient::Channel::OpenOpts get_opts() {
    AmqpClient::Channel::OpenOpts opts;
    opts.host = AMQP_HOST.c_str();
    opts.port = std::stoi(AMQP_PORT);
    opts.vhost = "/";
    opts.frame_max = 131072;
    opts.auth = AmqpClient::Channel::OpenOpts::BasicAuth(AMQP_USERNAME, AMQP_PASSWORD);
    return opts;
}

/**
 * @brief Sends a message to a specified routing name.
 * 
 * @param routing_name The routing name for the message.
 * @param msg The message to send.
 * @return true if the message was sent successfully, false otherwise.
 */
bool send_message(std::string routing_name, std::string msg) {
    auto passive = false; // meaning you want the server to create the exchange if it does not already exist.
    auto durable = true; // meaning the exchange will survive a broker restart
    auto auto_delete = false; // meaning the queue will not be deleted once the channel is closed
    try {
        AmqpClient::Channel::OpenOpts opts = get_opts();
        auto channel = AmqpClient::Channel::Open(opts);
        channel->DeclareExchange(EXCHANGE_NAME, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT, passive, durable, auto_delete);
        auto message = AmqpClient::BasicMessage::Create(msg);
        channel->BasicPublish(EXCHANGE_NAME, routing_name, message, false, false);
    }
    catch (const std::exception &error) {
        std::clog << "error\n" << error.what() << "\n";
        std::cout << "error\n" << error.what() << "\n";
    }
    return true;
}

/**
 * @brief Converts a vector of doubles to a string representation.
 * 
 * @param nds The vector of doubles to convert.
 * @return A string representation of the vector.
 */
std::string nds2str(std::vector<double> nds) {
    std::string accum;

    for (auto val : nds) {
        accum += fmt::format("_{:.2f}", val);
    }

    return accum;
}

/**
 * @brief Evaluates the population using a mathematical model.
 * 
 * @param pop Pointer to the population structure.
 * @param is_mixed_pop Indicates if the population is mixed.
 * @param mixed_gen The generation number for mixed populations.
 */
void evaluate_pop_mathmodel(population *pop, bool is_mixed_pop, int mixed_gen) {
    for (int i = 0; i < popsize; i++) {
        auto scenario_id = *redis.lpop("scenario_ids");
        auto exec_uuid = xg::newGuid().str();
        int nrows = evaluate_ind(&(pop->ind[i]), emo_uuid, exec_uuid);

        if (is_mixed_pop) {
            auto uuid_registry_idx = fmt::format("{}_{}", mixed_gen, i);
            uuid_surviving_pop_registry[uuid_registry_idx] = exec_uuid;
        }
        else {
            auto uuid_registry_idx = fmt::format("{}_{}", curr_gen, i);
            uuid_registry[uuid_registry_idx] = exec_uuid;
        }
        redis.lpush("scenario_ids", scenario_id);
    }
    return;
}

/**
 * @brief Evaluates the population in parallel.
 * 
 * @param pop Pointer to the population structure.
 * @param is_mixed_pop Indicates if the population is mixed.
 * @param mixed_gen The generation number for mixed populations.
 */
void evaluate_pop_parallel(population *pop, bool is_mixed_pop, int mixed_gen) {
    std::vector<double> cost;
    std::vector<double> load;
    std::vector<long> time_mathmodel;
    std::map<int, long> begin_time_cast;
    int _my_popsize = popsize;

    for (int i = 0; i < _my_popsize; i++) {
        auto scenario_id = *redis.lpop("scenario_ids");
        auto exec_uuid = xg::newGuid().str();
        long begin_time = get_time();
        int nrows = evaluate_ind(&(pop->ind[i]), emo_uuid, exec_uuid);
        long end_time = get_time();
        time_mathmodel.push_back(end_time - begin_time);

        if (is_mixed_pop) {
            auto uuid_registry_idx = fmt::format("{}_{}", mixed_gen, i);
            uuid_surviving_pop_registry[uuid_registry_idx] = exec_uuid;
        }
        else {
            auto uuid_registry_idx = fmt::format("{}_{}", curr_gen, i);
            uuid_registry[uuid_registry_idx] = exec_uuid;
        }

        if (nrows > 0) {
            redis.hset("solution_to_execute_dict", exec_uuid, fmt::format("{}_{}", emo_uuid, scenario_id));
            uuid2idx[exec_uuid] = i;

            begin_time = get_time();
            begin_time_cast[i] = begin_time;
            try {
                auto msg = exec_uuid;
                auto routing_name = "opt4cast_execution";
                send_message(routing_name, msg);
            }
            catch (const std::exception &error) {
                std::cout << "error en evaluate parallel " << error.what() << std::endl;
            }
        } else {
            redis.lpush("scenario_ids", scenario_id);
        }
    }

    int not_sent = _my_popsize - uuid2idx.size();

    int counter = 0;
    AmqpClient::Channel::OpenOpts opts = get_opts();
    auto channel = AmqpClient::Channel::Open(opts);
    auto passive = false; 
    auto durable = true; 
    auto auto_delete = false; 
    channel->DeclareExchange(EXCHANGE_NAME, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT, passive, durable, auto_delete);
    auto generate_queue_name = "";
    auto exclusive = false; 
    auto queue_name = channel->DeclareQueue(generate_queue_name, passive, durable, exclusive, auto_delete);
    channel->BindQueue(queue_name, EXCHANGE_NAME, emo_uuid);

    auto no_local = false; 
    auto no_ack = true; 
    auto message_prefetch_count = 1;
    auto consumer_tag = channel->BasicConsume(queue_name, generate_queue_name, no_local, no_ack, exclusive, message_prefetch_count);
    std::ofstream comparison_math_core_file("/opt/opt4cast/comparison_math_vs_core.csv", std::ios_base::app);
    std::ofstream time_mathmodel_cast("/opt/opt4cast/time_mathmodel_cast.csv", std::ios_base::app);
    while (uuid2idx.size() > 0) {
        try {
            std::cout << "remaining scenarios: " << uuid2idx.size() << std::endl;
            std::cout << fmt::format("[*] Waiting for execution service: {} \n", emo_uuid);
            auto envelop = channel->BasicConsumeMessage(consumer_tag);
            auto message_payload = envelop->Message()->Body();
            auto routing_key = envelop->RoutingKey();
            std::string exec_uuid = message_payload;

            if (routing_key == emo_uuid) {
                std::string exec_results_str = *redis.hget("executed_results", exec_uuid);
                long end_time = get_time();
                std::vector<std::string> exec_results_list;
                split_str(exec_results_str, '_', exec_results_list);
                double load_n = std::stod(exec_results_list[0]);
                double load_p = std::stod(exec_results_list[1]);
                double load_s = std::stod(exec_results_list[2]);

                int completed_ind = uuid2idx[exec_uuid];
                time_mathmodel_cast << fmt::format("{},{},{}\n", emo_uuid, time_mathmodel[completed_ind], end_time - begin_time_cast[completed_ind]);
                uuid2idx.erase(message_payload);
                std::cout << fmt::format("UUID: {}, Cost: {}, Load N (math model): {}, Load N (corecast) {}\n",
                                         exec_uuid,
                                         pop->ind[completed_ind].obj[0], pop->ind[completed_ind].obj[1], load_n);
                comparison_math_core_file << fmt::format("{:.2f},{:.2f}\n",
                                                         pop->ind[completed_ind].obj[1], load_n);
                pop->ind[completed_ind].obj[1] = load_n;
                cost.push_back(pop->ind[completed_ind].obj[0]);
                load.push_back(pop->ind[completed_ind].obj[1]);
                if (curr_gen == ngen) {
                    get_outcome_by_lrs(emo_uuid, exec_uuid, true);
                }
            } else {
                std::cout << fmt::format("Error at receiving message with routing_key {} and exec_uuid {}\n", emo_uuid,
                                         message_payload);
            }
        }
        catch (const std::exception &error) {
            std::cout << fmt::format("Error at sending scenarios: ") << error.what() << std::endl;
        }
    }
    comparison_math_core_file.close();
    time_mathmodel_cast.close();
    cost.insert(cost.end(), load.begin(), load.end());
    return;
}

/**
 * @brief Evaluates the population in serial.
 * 
 * @param pop Pointer to the population structure.
 */
void evaluate_pop_serial(population *pop) {
    int i;
    AmqpClient::Channel::OpenOpts opts;
    opts.host = AMQP_HOST.c_str();
    opts.port = std::stoi(AMQP_PORT);
    opts.vhost = "/";
    opts.frame_max = 131072;
    opts.auth = AmqpClient::Channel::OpenOpts::BasicAuth(AMQP_USERNAME, AMQP_PASSWORD);

    for (i = 0; i < popsize; i++) {
        auto scenario_id = *redis.lpop("scenario_ids");
        auto exec_uuid = xg::newGuid().str();
        redis.hset("solution_to_execute_dict", exec_uuid, fmt::format("{}_{}", emo_uuid, scenario_id));
        uuid2idx[exec_uuid] = i;
        std::cout << "evaluating " << scenario_id << std::endl;

        auto passive = false; 
        auto durable = true; 
        auto auto_delete = false; 
        try {
            auto channel = AmqpClient::Channel::Open(opts);
            channel->DeclareExchange(EXCHANGE_NAME, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT, passive, durable, auto_delete);
            auto message = AmqpClient::BasicMessage::Create(exec_uuid);
            auto routing_name = "opt4cast_execution";
            channel->BasicPublish(EXCHANGE_NAME, routing_name, message, false, false);
        }
        catch (const std::exception &error) {
            // Handle error
        }
    }
    return;
}

/**
 * @brief Evaluates the objective function values and constraints for a population.
 * 
 * @param pop Pointer to the population structure.
 * @param curr_gen Current generation number.
 * @param ngen Total number of generations.
 * @param corecast_gen Generation number for corecasting.
 * @param is_mixed_pop Indicates if the population is mixed.
 * @param mixed_gen The generation number for mixed populations.
 */
void evaluate_pop(population *pop, int curr_gen, int ngen, int corecast_gen, bool is_mixed_pop, int mixed_gen) {
    auto msg = fmt::format("{}_{}_{}", emo_uuid, curr_gen, ngen);
    auto routing_name = "opt4cast_begin_generation";
    send_message(routing_name, msg);

    if (curr_gen >= corecast_gen) {
        evaluate_pop_parallel(pop, is_mixed_pop, mixed_gen);
    }
    else {
        evaluate_pop_mathmodel(pop, is_mixed_pop, mixed_gen);
    }

    routing_name = "opt4cast_end_generation";
    msg = fmt::format("{}_{}_{}", emo_uuid, curr_gen, ngen);
    send_message(routing_name, msg);
}

/**
 * @brief Evaluates the objective function values and constraints for an individual.
 * 
 * @param ind Pointer to the individual structure.
 * @param emo_uuid The UUID for the evaluation.
 * @param exec_uuid The execution UUID.
 * @return The number of rows evaluated.
 */
int evaluate_ind(individual *ind, std::string emo_uuid, std::string exec_uuid) {
    int j;
    int nrows = test_problem2(ind->xreal, ind->xbin, ind->gene, ind->obj, ind->constr, emo_uuid, exec_uuid);

    auto filename = fmt::format("/opt/opt4cast/output/nsga3/{}/{}_output_t.csv", emo_uuid, exec_uuid);
    CSVReader fx(filename);
    std::vector<std::vector<std::string>> data_list = fx.getData();
    double total_cost = 0;
    int flag = true;
    for (std::vector<std::string> vec : data_list) {
        if (flag == true) {
            flag = false;
            continue;
        }
        total_cost += std::stod(vec[12]);
    }
    ind->obj[0] = total_cost;
    ind->constr_violation = 0.0;
    for (j = 0; j < ncon; j++) {
        if (ind->constr[j] < 0.0) {
            ind->constr_violation += ind->constr[j];
        }
    }
    return nrows;
}