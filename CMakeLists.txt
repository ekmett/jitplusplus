cmake_minimum_required(VERSION 3.30)
project(jitpp VERSION 0.1 LANGUAGES CXX ASM)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
   NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    message(FATAL_ERROR "jitpp currently requires Linux x86-64.")
endif()

find_package(Threads REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_path(UDIS86_INCLUDE_DIR udis86.h REQUIRED)
find_library(UDIS86_LIBRARY udis86 REQUIRED)

add_library(jitpp STATIC
    jit++/interpreting/tracer_start.S
)
add_library(jitpp::jitpp ALIAS jitpp)
target_compile_features(jitpp PUBLIC cxx_std_26)
set_target_properties(jitpp PROPERTIES CXX_EXTENSIONS OFF CXX_SCAN_FOR_MODULES ON)
target_sources(jitpp PUBLIC FILE_SET CXX_MODULES FILES
    jitpp.ccm
    jit++/application.ccm
    jit++/interpreter.ccm
)
target_sources(jitpp PRIVATE FILE_SET internal_modules TYPE CXX_MODULES FILES
    jit++/memory.ccm
)
target_include_directories(jitpp PRIVATE
    "${UDIS86_INCLUDE_DIR}"
)
target_link_libraries(jitpp PRIVATE glog::glog ${GFLAGS_TARGET} Threads::Threads
    "${UDIS86_LIBRARY}" ${CMAKE_DL_LIBS})

option(JITPP_BUILD_TESTS "Build the interpreter tests" "${PROJECT_IS_TOP_LEVEL}")
if(JITPP_BUILD_TESTS)
    enable_testing()
    add_executable(test_roundtrip test/t_roundtrip.cc test/roundtrip.S)
    target_link_libraries(test_roundtrip PRIVATE jitpp::jitpp ${GFLAGS_TARGET})
    add_executable(test_interpreter test/t_interpreter.cc)
    target_link_libraries(test_interpreter PRIVATE jitpp::jitpp glog::glog)
    add_executable(test_decoder test/t_decoder.cc)
    target_include_directories(test_decoder PRIVATE "${UDIS86_INCLUDE_DIR}")
    target_link_libraries(test_decoder PRIVATE jitpp::jitpp "${UDIS86_LIBRARY}")
    add_executable(bench_interpreter EXCLUDE_FROM_ALL test/bench_interpreter.cc)
    target_link_libraries(bench_interpreter PRIVATE jitpp::jitpp)
    foreach(target IN ITEMS test_roundtrip test_interpreter test_decoder bench_interpreter)
        set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    endforeach()
    add_test(NAME roundtrip COMMAND test_roundtrip)
    add_test(NAME interpreter COMMAND test_interpreter)
    add_test(NAME decoder COMMAND test_decoder)
endif()
