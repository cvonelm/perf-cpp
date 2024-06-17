#include <perf-cpp/perf_event.hpp>
#include <perf-cpp/types.hpp>

#include <utility>
#include <variant>

extern "C"
{
#include <unistd.h>
}

namespace perf_cpp
{
struct UserspaceReadFormat
{
    UserspaceReadFormat() : value(0), time_enabled(0), time_running(0)
    {
    }

    uint64_t value;
    uint64_t time_enabled;
    uint64_t time_running;
};

class PerfEventInstance
{
public:
    PerfEventInstance(PerfEvent ev, std::variant<Cpu, Thread> location);
    // we don't want copies flying around because of the fd
    PerfEventInstance(PerfEventInstance&) = delete;
    PerfEventInstance& operator=(const PerfEventInstance&) = delete;

    PerfEventInstance(PerfEventInstance&& other)
    {
        std::swap(fd_, other.fd_);
        std::swap(ev_, other.ev_);
    }

    PerfEventInstance& operator=(PerfEventInstance&& other)
    {
        std::swap(fd_, other.fd_);
        std::swap(ev_, other.ev_);
        return *this;
    }

    template <class T>
    T read()
    {
        uint64_t val;
        if (::read(fd_, &val, sizeof(val)) != sizeof(uint64_t))
        {
            throw std::system_error(errno, std::system_category());
        }

        return (static_cast<T>(val)) * ev_.get_scale();
    }

    ~PerfEventInstance()
    {
        close(fd_);
    }

private:
    int fd_;
    PerfEvent ev_;
};
} // namespace perf_cpp
