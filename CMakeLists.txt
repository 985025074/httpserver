CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
project(fortest)

# src
set(SRC_CPP)
set(CXX_STANDARDS 14)
# includes
include_directories(${PROJECT_SOURCE_DIR}/include)

# refresh src
aux_source_directory(${PROJECT_SOURCE_DIR}/src TEMP)
list(APPEND SRC_CPP ${TEMP})

#set target
add_executable(httpserver ${SRC_CPP})

# using C++ 20
target_compile_features(httpserver PUBLIC cxx_std_20)
