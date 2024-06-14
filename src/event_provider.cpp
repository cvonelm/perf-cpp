//  SPDX-FileCopyrightText: 2024 Technische Universität Dresden
//  SPDX-License-Identifier: MIT

#include <perf-cpp/event_provider.hpp>
#include <perf-cpp/log.hpp>
#include <perf-cpp/perf_event.hpp>
#include <perf-cpp/sysfs_event.hpp>
#include <perf-cpp/util.hpp>

#include <filesystem>
#include <fstream>
#include <ios>
#include <limits>
#include <regex>
#include <set>
#include <sstream>
#include <vector>

#include <cstring>

extern "C"
{
#include <linux/perf_event.h>
#include <linux/version.h>
#include <unistd.h>
}

namespace perf_cpp
{

#define PERF_EVENT(type, id)                                                                       \
    {                                                                                              \
        (type), (id), 0                                                                            \
    }
#define PERF_EVENT_HW(id) PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_##id)
#define PERF_EVENT_SW(id) PERF_EVENT(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_##id)

static std::map<std::string, PerfEvent> HW_EVENT_TABLE{
    { "cpu-cycles", PERF_EVENT_HW(CPU_CYCLES) },
    { "instructions", PERF_EVENT_HW(INSTRUCTIONS) },
    { "cache-references", PERF_EVENT_HW(CACHE_REFERENCES) },
    { "cache-misses", PERF_EVENT_HW(CACHE_MISSES) },
    { "branch-instructions", PERF_EVENT_HW(BRANCH_INSTRUCTIONS) },
    { "branch-misses", PERF_EVENT_HW(BRANCH_MISSES) },
    { "bus-cycles", PERF_EVENT_HW(BUS_CYCLES) },
#ifdef HAVE_PERF_EVENT_STALLED_CYCLES_FRONTEND
    { "stalled-cycles-frontend", PERF_EVENT_HW(STALLED_CYCLES_FRONTEND) },
#endif
#ifdef HAVE_PERF_EVENT_STALLED_CYCLES_BACKEND
    { "stalled-cycles-backend", PERF_EVENT_HW(STALLED_CYCLES_BACKEND) },
#endif
#ifdef HAVE_PERF_EVENT_REF_CYCLES
    { "ref-cycles", PERF_EVENT_HW(REF_CPU_CYCLES) },
#endif
};

static std::map<std::string, PerfEvent> SW_EVENT_TABLE{
    { "cpu-clock", PERF_EVENT_SW(CPU_CLOCK) },
    { "task-clock", PERF_EVENT_SW(TASK_CLOCK) },
    { "page-faults", PERF_EVENT_SW(PAGE_FAULTS) },
    { "context-switches", PERF_EVENT_SW(CONTEXT_SWITCHES) },
    { "cpu-migrations", PERF_EVENT_SW(CPU_MIGRATIONS) },
    { "minor-faults", PERF_EVENT_SW(PAGE_FAULTS_MIN) },
    { "major-faults", PERF_EVENT_SW(PAGE_FAULTS_MAJ) },
#ifdef HAVE_PERF_EVENT_ALIGNMENT_FAULTS
    { "alignment-faults", PERF_EVENT_SW(ALIGNMENT_FAULTS) },
#endif
#ifdef HAVE_PERF_EVENT_EMULATION_FAULTS
    { "emulation-faults", PERF_EVENT_SW(EMULATION_FAULTS) },
#endif
};

template <typename T>
struct string_to_id
{
    const char* name;
    T id;
};

static string_to_id<perf_hw_cache_id> CACHE_NAME_TABLE[] = {
    { "L1-dcache", PERF_COUNT_HW_CACHE_L1D }, { "L1-icache", PERF_COUNT_HW_CACHE_L1I },
    { "LLC", PERF_COUNT_HW_CACHE_LL },        { "dTLB", PERF_COUNT_HW_CACHE_DTLB },
    { "iTLB", PERF_COUNT_HW_CACHE_ITLB },     { "branch", PERF_COUNT_HW_CACHE_BPU },
#ifdef HAVE_PERF_EVENT_CACHE_NODE
    { "node", PERF_COUNT_HW_CACHE_NODE },
#endif
};

struct cache_op_and_result
{
    perf_hw_cache_op_id op_id;
    perf_hw_cache_op_result_id result_id;
};

static string_to_id<cache_op_and_result> CACHE_OPERATION_TABLE[] = {
    { "loads", { PERF_COUNT_HW_CACHE_OP_READ, PERF_COUNT_HW_CACHE_RESULT_ACCESS } },
    { "stores", { PERF_COUNT_HW_CACHE_OP_WRITE, PERF_COUNT_HW_CACHE_RESULT_ACCESS } },
    { "prefetches", { PERF_COUNT_HW_CACHE_OP_PREFETCH, PERF_COUNT_HW_CACHE_RESULT_ACCESS } },
    { "load-misses", { PERF_COUNT_HW_CACHE_OP_READ, PERF_COUNT_HW_CACHE_RESULT_MISS } },
    { "store-misses", { PERF_COUNT_HW_CACHE_OP_WRITE, PERF_COUNT_HW_CACHE_RESULT_MISS } },
    { "prefetch-misses", { PERF_COUNT_HW_CACHE_OP_PREFETCH, PERF_COUNT_HW_CACHE_RESULT_MISS } },
};

inline constexpr std::uint64_t make_cache_config(perf_hw_cache_id cache, perf_hw_cache_op_id op,
                                                 perf_hw_cache_op_result_id op_result)
{
    return cache | (op << 8) | (op_result << 16);
} // END

std::map<std::string, PerfEvent> EventProvider::get_pmu_events()
{
    std::map<std::string, PerfEvent> events;

    const std::filesystem::path pmu_devices("/sys/bus/event_source/devices");

    for (const auto& pmu : std::filesystem::directory_iterator(pmu_devices))
    {
        const auto pmu_path = pmu.path();

        const std::filesystem::path event_dir(pmu_path / "events");

        // some PMUs don't have any events, in that case event_dir doesn't exist
        if (!std::filesystem::is_directory(event_dir))
        {
            continue;
        }

        for (const auto& event : std::filesystem::directory_iterator(event_dir))
        {
            std::stringstream event_name;

            const auto event_path = event.path();
            const auto extension = event_path.extension();

            // ignore scaling and unit information
            if (extension == ".scale" || extension == ".unit")
            {
                continue;
            }

            // use std::filesystem::path::string, otherwise the paths are formatted
            // quoted
            event_name << pmu_path.filename().string() << '/' << event_path.filename().string()
                       << '/';

            events.emplace(event_name.str(), SysfsPerfEvent(pmu_path.filename().string(),
                                                            event_path.filename().string()));
        }
    }

    return events;
}

EventProvider::EventProvider()
{
    Log::info() << "checking available events...";

    event_map_.insert(HW_EVENT_TABLE.begin(), HW_EVENT_TABLE.end());
    event_map_.insert(SW_EVENT_TABLE.begin(), SW_EVENT_TABLE.end());

    std::stringstream name_fmt;
    for (auto& cache : CACHE_NAME_TABLE)
    {
        for (auto& operation : CACHE_OPERATION_TABLE)
        {
            name_fmt.str(std::string());
            name_fmt << cache.name << '-' << operation.name;

            PerfEvent ev(PERF_TYPE_HW_CACHE,
                         make_cache_config(cache.id, operation.id.op_id, operation.id.result_id),
                         0);

            event_map_.emplace(name_fmt.str(), ev);
        }
    }
}

PerfEvent EventProvider::cache_event(const std::string& name)
{
    // Format for raw events is r followed by a hexadecimal number
    static const std::regex raw_regex("r([[:xdigit:]]{1,8})");

    // save event in event map; return a reference to the inserted event to
    // the caller.
    std::smatch raw_match;
    if (regex_match(name, raw_match, raw_regex))
    {
        return event_map_.emplace(name, PerfEvent(PERF_TYPE_RAW, std::stoull(raw_match[1]), 0))
            .first->second;
    }
    else
    {
        /* Event description format:
         *   Name of a Performance Monitoring Unit (PMU) and an event name,
         *   separated by either '/' or ':' (for perf-like syntax);  followed by an
         *   optional separator:
         *
         *     <pmu>/<event_name>[/]
         *   OR
         *     <pmu>:<event_name>[/]
         *
         *   Examples (both specify the same event):
         *
         *     cpu/cache-misses/
         *     cpu:cache-misses
         *
         * */

        enum EVENT_DESCRIPTION_REGEX_GROUPS
        {
            ED_WHOLE_MATCH,
            ED_PMU,
            ED_NAME,
        };

        static const std::regex ev_desc_regex(R"(([a-z0-9-_]+)[\/:]([a-z0-9-_]+)\/?)");
        std::smatch ev_desc_match;

        if (!std::regex_match(name, ev_desc_match, ev_desc_regex))
        {
            return event_map_.emplace(name, PerfEvent()).first->second;
        }

        const std::string& pmu_name = ev_desc_match[ED_PMU];
        const std::string& event_name = ev_desc_match[ED_NAME];
        return event_map_.emplace(name, SysfsPerfEvent(pmu_name, event_name)).first->second;
    }
}

PerfEvent EventProvider::get_event_by_name(const std::string& name)
{
    auto& ev_map = instance().event_map_;
    auto event_it = ev_map.find(name);
    if (event_it != ev_map.end())
    {
        return event_it->second;
    }
    else
    {
        return instance_mutable().cache_event(name);
    }
}

std::map<std::string, PerfEvent> EventProvider::get_predefined_events()
{

    const auto& ev_map = instance().event_map_;

    std::map<std::string, PerfEvent> events;

    for (const auto& event : ev_map)
    {
        if (event.second.is_valid())
        {
            events.emplace(event);
        }
    }

    return events;
}
} // namespace perf_cpp
