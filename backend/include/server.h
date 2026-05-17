#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include <mutex>
#include <unordered_set>
#include <string>
#include <cstdint>

#include "../libs/crow_all.h"
#include "modules.h"

class Server
{
    crow::SimpleApp app;
    std::thread server_thread;
    
    std::mutex mtx;
    std::unordered_set<crow::websocket::connection*> connections;

    logCallback callback_;

    void setup_routes();

    void report_log(const std::string& log) const;

    void report_error(const std::string& error) const;

    public:
    Server() = default;
    
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void start(uint16_t port = 8080);
    
    void stop();

    void broadcast(const std::string& json_data);

    void set_callback(logCallback callback) { callback_ = callback;}
};

#endif