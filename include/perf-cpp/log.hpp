//  SPDX-FileCopyrightText: 2024 Technische Universität Dresden
//  SPDX-License-Identifier: MIT

#pragma once

#include <nitro/log/log.hpp>

#include <nitro/log/sink/stderr_mt.hpp>

#include <nitro/log/attribute/message.hpp>
#include <nitro/log/attribute/pid.hpp>
#include <nitro/log/attribute/severity.hpp>
#include <nitro/log/attribute/timestamp.hpp>

#include <nitro/log/filter/and_filter.hpp>
#include <nitro/log/filter/mpi_master_filter.hpp>
#include <nitro/log/filter/severity_filter.hpp>

#include <fstream>
#include <mutex>

namespace perf_cpp
{
namespace logging
{

using Record =
    nitro::log::record<nitro::log::message_attribute,
                       nitro::log::timestamp_clock_attribute<std::chrono::high_resolution_clock>,
                       nitro::log::severity_attribute, nitro::log::pid_attribute>;

template <typename R>
class PerfCppLogFormatter
{
public:
    std::string format(R& r)
    {
        std::stringstream s;

        s << "[" << r.timestamp().time_since_epoch().count() << "][pid: " << r.pid()
          << "][tid: " << r.tid() << "][" << r.severity() << "]: " << r.message() << '\n';

        return s.str();
    }
};

template <typename R>
using PerfCppFilter = nitro::log::filter::severity_filter<R>;

using Logging = nitro::log::logger<Record, PerfCppLogFormatter, nitro::log::sink::StdErrThreaded,
                                   PerfCppFilter>;

inline void set_min_severity_level(nitro::log::severity_level sev)
{
    PerfCppFilter<Record>::set_severity(sev);
}

inline nitro::log::severity_level get_min_severity_level()
{
    return PerfCppFilter<Record>::min_severity();
}

inline void set_min_severity_level(std::string verbosity)
{
    set_min_severity_level(
        nitro::log::severity_from_string(verbosity, nitro::log::severity_level::info));
}

} // namespace logging

using Log = logging::Logging;
} // namespace perf_cpp
