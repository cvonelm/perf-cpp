// SPDX-FileCopyrightText: 2024 Technische Universität Dresden
// SPDX-License-Identifier: MIT

#pragma once

#include <stdexcept>
#include <string>

#include <perf-cpp/perf_event.hpp>

namespace perf_cpp
{

class EventProvider
{
public:
    EventProvider();
    EventProvider(const EventProvider&) = delete;
    void operator=(const EventProvider&) = delete;

    static const EventProvider& instance()
    {
        return instance_mutable();
    }

    static PerfEvent get_event_by_name(const std::string& name);

    static std::map<std::string, PerfEvent> get_predefined_events();
    static std::map<std::string, PerfEvent> get_pmu_events();

    class InvalidEvent : public std::runtime_error
    {
    public:
        InvalidEvent(const std::string& event_description)
        : std::runtime_error(std::string{ "Invalid event: " } + event_description)
        {
        }
    };

private:
    static EventProvider& instance_mutable()
    {
        static EventProvider e;
        return e;
    }

    PerfEvent cache_event(const std::string& name);

    std::map<std::string, PerfEvent> event_map_;
};
} // namespace perf_cpp
