set(TEST_TARGETS)
set(TEST_HRD "${XG_TIMER_PROJ_TOP}/include"
             "${XG_TIMER_PROJ_TOP}/include/logger"
             "${XG_TIMER_PROJ_TOP}/lib"
             )
set(TEST_LOG_SRC test_log.cc)
add_executable(test_log ${TEST_LOG_SRC})
target_include_directories(test_log PRIVATE ${TEST_HRD})
list(APPEND TEST_TARGETS test_log)

set(TEST_SCHEDULE_DURATION_SRC test_schedule_duration.cc)
add_executable(test_schedule_duration ${TEST_SCHEDULE_DURATION_SRC})
target_include_directories(test_schedule_duration PRIVATE ${TEST_HRD})
target_link_directories(test_schedule_duration PRIVATE "${CMAKE_BINARY_DIR}/lib")
target_link_libraries(test_schedule_duration xgtimer)
list(APPEND TEST_TARGETS test_schedule_duration)

set(TEST_SCHEDULE_CRONTAB_SRC test_schedule_crontab.cc)
add_executable(test_schedule_crontab ${TEST_SCHEDULE_CRONTAB_SRC})
target_include_directories(test_schedule_crontab PRIVATE ${TEST_HRD})
target_link_directories(test_schedule_crontab PRIVATE "${CMAKE_BINARY_DIR}/lib")
target_link_libraries(test_schedule_crontab xgtimer)
list(APPEND TEST_TARGETS test_schedule_crontab)

add_custom_target(test)
add_dependencies(test ${TEST_TARGETS})
INSTALL(TARGETS ${TEST_TARGETS}
        RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/test"
       )
