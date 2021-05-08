cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(Clutch
  VERSION 0.0.1
  LANGUAGES CXX
  DESCRIPTION "Simple and naive type erasure library"
  )

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)

add_library(clutch INTERFACE)
target_include_directories(clutch INTERFACE include)

target_compile_features(clutch
  INTERFACE cxx_std_17
  )

add_subdirectory(${EXTERNAL_DIR}/catch)
include_directories(${CATCH_INCLUDE_DIR})
enable_testing(true)

add_subdirectory(tests)
