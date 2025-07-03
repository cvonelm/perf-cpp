/*
 * This file is part of the perf_cpp library.
 * Linux Perf C++ bindings
 *
 * Copyright (c) 2024,
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

#include <perf-cpp/event_attr.hpp>

namespace perf_cpp
{

namespace tracepoint
{

/**
 * Contains an event that is addressable via name
 */
class TracepointEventAttr : public EventAttr
{
public:
    class ParseError : public std::runtime_error
    {
    public:
        ParseError(const std::string& what) : std::runtime_error(what)
        {
        }

        ParseError(const std::string& what, int error_code);
    };

    TracepointEventAttr(const std::string& name);

    void parse_format();

    const auto& fields() const
    {
        return fields_;
    }

    const auto& field(const std::string& name) const
    {
        for (const auto& field : fields())
        {
            if (field.name() == name)
            {
                return field;
            }
        }
        throw std::out_of_range("field not found");
    }

    int id()
    {
        return id_;
    }

private:
    void parse_format_line(const std::string& line);

    const static std::filesystem::path base_path_;
    int id_;
    std::vector<tracepoint::EventField> fields_;
};

} // namespace tracepoint
} // namespace perf_cpp
