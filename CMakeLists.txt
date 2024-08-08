cmake_minimum_required(VERSION 3.11)
project(perf-cpp VERSION 0.0.1)


include(FeatureSummary)
include(CMakeDependentOption)

add_subdirectory(lib/nitro)
add_subdirectory(lib/fmt)

if(NOT PERF_CPP_POSITION_INDEPENDENT_CODE)
	set(PERF_CPP_POSITION_INDEPENDENT_CODE OFF)
endif()

option(PERF_CPP_POSITION_INDEPENDENT_CODE "Whether to build perf-cpp with position independent code" ${PERF_CPP_POSITION_INDEPENDENT_CODE})


set(LIB_SRCS 
    src/event_provider.cpp
    src/perf_event.cpp
    src/perf_event_instance.cpp
    src/util.cpp
    src/topology.cpp)

add_library(perf-cpp ${LIB_SRCS})


target_link_libraries(perf-cpp
	PUBLIC
	Nitro::env
	Nitro::log
	fmt::fmt)
target_include_directories(perf-cpp
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set_target_properties(perf-cpp PROPERTIES
	POSITION_INDEPENDENT_CODE ${PERF_CPP_POSITION_INDEPENDENT_CODE}
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	add_executable(test examples/test.cpp)
	target_link_libraries(test PRIVATE perf-cpp)
endif()
