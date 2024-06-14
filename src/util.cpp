//  SPDX-FileCopyrightText: 2024 Technische Universität Dresden
//  SPDX-License-Identifier: MIT

#include <perf-cpp/error.hpp>
#include <perf-cpp/log.hpp>
#include <perf-cpp/types.hpp>
#include <perf-cpp/util.hpp>

#include <filesystem>
extern "C"
{
#include <linux/perf_event.h>
#include <syscall.h>
}

namespace perf_cpp
{
std::set<std::uint32_t> parse_list(std::string list)
{
    std::stringstream s;
    s << list;

    std::set<uint32_t> res;

    std::string part;
    while (std::getline(s, part, ','))
    {
        auto pos = part.find('-');
        if (pos != std::string::npos)
        {
            // is a range
            uint32_t from = std::stoi(part.substr(0, pos));
            uint32_t to = std::stoi(part.substr(pos + 1));

            for (auto i = from; i <= to; ++i)
                res.insert(i);
        }
        else
        {
            // single value
            res.insert(std::stoi(part));
        }
    }

    return res;
}

std::set<std::uint32_t> parse_list_from_file(std::filesystem::path file)
{
    std::ifstream list_stream(file);
    std::string list_string;
    list_stream >> list_string;

    if (list_stream)
    {
        return parse_list(list_string);
    }

    return std::set<std::uint32_t>();
}
}
