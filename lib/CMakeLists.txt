set(LIBXGTIMER_HRD "${XG_TIMER_PROJ_TOP}/include/logger"
                   "${XG_TIMER_PROJ_TOP}/include"
                   "${XG_TIMER_PROJ_TOP}/lib"
                   )
set(LIBXGTIMER_SRC timer_schedule_rule_duration.cc
                   )

set(LIBXGTIMER_TARGETS)
if(BUILD_SHARED_LIBS)
    add_library(libxgtimer.so SHARED ${LIBXGTIMER_SRC})
    list(LENGTH DEPEND_PROJECTS DEPEND_PROJECTS_NUM)
    target_include_directories(libxgtimer.so PRIVATE ${LIBXGTIMER_HRD})
    target_link_directories(libxgtimer.so PRIVATE ${CMAKE_BINARY_DIR})
    set_target_properties(libxgtimer.so PROPERTIES OUTPUT_NAME "xgtimer" CLEAN_DIRECT_OUTPUT 1)
    list(APPEND LIBXGTIMER_TARGETS libxgtimer.so)
endif()

if(BUILD_STATIC_LIBS)
    add_library(libxgtimer.a STATIC ${LIBXGTIMER_SRC})
    list(LENGTH DEPEND_PROJECTS DEPEND_PROJECTS_NUM)
    target_include_directories(libxgtimer.a PRIVATE ${LIBXGTIMER_HRD})
    target_link_directories(libxgtimer.a PRIVATE ${CMAKE_BINARY_DIR})
    set_target_properties(libxgtimer.a PROPERTIES OUTPUT_NAME "xgtimer" CLEAN_DIRECT_OUTPUT 1)
    list(APPEND LIBXGTIMER_TARGETS libxgtimer.a)
endif()

add_custom_target(lib)
add_dependencies(lib ${LIBXGTIMER_TARGETS})

INSTALL(TARGETS ${LIBXGTIMER_TARGETS})
