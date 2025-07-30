/*
 * This file is part of the perf_cpp library.
 * Linux Perf C++ bindings
 *
 * Copyright (c) 2016,
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

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <cstdint>

#include <filesystem>

#include <perf-cpp/error.hpp>
#include <perf-cpp/types.hpp>

#ifdef HAVE_VEOSINFO
extern "C"
{
#include <veosinfo/veosinfo.h>
}
#endif

using namespace std::literals::string_literals;

namespace perf_cpp
{
class Topology
{

private:
    void read_proc();

    Topology()
    {
        read_proc();
    }

public:
    static Topology& instance()
    {
        static Topology t;

        return t;
    }

    bool hypervised() const
    {
        return hypervised_;
    }

    const std::set<Cpu>& cpus() const
    {
        return cpus_;
    }

#ifdef HAVE_VEOSINFO
    const std::set<NecDevice> nec_devices() const
    {
        ve_nodeinfo nodeinfo;
        auto ret = ve_node_info(&nodeinfo);
        if (ret == -1)
        {
            throw_errno();
        }

        std::set<NecDevice> devices;
        for (int i = 0; i < nodeinfo.total_node_count; i++)
        {
            devices.emplace(NecDevice(i));
        }
        return devices;
    }
#endif

    Core core_of(Cpu cpu) const
    {
        return cpu_to_core_.at(cpu);
    }

    const std::set<Core>& cores() const
    {
        return cores_;
    }

    const std::set<Package>& packages() const
    {
        return packages_;
    }

    template <typename T>
    Package package_of(T t) const;

    Cpu measuring_cpu_for_core(Core core) const
    {
        auto core_it = std::find_if(cpu_to_core_.begin(), cpu_to_core_.end(),
                                    [core](auto elem) { return elem.second == core; });

        if (core_it != cpu_to_core_.end())
        {
            return core_it->first;
        }
        return Cpu::invalid();
    }
    Cpu measuring_cpu_for_package(Package package) const
    {
        auto package_it = std::find_if(cpu_to_package_.begin(), cpu_to_package_.end(),
                                       [package](auto elem) { return elem.second == package; });

        if (package_it != cpu_to_package_.end())
        {
            return package_it->first;
        }
        return Cpu::invalid();
    }

    Cpu measuring_core_for_cpu(Core core) const
    {
        auto core_it = std::find_if(cpu_to_core_.begin(), cpu_to_core_.end(),
                                    [core](auto elem) { return elem.second == core; });

        if (core_it != cpu_to_core_.end())
        {
            return core_it->first;
        }
        return Cpu::invalid();
    }

private:
    std::set<Cpu> cpus_;
    std::set<Core> cores_;
    std::set<Package> packages_;
    std::map<Cpu, Core> cpu_to_core_;
    std::map<Core, Package> core_to_package_;
    std::map<Cpu, Package> cpu_to_package_;

    bool hypervised_ = false;
};
} // namespace perf_cpp
