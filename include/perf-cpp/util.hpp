#pragma once

#include <perf-cpp/types.hpp>

#include <set>
#include <variant>

extern "C"
{
#include <linux/perf_event.h>
}

namespace perf_cpp
{
std::set<std::uint32_t> parse_list(std::string list);
std::set<std::uint32_t> parse_list_from_file(std::filesystem::path file);

int perf_event_paranoid();
int perf_event_open(struct perf_event_attr* perf_attr, std::variant<Cpu, Thread> location,
                    int group_fd, unsigned long flags, int cgroup_fd = -1);

} // namespace perf_cpp
