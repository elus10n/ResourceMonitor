#include "../include/data_manager.h"

inline double round_to_two_places(double val) 
{
    return std::round(val * 100.0) / 100.0;
}

uint64_t DataManager::get_total_jiffies(const KernelData& data) const
{
    return data.user_j + data.nice_j + data.system_j + data.idle_j + 
           data.iowait_j + data.irq_j + data.softirq_j + data.steal_j + 
           data.guest_j + data.guest_nice_j;
}

nlohmann::json DataManager::get_data() 
{
    report_log("start parsing");
    ParserOutput current = parser.parse();
    report_log("stop parsing");

    nlohmann::json result = data_template;

    result["cpu"] = prepare_cpu_json(current.cpu_data);
    result["memory"] = prepare_memory_json(current.memory_data);
    result["processes"] = prepare_processes_json(current.proc_data, current.cpu_data);

    prev = current;

    return result;
}

nlohmann::json DataManager::prepare_cpu_json(const CPUData &data) const 
{
    nlohmann::json cpu_json = data_template["cpu"];

    uint64_t cur_total = get_total_jiffies(data.total);
    uint64_t prev_total = get_total_jiffies(prev.cpu_data.total);
    
    uint64_t cur_idle = data.total.idle_j + data.total.iowait_j;
    uint64_t prev_idle = prev.cpu_data.total.idle_j + prev.cpu_data.total.iowait_j;

    uint64_t delta_total = cur_total - prev_total;
    uint64_t delta_idle = cur_idle - prev_idle;

    if (delta_total > 0)
        cpu_json["total_percent"] = (1.0 - static_cast<double>(delta_idle) / delta_total) * 100.0;

    std::unordered_map<std::string, KernelData> prev_cores;
    for (const auto& core : prev.cpu_data.kernel_data)
        prev_cores[core.name] = core;

    for (const auto& core : data.kernel_data) 
    {
        nlohmann::json core_json = core_template;
        core_json["name"] = core.name;
        
        if (prev_cores.contains(core.name)) 
        {
            const auto& p_core = prev_cores[core.name];
            uint64_t d_tot = get_total_jiffies(core) - get_total_jiffies(p_core);
            uint64_t d_idl = (core.idle_j + core.iowait_j) - (p_core.idle_j + p_core.iowait_j);
            
            if (d_tot > 0)
                core_json["percent"] = (1.0 - static_cast<double>(d_idl) / d_tot) * 100.0;

        }
        
        cpu_json["cores"].push_back(core_json);
    }

    return cpu_json;
}

nlohmann::json DataManager::prepare_memory_json(const MemoryData &data) const 
{
    nlohmann::json mem_json = data_template["memory"];

    mem_json["total_kb"] = data.mem_total;
    mem_json["free_kb"] = data.mem_free;
    mem_json["available_kb"] = data.mem_available;

    if (data.mem_total > 0) 
    {
        double used = data.mem_total - data.mem_available;
        mem_json["usage_percent"] = (used / data.mem_total) * 100.0;
    }

    return mem_json;
}

nlohmann::json DataManager::prepare_processes_json(const std::vector<ProcData> &data, const CPUData &current_cpu) const 
{
    nlohmann::json proc_list = nlohmann::json::array();

    std::unordered_map<pid_t, ProcData> prev_procs;
    for (const auto& p : prev.proc_data)
        prev_procs[p.stat.pid] = p;

    uint64_t delta_system_total = get_total_jiffies(current_cpu.total) - get_total_jiffies(prev.cpu_data.total);

    for (const auto& proc : data) 
    {
        nlohmann::json p_json = prepare_process_json(proc);

        if (prev_procs.contains(proc.stat.pid) && delta_system_total > 0) 
        {
            const auto& p_proc = prev_procs[proc.stat.pid];
            
            uint64_t proc_delta = (proc.stat.user_j + proc.stat.system_j) - (p_proc.stat.user_j + p_proc.stat.system_j);
            
            double percent = (static_cast<double>(proc_delta) / delta_system_total) * 100.0;
            p_json["cpu_percent"] = round_to_two_places(percent);
        }

        if(p_json["cpu_percent"] > 0.0)
            proc_list.push_back(p_json);
    }

    return proc_list;
}

nlohmann::json DataManager::prepare_process_json(const ProcData &data) const 
{
    nlohmann::json p_json = process_template;

    p_json["pid"] = data.stat.pid;
    p_json["name"] = data.stat.e_name;
    p_json["state"] = std::string(1, data.stat.state);
    p_json["mem_size_pages"] = data.statm.size;
    p_json["mem_resident_pages"] = data.statm.resident;
    p_json["command"] = data.command;

    return p_json;
}

void DataManager::report_log(const std::string& log) const
{
    if(callback_) 
        callback_("[LOG]: " + log);
}