#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <unordered_map>

#include "modules.h"
#include "parser.h"

#include "../libs/json.hpp"

class DataManager
{
    ParserOutput prev;

    Parser parser;

    nlohmann::json prepare_cpu_json(const CPUData &data) const;

    nlohmann::json prepare_memory_json(const MemoryData &data) const;

    nlohmann::json prepare_processes_json(const std::vector<ProcData> &data, const CPUData &current_cpu) const;
    nlohmann::json prepare_process_json(const ProcData &data) const;

    uint64_t get_total_jiffies(const KernelData& data) const;

    public:
    DataManager() : prev(parser.parse()) {}

    nlohmann::json get_data();
};

#endif