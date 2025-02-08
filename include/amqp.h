//
// Created by gtoscano on 4/1/23.
//

#ifndef CBO_EVALUATION_AMQP_CPP_H
#define CBO_EVALUATION_AMQP_CPP_H

#include <string>
#include <unordered_map>

#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <sw/redis++/redis++.h>

/**
 * @class RabbitMQClient
 * @brief A class to interact with RabbitMQ and handle messaging.
 */
class RabbitMQClient {
public:
    /**
     * @brief Constructor with initialization parameters.
     * @param emo_data The data related to emotion.
     * @param emo_uuid The UUID related to emotion.
     */
    RabbitMQClient(std::string emo_data, std::string emo_uuid);

    /**
     * @brief Default constructor.
     */
    RabbitMQClient();

    /**
     * @brief Initializes the client with given parameters.
     * @param emo_data The data related to emotion.
     * @param emo_uuid The UUID related to emotion.
     */
    void init(std::string emo_data, std::string emo_uuid);

    /**
     * @brief Destructor.
     */
    ~RabbitMQClient();

    /**
     * @brief Fetches and sets options for the client.
     */
    void get_opts();

    /**
     * @brief Sends a message to a specified routing name.
     * @param routing_name The name of the routing key.
     * @param msg The message to be sent.
     * @return True if the message was sent successfully.
     */
    bool send_message(std::string routing_name, std::string msg);

    /**
     * @brief Sends a signal with a given execution UUID.
     * @param exec_uuid The UUID for the execution signal.
     */
    void send_signal(std::string exec_uuid);

    /**
     * @brief Waits for data to be received.
     * @return The received data as a string.
     */
    std::string wait_for_data();

    /**
     * @brief Waits for all data to be received.
     * @return A vector of all received data.
     */
    std::vector<std::string> wait_for_all_data();

    /**
     * @brief Safely waits for all data to be received.
     * @return A vector of all safely received data.
     */
    std::vector<std::string> safe_wait_for_all_data();

    /**
     * @brief Gets the number of remaining transfers.
     * @return The number of remaining transfers.
     */
    int transfers_remaining();

    /**
     * @brief Checks if the client is initialized.
     * @return True if the client is initialized.
     */
    bool is_init();

private:
    AmqpClient::Channel::OpenOpts opts_; /**< Options for opening the channel. */
    sw::redis::Redis redis_; /**< Redis client for interaction with Redis database. */
    std::string emo_uuid_; /**< UUID related to emotion. */
    std::string emo_data_; /**< Data related to emotion. */
    std::unordered_map<std::string, std::string> sent_list_; /**< List of sent messages. */
    bool is_initialized; /**< Flag to check if the client is initialized. */
};

#endif //CBO_EVALUATION_AMQP_CPP_H
