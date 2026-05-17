#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <chrono>
#include <thread>
#include <string>
#include <cstdint>
#include <atomic>
#include <csignal>

#include "server.h"
#include "data_manager.h"
#include "logger.h"

inline std::atomic<bool> is_running = true;

inline void signal_handler(int signal) { is_running = false;}

class Orchestrator
{
    Server server;
    DataManager data_manager;

    public:
    Orchestrator()
    {
        auto callback = Logger::get_callback(std::cout);
        server.set_callback(callback);
        data_manager.set_callback(callback);
    }

    ~Orchestrator() = default;

    Orchestrator(const Orchestrator&) = delete;
    Orchestrator& operator=(const Orchestrator&) = delete;

    void run(uint16_t port = 8080)
    {
        server.start(port);

        std::signal(SIGINT, signal_handler);

        while (is_running) 
        {
            auto start_time = std::chrono::steady_clock::now();

            nlohmann::json fresh_data = data_manager.get_data();
            
            server.broadcast(fresh_data.dump());

            auto end_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            if (elapsed < std::chrono::milliseconds(1000)) std::this_thread::sleep_for(std::chrono::milliseconds(1000) - elapsed);
        }
    }
};

#endif