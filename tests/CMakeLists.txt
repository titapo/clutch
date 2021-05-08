
set(TESTING_INCLUDES ${CATCH_SINGLE_INCLUDE_DIR})

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
  # target_link_libraries()
endfunction(add_clutch_test)

message(STATUS ${CATCH_INCLUDE_DIR})
message(STATUS ${TESTING_INLUCES})

add_clutch_test(type_erased)
