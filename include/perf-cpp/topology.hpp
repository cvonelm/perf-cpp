#pragma once
#include <perf-cpp/types.hpp>

#include <algorithm>
#include <filesystem>
#include <set>

namespace perf_cpp
{
// TODO: _real_ topology
std::set<Cpu> get_cpus();

} // namespace perf_cpp
