#include <perf-cpp/topology.hpp>
#include <perf-cpp/util.hpp>

namespace perf_cpp
{

// TODO: _real_ topology
std::set<Cpu> get_cpus()
{
    std::set<Cpu> cpus;
    const std::filesystem::path base_path = "/sys/devices/system/cpu";
    auto online = parse_list_from_file(base_path / "online");
    std::transform(online.begin(), online.end(), std::inserter(cpus, cpus.end()),
                   [](uint32_t cpuid) { return Cpu(cpuid); });
    return cpus;
}
} // namespace perf_cpp
