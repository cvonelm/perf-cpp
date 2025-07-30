/*
 * This file is part of the perf_cpp library.
 * Linux Perf C++ bindings
 *
 * Copyright (c) 2017,
 *    Technische Universitaet Dresden, Germany
 *
 * perf_cpp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * perf_cpp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with perf_cpp.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

extern "C"
{
#include <sys/types.h>
#include <unistd.h>
}

#include <fmt/format.h>

#include <vector>

#include <cstdint>

namespace perf_cpp
{

class Process;

class Thread
{
public:
    explicit Thread(pid_t tid) : tid_(tid)
    {
    }

    explicit Thread() : tid_(-1)
    {
    }

    friend bool operator==(const Thread& lhs, const Thread& rhs)
    {
        return lhs.tid_ == rhs.tid_;
    }

    friend bool operator!=(const Thread& lhs, const Thread& rhs)
    {
        return lhs.tid_ != rhs.tid_;
    }

    friend bool operator<(const Thread& lhs, const Thread& rhs)
    {
        return lhs.tid_ < rhs.tid_;
    }

    friend bool operator>(const Thread& lhs, const Thread& rhs)
    {
        return lhs.tid_ > rhs.tid_;
    }

    friend bool operator!(const Thread& thread)
    {
        return thread.tid_ == -1;
    }

    Process as_process() const;

    static Thread invalid()
    {
        return Thread(-1);
    }

    friend std::ostream& operator<<(std::ostream& stream, const Thread& thread)
    {
        return stream << fmt::format("{}", thread);
    }

    pid_t as_pid_t() const
    {
        return tid_;
    }

private:
    pid_t tid_;
};

class Process
{
public:
    explicit Process(pid_t pid) : pid_(pid)
    {
    }

    explicit Process() : pid_(-1)
    {
    }

    friend bool operator==(const Process& lhs, const Process& rhs)
    {
        return lhs.pid_ == rhs.pid_;
    }

    friend bool operator!=(const Process& lhs, const Process& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const Process& lhs, const Process& rhs)
    {
        return lhs.pid_ < rhs.pid_;
    }

    friend bool operator!(const Process& process)
    {
        return process.pid_ == -1;
    }

    static Process invalid()
    {
        return Process(-1);
    }

    static Process idle()
    {
        return Process(0);
    }

    static Process me()
    {
        return Process(getpid());
    }

    pid_t as_pid_t() const
    {
        return pid_;
    }

    Thread as_thread() const;

    friend std::ostream& operator<<(std::ostream& stream, const Process& process)
    {
        return stream << fmt::format("{}", process);
    }

private:
    pid_t pid_;
};

class Cpu
{
public:
    explicit Cpu(int cpuid) : cpu_(cpuid)
    {
    }

    int as_int() const;

    static Cpu invalid()
    {
        return Cpu(-1);
    }

    friend bool operator==(const Cpu& lhs, const Cpu& rhs)
    {
        return lhs.cpu_ == rhs.cpu_;
    }

    friend bool operator<(const Cpu& lhs, const Cpu& rhs)
    {
        return lhs.cpu_ < rhs.cpu_;
    }

    friend bool operator>(const Cpu& lhs, const Cpu& rhs)
    {
        return lhs.cpu_ > rhs.cpu_;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Cpu& cpu)
    {
        return stream << fmt::format("{}", cpu);
    }

private:
    int cpu_;
};

class Core
{
public:
    Core(int core_id) : core_id_(core_id)
    {
    }

    static Core invalid()
    {
        return Core(-1);
    }

    friend bool operator==(const Core& lhs, const Core& rhs)
    {
        return (lhs.core_id_ == rhs.core_id_);
    }

    friend bool operator<(const Core& lhs, const Core& rhs)
    {
        return lhs.core_id_ < rhs.core_id_;
    }

    int as_int() const
    {
        return core_id_;
    }

private:
    int core_id_;
};

class Package
{
public:
    explicit Package(int id) : id_(id)
    {
    }

    static Package invalid()
    {
        return Package(-1);
    }

    friend bool operator==(const Package& lhs, const Package& rhs)
    {
        return lhs.id_ == rhs.id_;
    }

    friend bool operator<(const Package& lhs, const Package& rhs)
    {
        return lhs.id_ < rhs.id_;
    }

    int as_int() const
    {
        return id_;
    }

private:
    int id_;
};

class NecDevice
{
public:
    explicit NecDevice(int id) : id_(id)
    {
    }

    int as_int() const
    {
        return id_;
    }

    friend bool operator==(const NecDevice& lhs, const NecDevice& rhs)
    {
        return lhs.id_ == rhs.id_;
    }

    friend bool operator<(const NecDevice& lhs, const NecDevice& rhs)
    {
        return lhs.id_ < rhs.id_;
    }

private:
    int id_;
};

} // namespace perf_cpp

namespace fmt
{
template <>
struct formatter<perf_cpp::Thread>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
        {
            throw format_error("invalid format");
        }

        return it;
    }

    template <typename FormatContext>
    auto format(const perf_cpp::Thread& thread, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "thread {}", thread.as_pid_t());
    }
};

template <>
struct formatter<perf_cpp::Process>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
        {
            throw format_error("invalid format");
        }

        return it;
    }

    template <typename FormatContext>
    auto format(const perf_cpp::Process& process, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "process {}", process.as_pid_t());
    }
};

template <>
struct formatter<perf_cpp::Cpu>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
        {
            throw format_error("invalid format");
        }

        return it;
    }

    template <typename FormatContext>
    auto format(const perf_cpp::Cpu& cpu, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "cpu {}", cpu.as_int());
    }
};

template <>
struct formatter<perf_cpp::NecDevice>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
        {
            throw format_error("invalid format");
        }

        return it;
    }

    template <typename FormatContext>
    auto format(const perf_cpp::NecDevice& device, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "VE {}", device.as_int());
    }
};
} // namespace fmt

namespace std
{
template <>
struct hash<perf_cpp::Thread>
{
    std::size_t operator()(const perf_cpp::Thread& t) const
    {
        return ((std::hash<pid_t>()(t.as_pid_t())));
    }
};

} // namespace std
