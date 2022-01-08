cmake_minimum_required (VERSION 3.12)

project ("tests")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

execute_process(COMMAND echo "Run conan install...")
execute_process(COMMAND conan install . --build missing WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

include(${PROJECT_SOURCE_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(BENCH_SOURCES
	bench.cpp
	builder.h
	builder.cpp
	)

add_executable("bench" ${BENCH_SOURCES})
target_link_libraries("bench" PUBLIC ${CONAN_LIBS})

enable_testing()

set(TEST_SOURCES
	tests.cpp
	builder.h
	builder.cpp
	)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})