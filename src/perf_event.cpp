//  SPDX-FileCopyrightText: 2024 Technische Universität Dresden
//  SPDX-License-Identifier: MIT

#include <perf-cpp/perf_event.hpp>
#include <perf-cpp/util.hpp>
#include <perf-cpp/perf_event_instance.hpp>

namespace perf_cpp
{

PerfEvent::PerfEvent(uint64_t type, uint64_t config, uint64_t config1)
{
    memset(&attr_, 0, sizeof(attr_));
    attr_.size = sizeof(attr_);
    attr_.type = type;
    attr_.config = config;
    attr_.config1 = config1;
}

PerfEvent::PerfEvent()
{
    memset(&attr_, 0, sizeof(attr_));
    attr_.size = sizeof(attr_);
    attr_.type = -1;
}

PerfEventInstance PerfEvent::open(std::variant<Cpu, Thread> location)
{
    return PerfEventInstance(*this, location);
}

bool PerfEvent::is_supported_in(std::variant<Cpu, Thread> location) const
{
    return std::visit(
        overloaded{ [&](Cpu cpu) -> bool {
                       return std::visit(
                           overloaded{ [](AnyLocation loc) { return true; },
                                       [](AnyCpu cpu) { return true; },
                                       [cpu](std::set<Cpu> cpus) { return cpus.count(cpu) != 0; },
                                       [](auto arg) { return false; } },
                           availability_);
                   },
                    [&](Thread thread) -> bool {
                        return std::visit(overloaded{
                                              [](AnyLocation loc) { return true; },
                                              [](auto arg) { return false; },
                                          },
                                          availability_);
                    } },
        location);
}

} // namespace perf_cpp
