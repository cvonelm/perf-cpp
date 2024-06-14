//  SPDX-FileCopyrightText: 2024 Technische Universität Dresden
//  SPDX-License-Identifier: MIT

#pragma once

#include <system_error>

namespace perf_cpp
{
inline std::system_error make_system_error()
{
    return std::system_error(errno, std::system_category());
}

[[noreturn]] inline void throw_errno()
{
    throw make_system_error();
}

inline void check_errno(long retval)
{
    if (retval == -1)
    {
        throw_errno();
    }
}
} // namespace perf_cpp
