cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(catch2_builder CXX)
include(ExternalProject)

ExternalProject_Add(Catch2
  PREFIX ${CMAKE_BINARY_DIR}/catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG "v2.13.6"
  TIMEOUT 10
  # UPDATE_COMMAND ${GIT_EXECUTABLE} pull
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(Catch2 source_dir)
#set(CATCH_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "Catch2 source dir")
set(CATCH_SINGLE_INCLUDE_DIR ${source_dir}/single_include CACHE INTERNAL "Catch2 single include dir")
