cmake_minimum_required(VERSION 3.15)

project("tests")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BENCH_SOURCES bench.cpp builder.h builder.cpp)

# init for vs 2002
# conan install . -s build_type=Release --build=missing
# conan install . -s build_type=Debug --build=missing
# cmake -B build -S . -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake
# Open build\fit2srt.sln
# same for other IDEs

find_package(nlohmann_json REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

add_executable("bench" ${BENCH_SOURCES})
target_link_libraries("bench" PRIVATE rapidjson
                                      benchmark::benchmark_main
                                      nlohmann_json::nlohmann_json)

enable_testing()

set(TEST_SOURCES tests.cpp builder.h builder.cpp)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC rapidjson gtest::gtest)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
