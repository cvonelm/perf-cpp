#include <perf-cpp/error.hpp>
#include <perf-cpp/event_attr.hpp>
#include <perf-cpp/topology.hpp>
#include <perf-cpp/util.hpp>

#include <fstream>

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

template <typename T>
T get_sysctl(const std::string& group, const std::string& name)
{
    auto sysctl_path = std::filesystem::path("/proc/sys") / group / name;
    std::ifstream sysctl_stream;
    sysctl_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    sysctl_stream.open(sysctl_path);

    T result;
    sysctl_stream >> result;
    return result;
}

int perf_event_paranoid()
{
    return get_sysctl<int>("kernel", "perf_event_paranoid");
}

int perf_event_open(struct perf_event_attr* perf_attr, std::variant<Cpu, Thread> location,
                    int group_fd, unsigned long flags, int cgroup_fd)
{
    int cpuid = -1;
    pid_t pid = -1;
    if (std::holds_alternative<Cpu>(location))
    {
        if (cgroup_fd != -1)
        {
            pid = cgroup_fd;
            flags |= PERF_FLAG_PID_CGROUP;
        }
        cpuid = std::get<Cpu>(location).as_int();
    }
    else
    {
        pid = std::get<Thread>(location).as_pid_t();
    }
    return syscall(__NR_perf_event_open, perf_attr, pid, cpuid, group_fd, flags);
}

} // namespace perf_cpp
