#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace perf_cpp
{

std::set<std::uint32_t> parse_list(std::string list);
std::set<std::uint32_t> parse_list_from_file(std::filesystem::path file);

} // namespace perf_cpp
