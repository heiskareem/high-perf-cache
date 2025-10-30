cmake_minimum_required(VERSION 3.15)
project(HighPerfCache VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -pthread")

# Dependencies
find_package(Threads REQUIRED)
find_package(benchmark QUIET)
find_package(GTest QUIET)

# Prometheus C++ client (install via: git clone https://github.com/jupp0r/prometheus-cpp)
find_package(prometheus-cpp CONFIG QUIET)

include_directories(include)

# Library
add_library(cache_lib STATIC
    src/lru_cache.cpp
    src/lfu_cache.cpp
    src/arc_cache.cpp
    src/memory_allocator.cpp
    src/metrics.cpp
)
target_link_libraries(cache_lib PUBLIC Threads::Threads)
if(prometheus-cpp_FOUND)
    target_link_libraries(cache_lib PUBLIC prometheus-cpp::core)
    target_compile_definitions(cache_lib PUBLIC HAS_PROMETHEUS)
endif()

# Benchmarks
if(benchmark_FOUND)
  add_executable(cache_benchmark benchmark/cache_benchmark.cpp)
  target_link_libraries(cache_benchmark PRIVATE cache_lib benchmark::benchmark benchmark::benchmark_main)
endif()

# Tests
enable_testing()
if(GTest_FOUND)
  add_executable(cache_tests
      test/test_lru.cpp
      test/test_lfu.cpp
      test/test_arc.cpp
      test/test_concurrency.cpp
  )
  target_link_libraries(cache_tests PRIVATE cache_lib GTest::gtest GTest::gtest_main)
  add_test(NAME CacheTests COMMAND cache_tests)
endif()

# Examples
add_executable(example examples/example_usage.cpp)
target_link_libraries(example PRIVATE cache_lib)

add_executable(example_lru examples/example_lru.cpp)
target_link_libraries(example_lru PRIVATE cache_lib)

add_executable(example_lfu examples/example_lfu.cpp)
target_link_libraries(example_lfu PRIVATE cache_lib)

add_executable(example_arc examples/example_arc.cpp)
target_link_libraries(example_arc PRIVATE cache_lib)

add_executable(example_concurrency examples/example_concurrency.cpp)
target_link_libraries(example_concurrency PRIVATE cache_lib)

add_executable(example_mixed examples/example_mixed.cpp)
target_link_libraries(example_mixed PRIVATE cache_lib)

add_custom_target(examples ALL DEPENDS example example_lru example_lfu example_arc example_concurrency example_mixed)
