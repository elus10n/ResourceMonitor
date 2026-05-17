#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <filesystem>
#include <string>
#include <sys/types.h>

#include "modules.h"

class Parser
{
    CPUData cpu_parse() const;

    MemoryData mem_parse() const;

    std::vector<ProcData> processes_parse() const;

    ProcData process_parse(pid_t pid) const;

    public:
    ParserOutput parse() const;
};

#endif