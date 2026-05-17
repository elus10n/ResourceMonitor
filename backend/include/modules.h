#ifndef MODULES_H
#define MODULES_H

///Parser
struct ParserOutput
{
    CPUData cpu_data;
    MemoryData memory_data;
    std::vector<ProcData> proc_data;
};

struct CPUData
{
    KernelData total;
    std::vector<KernelData> kernel_data;
};

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

struct MemoryData
{
    //kb
    uint64_t mem_total;
    uint64_t mem_free;
    uint64_t mem_available;
};

struct ProcData
{
    ProcStat stat;

    ProcMem statm;

    // /proc/[pid]/cmdline
    std::string command;
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
///Parser

#endif