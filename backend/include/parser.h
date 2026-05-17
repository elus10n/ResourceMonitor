#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>

#include "modules.h"

class Parser
{
    CPUData cpu_parse() const;

    MemoryData mem_parse() const;

    std::vector<ProcData> processes_parse() const;

    ProcData process_parse(pid_t pid) const;
    ProcStat stat_parse(std::filesystem::path path) const;
    ProcMem statm_parse(std::filesystem::path path) const;
    std::string cmd_parse(std::filesystem::path path) const;


    public:
    ParserOutput parse() const;
};

#endif