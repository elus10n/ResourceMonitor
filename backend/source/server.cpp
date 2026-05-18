#include "../include/server.h"
#include <iostream>

Server::~Server()
{
    stop();
}

void Server::start(uint16_t port)
{
    report_log("Server started");
    setup_routes();

    server_thread = std::thread([this, port]() {
        try 
        {
            app.loglevel(crow::LogLevel::Warning);
            app.signal_clear();
            app.port(port).multithreaded().run();
        } 
        catch (const std::exception& e) 
        {
            report_error(std::string("Exception in server thread: ") + e.what());
        }
    });
}

void Server::stop()
{
    app.stop();
    
    if (server_thread.joinable())
        server_thread.join();

    report_log("Server stop");
}

void Server::setup_routes()
{
    CROW_ROUTE(app, "/")([]() {
        crow::response res;

        res.set_static_file_info("index.html");
        return res;
    });

    CROW_ROUTE(app, "/static/<string>")([](std::string filename) {
        crow::response res;

        res.set_static_file_info("static/" + filename);
        return res;
    });

    CROW_ROUTE(app, "/ws")
        .websocket(&app)
        .onopen([this](crow::websocket::connection& conn) 
        {
            report_log("Someone connected!");
            std::lock_guard<std::mutex> lock(mtx);
            connections.insert(&conn);
        })
        .onclose([this](crow::websocket::connection& conn, const std::string&) 
        {
            report_log("Someone disconnected!");
            std::lock_guard<std::mutex> lock(mtx);
            connections.erase(&conn);
        });
}

void Server::broadcast(const std::string& json_data)
{
    std::lock_guard<std::mutex> lock(mtx);
    for (auto* conn : connections)
        if (conn) conn->send_text(json_data);
}

void Server::report_log(const std::string& log) const
{
    if(callback_) 
        callback_("[LOG]: " + log);
}

void Server::report_error(const std::string& error) const
{
    if(callback_) 
        callback_("[ERROR]: " + error);
}