set(TEST_TARGETS)
set(TEST_HRD "${XG_TIMER_PROJ_TOP}/include"
             "${XG_TIMER_PROJ_TOP}/include/logger"
             "${XG_TIMER_PROJ_TOP}/lib"
             )
set(TEST_LOG_SRC test_log.cc)
add_executable(test_log ${TEST_LOG_SRC})
target_include_directories(test_log PRIVATE ${TEST_HRD})
list(APPEND TEST_TARGETS test_log)

set(TEST_SCHEDULE_SRC test_schedule.cc)
add_executable(test_schedule ${TEST_SCHEDULE_SRC})
target_include_directories(test_schedule PRIVATE ${TEST_HRD})
target_link_directories(test_schedule PRIVATE "${CMAKE_BINARY_DIR}/lib")
target_link_libraries(test_schedule xgtimer)
list(APPEND TEST_TARGETS test_schedule)

add_custom_target(test)
add_dependencies(test ${TEST_TARGETS})
INSTALL(TARGETS ${TEST_TARGETS}
        RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/test"
       )
