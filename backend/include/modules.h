#ifndef MODULES_H
#define MODULES_H

#include <functional>
#include <string>

#include "../libs/json.hpp"

///Logger
using logCallback = std::function<void(const std::string&)>;
///Logger

///Parser
struct KernelData
{
    std::string name;

    //jiffies
    uint64_t user_j;
    uint64_t nice_j;
    uint64_t system_j;
    uint64_t idle_j;
    uint64_t iowait_j;
    uint64_t irq_j;
    uint64_t softirq_j;
    uint64_t steal_j;
    uint64_t guest_j;
    uint64_t guest_nice_j;
};

struct CPUData
{
    KernelData total;
    std::vector<KernelData> kernel_data;
};

struct MemoryData
{
    //kb
    uint64_t mem_total;
    uint64_t mem_free;
    uint64_t mem_available;
};

struct ProcStat
{
    // /proc/[pid]/stat
    pid_t pid;
    std::string e_name;
    char state;
    uint64_t user_j;
    uint64_t system_j;

};

struct ProcMem
{
    // /proc/[pid]/statm
    uint64_t size; //pages
    uint64_t resident; //pages
};

struct ProcData
{
    ProcStat stat;

    ProcMem statm;

    // /proc/[pid]/cmdline
    std::string command;

    std::string owner;
};

struct ParserOutput
{
    CPUData cpu_data;
    MemoryData memory_data;
    std::vector<ProcData> proc_data;
};
///Parser

///DataManager
inline const nlohmann::json data_template = 
{
    {
        "cpu", 
        {
            {"total_percent", 0.0},
            {"cores", nlohmann::json::array()}
        }
    },
    {
        "memory", 
        {
            {"total_kb", 0},
            {"free_kb", 0},
            {"available_kb", 0},
            {"usage_percent", 0.0}
        }
    },
    {
        "processes",
        {
            {"count_of_processes", 0},
            {"count_of_zombie", 0},
            {"count_of_running", 0},
            {"count_of_sleeping", 0},
            {"processes", nlohmann::json::array()}
        }
    }
};

inline const nlohmann::json core_template =
{
    {"name", ""}, 
    {"percent", 0.0}
};

inline const nlohmann::json process_template = 
{
    {"pid", 0},
    {"owner", ""},
    {"name", ""},
    {"state", ""},
    {"cpu_percent", 0.0},
    {"mem_size_pages", 0},
    {"mem_resident_pages", 0},
    {"command", ""}
};
///DataManager

#endif