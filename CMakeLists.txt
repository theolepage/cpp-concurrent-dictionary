cmake_minimum_required (VERSION 3.11)
project (PRPA)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

find_package(Threads REQUIRED)
find_package(GTest 1.8.1 REQUIRED)
find_package(benchmark 1.5.0 REQUIRED)
find_package(gsl-lite 0.34.0 REQUIRED)
find_package(spdlog 1.6.0 REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native")

set(RESOURCES_DIR ${CMAKE_SOURCE_DIR})
configure_file(src/resources.cpp.in src/resources.cpp)


add_library(dictionary
  src/IAsyncDictionary.hpp
  src/IDictionary.hpp
  src/naive_async_dictionary.cpp
  src/naive_async_dictionary.hpp
  src/naive_dictionary.cpp
  src/naive_dictionary.hpp
  src/hashmap_dictionary.cpp
  src/hashmap_dictionary.hpp
  src/tools.cpp
  src/tools.hpp
  src/utils/hashmap.hpp
  ${CMAKE_CURRENT_BINARY_DIR}/src/resources.cpp
  )


target_link_libraries(dictionary PUBLIC gsl-lite::gsl-lite spdlog::spdlog)

add_executable(tests src/tests.cpp)
target_link_libraries(tests PRIVATE dictionary GTest::GTest)

add_executable(bench src/bench.cpp)
target_link_libraries(bench PRIVATE dictionary benchmark::benchmark)
