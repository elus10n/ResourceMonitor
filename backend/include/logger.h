#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

#include "modules.h"

namespace Logger
{
    inline logCallback get_callback(std::ostream& stream)
    {
        auto callback = [&stream](const std::string &log_message)
        {
            stream << log_message << std::endl;
        };

        return callback;
    }
}

#endif