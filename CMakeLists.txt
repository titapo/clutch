cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(CLUTCH_IS_TOP_LEVEL ON)
else()
  set(CLUTCH_IS_TOP_LEVEL OFF)
endif ()


project(Clutch
  VERSION 0.0.1
  LANGUAGES CXX
  DESCRIPTION "Simple and naive type erasure library"
  )

option(CLUTCH_BUILD_TESTS "Build ezy tests" ON)


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)

add_library(clutch INTERFACE)
target_include_directories(clutch INTERFACE include)

target_compile_features(clutch
  INTERFACE cxx_std_11
  )

include_directories(${CATCH_INCLUDE_DIR})

if (CLUTCH_BUILD_TESTS AND CLUTCH_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(tests)
endif ()
