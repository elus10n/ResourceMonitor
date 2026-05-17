#include "../include/parser.h"

ParserOutput Parser::parse() const
{
    ParserOutput output;
    output.cpu_data = cpu_parse();
    output.memory_data = mem_parse();
    output.proc_data = processes_parse();
    return output;
}

CPUData Parser::cpu_parse() const
{
    CPUData cpu_data;
    
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return cpu_data;

    std::string line;
    while (std::getline(file, line)) 
    {
        if (line.starts_with("cpu")) 
        {
            std::istringstream ss(line);
            KernelData kernel;
            
            ss >> kernel.name;
            ss >> kernel.user_j >> kernel.nice_j >> kernel.system_j 
               >> kernel.idle_j >> kernel.iowait_j >> kernel.irq_j 
               >> kernel.softirq_j >> kernel.steal_j >> kernel.guest_j 
               >> kernel.guest_nice_j;
               
            if (kernel.name == "cpu") cpu_data.total = kernel;
            else cpu_data.kernel_data.push_back(kernel);
        } 
        else { break;}
    }

    return cpu_data;
}

MemoryData Parser::mem_parse() const
{
    MemoryData mem_data;

    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return mem_data;

    std::string key;
    uint64_t value;
    std::string unit;
    while (file >> key >> value >> unit)//<nm>: <count> kB
    {
        if (key == "MemTotal:") mem_data.mem_total = value; 
        else if (key == "MemFree:") mem_data.mem_free = value; 
        else if (key == "MemAvailable:") 
        {
            mem_data.mem_available = value;
            break;//последняя нужная
        }
    }

    return mem_data;
}

std::vector<ProcData> Parser::processes_parse() const
{
    std::vector<ProcData> list;
    const std::filesystem::path proc_dir{"/proc"};

    if (!std::filesystem::exists(proc_dir) || !std::filesystem::is_directory(proc_dir)) return list;

    for (const auto& entry : std::filesystem::directory_iterator(proc_dir)) 
    {
        if (entry.is_directory()) 
        {
            std::string filename = entry.path().filename().string();
            
            if (!filename.empty() && std::all_of(filename.begin(), filename.end(), ::isdigit)) 
            {
                pid_t pid = std::stoul(filename);
                ProcData proc = process_parse(pid);
                
                if (proc.stat.pid != 0) list.push_back(proc);
            }
        }
    }

    return list;
}

ProcData Parser::process_parse(pid_t pid) const
{
    ProcData proc;
    std::filesystem::path pid_dir = std::filesystem::path("/proc") / std::to_string(pid);

    proc.stat = stat_parse(pid_dir / "stat");
    if (proc.stat.pid == 0) return proc; 

    proc.statm = statm_parse(pid_dir / "statm");

    proc.command = cmd_parse(pid_dir / "cmdline");

    if (proc.command.empty()) proc.command = "[" + proc.stat.e_name + "]";

    return proc;
}

ProcStat Parser::stat_parse(std::filesystem::path path) const
{
    ProcStat stat{};
    stat.pid = 0;//флажок ошибки

    std::ifstream file(path);
    if (!file.is_open()) return stat;

    std::string line;
    if (std::getline(file, line)) 
    {
        size_t first_bracket = line.find('(');
        size_t last_bracket = line.rfind(')');
        
        if (first_bracket == std::string::npos || last_bracket == std::string::npos || first_bracket >= last_bracket) return stat;

        stat.pid = std::stoul(line.substr(0, first_bracket));

        stat.e_name = line.substr(first_bracket + 1, last_bracket - first_bracket - 1);

        std::istringstream ss(line.substr(last_bracket + 2));
        
        ss >> stat.state;//state

        std::string skip;
        for (int i = 0; i < 10; ++i) ss >> skip; 

        ss >> stat.user_j >> stat.system_j;
    }

    return stat;
}

ProcMem Parser::statm_parse(std::filesystem::path path) const
{
    ProcMem mem{0, 0};
    
    std::ifstream file(path);
    if (!file.is_open()) return mem;

    file >> mem.size >> mem.resident;
    
    return mem;
}

std::string Parser::cmd_parse(std::filesystem::path path) const
{
    std::string cmd;
    
    std::ifstream file(path);
    if (!file.is_open()) return cmd;

    if (std::getline(file, cmd)) 
    {
        for (char& ch : cmd) 
            if (ch == '\0') ch = ' ';
        
        if (!cmd.empty() && cmd.back() == ' ') cmd.pop_back();
    }

    return cmd;
}