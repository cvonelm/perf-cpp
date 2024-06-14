#include <perf-cpp/perf_event.hpp>
#include <perf-cpp/util.hpp>
#include <perf-cpp/perf_event_instance.hpp>

namespace perf_cpp
{
static int perf_event_open(struct perf_event_attr perf_attr, std::variant<Cpu, Thread> location,
                           int group_fd , unsigned long flags )
{
    int cpuid = -1;
    pid_t pid = -1;

    std::visit(overloaded{ [&](Cpu cpu) { cpuid = cpu.as_int(); },
                           [&](Thread thread) { pid = thread.as_pid_t(); } },
               location);

    return syscall(__NR_perf_event_open, &perf_attr, pid, cpuid, group_fd, flags);
}
PerfEventInstance::PerfEventInstance(PerfEvent ev, std::variant<Cpu, Thread> location)
{
	fd_ = perf_event_open(ev.get_attr(), location, -1, 0);
	if(fd_ == -1)
	{
throw std::system_error(errno, std::system_category());
	}
	
}

uint64_t PerfEventInstance::read()
{
	uint64_t val;
	if(::read(fd_, &val, sizeof(val)) != sizeof(uint64_t))
	{
		throw std::system_error(errno, std::system_category());
	}

	return val;
}
} // namespace perf_cpp
