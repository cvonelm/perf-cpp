cmake_minimum_required(VERSION 3.11)
project(perf-cpp VERSION 0.0.1)


include(FeatureSummary)
include(CMakeDependentOption)

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

set_property(TARGET perf-cpp PROPERTY CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

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
