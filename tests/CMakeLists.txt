
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.7
)

FetchContent_MakeAvailable(Catch2)

function(add_clutch_test name)
  include_directories(${TESTING_INCLUDES} ${COMMON_INCLUDES})
  add_executable(${name}_test ${name}.cc)
  target_link_libraries(${name}_test
    PRIVATE clutch
    )
  target_include_directories(${name}_test
    PRIVATE ${COMMON_INCLUDES}
    )
  add_test(${name}_test ${name}_test)
  target_link_libraries(${name}_test PRIVATE Catch2)
endfunction(add_clutch_test)

message(STATUS ${CATCH_INCLUDE_DIR})
message(STATUS ${TESTING_INLUCES})

add_clutch_test(type_erased)
