#include <perf-cpp/perf_event.hpp>
#include <perf-cpp/perf_event_instance.hpp>
#include <perf-cpp/util.hpp>

extern "C"
{
#include <sys/ioctl.h>
}

namespace perf_cpp
{
static int perf_event_open(struct perf_event_attr perf_attr, std::variant<Cpu, Thread> location,
                           int group_fd, unsigned long flags)
{
    int cpuid = -1;
    pid_t pid = -1;

    std::visit(overloaded{ [&](Cpu cpu) { cpuid = cpu.as_int(); },
                           [&](Thread thread) { pid = thread.as_pid_t(); } },
               location);

    return syscall(__NR_perf_event_open, &perf_attr, pid, cpuid, group_fd, flags);
}

PerfEventInstance::PerfEventInstance(PerfEvent ev, std::variant<Cpu, Thread> location) : ev_(ev)
{
    fd_ = perf_event_open(ev.get_attr(), location, -1, 0);
    if (fd_ == -1)
    {
        throw std::system_error(errno, std::system_category());
    }

    ioctl(fd_, PERF_EVENT_IOC_ENABLE, 0);
}

} // namespace perf_cpp
