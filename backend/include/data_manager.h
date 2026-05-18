#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <unordered_map>
#include <cmath>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <string>
#include <filesystem>

#include "modules.h"
#include "parser.h"

#include "../libs/json.hpp"

class DataManager
{
    ParserOutput prev;

    Parser parser;

    logCallback callback_;

    nlohmann::json prepare_cpu_json(const CPUData &data) const;

    nlohmann::json prepare_memory_json(const MemoryData &data) const;

    nlohmann::json prepare_processes_json(const std::vector<ProcData> &data, const CPUData &current_cpu) const;
    nlohmann::json prepare_process_json(const ProcData &data) const;

    uint64_t get_total_jiffies(const KernelData& data) const;

    void report_log(const std::string& log) const;

    public:
    DataManager() : prev(parser.parse()) {}

    nlohmann::json get_data();

    void set_callback(logCallback callback) { callback_ = callback;}
};

#endif