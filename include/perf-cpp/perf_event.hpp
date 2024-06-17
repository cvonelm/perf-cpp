//  SPDX-FileCopyrightText: 2024 Technische Universität Dresden
//  SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <perf-cpp/error.hpp>
#include <perf-cpp/log.hpp>
#include <perf-cpp/types.hpp>
#include <regex>
#include <set>
#include <string>
#include <variant>

extern "C"
{
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
}

namespace perf_cpp
{

class AnyLocation
{
};

class AnyCpu
{
};

typedef std::variant<std::monostate, AnyLocation, AnyCpu, std::set<Cpu>> location_t;

class PerfEventInstance;

class PerfEvent
{
public:
    PerfEvent(uint64_t type, uint64_t config, uint64_t config1);
    PerfEvent();

    bool is_valid() const
    {
        return attr_.type != -1;
    }

    PerfEventInstance open(std::variant<Cpu, Thread> location);

    bool is_supported_in(std::variant<Cpu, Thread> location) const;

    friend bool operator==(const PerfEvent& lhs, const PerfEvent& rhs)
    {
        return !memcmp(&lhs.attr_, &rhs.attr_, sizeof(struct perf_event_attr));
    }

    friend bool operator<(const PerfEvent& lhs, const PerfEvent& rhs)
    {
        return memcmp(&lhs.attr_, &rhs.attr_, sizeof(struct perf_event_attr));
    }

    struct perf_event_attr get_attr() const
    {
        return attr_;
    }

    double get_scale() const
    {
        return scale_;
    }

protected:
    struct perf_event_attr attr_;
    location_t availability_;
    double scale_ = 1.0;
    std::string unit_ = "#";
};

} // namespace perf_cpp
