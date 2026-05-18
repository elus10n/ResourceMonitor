#include <iostream>
#include <chrono>
#include <thread>

#include "backend/include/orchestrator.h"

int main()
{
    Orchestrator orch;
    orch.run();
}