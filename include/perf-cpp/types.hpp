//  SPDX-FileCopyrightText: 2024 Technische Universität Dresden
//  SPDX-License-Identifier: MIT

#pragma once

extern "C"
{
#include <sys/types.h>
}

#include <cstdint>
#include <string>
#include <vector>

namespace perf_cpp
{
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

    static Thread invalid()
    {
        return Thread(-1);
    }

    friend std::ostream& operator<<(std::ostream& stream, const Thread& thread)
    {
        return stream << ("thread " + std::to_string(thread.as_pid_t()));
    }

    pid_t as_pid_t() const
    {
        return tid_;
    }

private:
    pid_t tid_;
};

class Cpu
{
public:
    explicit Cpu(int cpuid) : cpu_(cpuid)
    {
    }

    int as_int() const
    {
        return cpu_;
    }

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
        return stream << std::to_string(cpu.as_int());
    }

private:
    int cpu_;
};

class Core
{
public:
    Core(int core_id, int package_id) : core_id_(core_id), package_id_(package_id)
    {
    }

    static Core invalid()
    {
        return Core(-1, -1);
    }

    friend bool operator==(const Core& lhs, const Core& rhs)
    {
        return (lhs.core_id_ == rhs.core_id_) && (lhs.package_id_ == rhs.package_id_);
    }

    friend bool operator<(const Core& lhs, const Core& rhs)
    {
        if (lhs.package_id_ == rhs.package_id_)
        {
            return lhs.core_id_ < rhs.core_id_;
        }
        return lhs.package_id_ < rhs.package_id_;
    }

    int core_as_int() const
    {
        return core_id_;
    }

    int package_as_int() const
    {
        return package_id_;
    }

private:
    int core_id_;
    int package_id_;
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

} // namespace perf_cpp
