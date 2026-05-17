#include <iostream>
#include <chrono>
#include <thread>

#include "backend/include/data_manager.h"

int main()
{
    DataManager manager;
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    nlohmann::json data = manager.get_data();

    std::cout << data << std::endl;
}