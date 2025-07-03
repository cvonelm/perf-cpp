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

#include <stdexcept>
#include <vector>

#include <cstddef>

#include <filesystem>

namespace perf_cpp
{

namespace tracepoint
{
class EventField
{
public:
    EventField()
    {
    }

    EventField(const std::string& name, std::ptrdiff_t offset, std::size_t size)
    : name_(name), offset_(offset), size_(size)
    {
    }

    const std::string& name() const
    {
        return name_;
    }

    std::ptrdiff_t offset() const
    {
        return offset_;
    }

    std::size_t size() const
    {
        return size_;
    }

    bool is_integer() const
    {
        // Parsing the type name is hard... really you don't want to do that
        switch (size())
        {
        case 1:
        case 2:
        case 4:
        case 8:
            return true;
        default:
            return false;
        }
    }

    bool valid() const
    {
        return size_ > 0;
    }

private:
    std::string name_;
    std::ptrdiff_t offset_;
    std::size_t size_ = 0;
};
} // namespace tracepoint
} // namespace perf_cpp
