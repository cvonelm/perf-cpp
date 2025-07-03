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

#include <perf-cpp/tracepoint/format.hpp>
#include <perf-cpp/tracepoint/reader.hpp>

#include <perf-cpp/time/converter.hpp>

#include <perf-cpp/trace/trace.hpp>

#include <otf2xx/definition/metric_instance.hpp>
#include <otf2xx/event/metric.hpp>
#include <otf2xx/writer/local.hpp>

#include <vector>

namespace perf_cpp
{

{

    {
        // Note, this cannot be protected for CRTP reasons...
        class Writer : public Reader<Writer>
        {
        public:
            Writer(Cpu cpu, perf::tracepoint::TracepointEventAttr event, trace::Trace& trace,
                   const otf2::definition::metric_class& metric_class);

            Writer(const Writer& other) = delete;

            Writer(Writer&& other) = default;

        public:
            using Reader<Writer>::handle;

            bool handle(const Reader::RecordSampleType* sample);

        private:
            otf2::writer::local& writer_;
            otf2::definition::metric_instance metric_instance_;

            const time::Converter time_converter_;

            otf2::event::metric metric_event_;
        };
    } //
} //
} // namespace perf_cpp
