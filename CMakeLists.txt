cmake_minimum_required(VERSION 3.11)
project(perf-cpp VERSION 0.1.0)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

include(FeatureSummary)
include(CMakeDependentOption)

include(cmake/CheckNameExists.cmake)

add_subdirectory(lib/fmt)

set(LIB_SRCS 
    src/event_attr.cpp
    src/event_resolver.cpp
    src/util.cpp
    src/topology.cpp
    src/types.cpp
    src/topology.cpp)

add_library(perf-cpp SHARED ${LIB_SRCS})

target_link_libraries(perf-cpp PRIVATE fmt::fmt-header-only)

target_include_directories(perf-cpp
	PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/include
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# check if specific perf events are available
CHECK_NAME_EXISTS(PERF_COUNT_SW_ALIGNMENT_FAULTS linux/perf_event.h HAVE_PERF_EVENT_ALIGNMENT_FAULTS)
CHECK_NAME_EXISTS(PERF_COUNT_SW_EMULATION_FAULTS linux/perf_event.h HAVE_PERF_EVENT_EMULATION_FAULTS)
CHECK_NAME_EXISTS(PERF_COUNT_HW_REF_CPU_CYCLES linux/perf_event.h HAVE_PERF_EVENT_REF_CYCLES)
CHECK_NAME_EXISTS(PERF_COUNT_HW_STALLED_CYCLES_FRONTEND linux/perf_event.h HAVE_PERF_EVENT_STALLED_CYCLES_FRONTEND)
CHECK_NAME_EXISTS(PERF_COUNT_HW_STALLED_CYCLES_BACKEND linux/perf_event.h HAVE_PERF_EVENT_STALLED_CYCLES_BACKEND)
CHECK_NAME_EXISTS(PERF_COUNT_HW_CACHE_NODE linux/perf_event.h HAVE_PERF_EVENT_CACHE_NODE)
CHECK_NAME_EXISTS(PERF_COUNT_SW_DUMMY linux/perf_event.h HAVE_PERF_EVENT_DUMMY)
CHECK_NAME_EXISTS(PERF_COUNT_SW_BPF_OUTPUT linux/perf_event.h HAVE_PERF_EVENT_BPF_OUTPUT)
CHECK_NAME_EXISTS(PERF_COUNT_SW_CGROUP_SWITCHES linux/perf_event.h HAVE_PERF_EVENT_CGROUP_SWITCHES)

configure_file(include/perf-cpp/build_config.hpp.in include/perf-cpp/build_config.hpp)

set_property(TARGET perf-cpp PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	add_executable(test examples/test.cpp)
	target_link_libraries(test PRIVATE perf-cpp fmt::fmt-header-only)
endif()

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/perf-cpp
    DESTINATION include)

install(TARGETS perf-cpp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
