cmake_minimum_required(VERSION 3.15)

set(LIBRARY_NAME log_c)
project(${LIBRARY_NAME} C)
set(CMAKE_C_STANDARD 99)

option(${LIBRARY_NAME}_BUILD_DEMO "Build demo application" ON)

add_library(${LIBRARY_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/log.h)
target_include_directories(${LIBRARY_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src)

if(${${LIBRARY_NAME}_BUILD_DEMO})
    add_executable(test_${LIBRARY_NAME} ${CMAKE_CURRENT_LIST_DIR}/main.c)
    target_link_libraries(test_${LIBRARY_NAME} PRIVATE ${LIBRARY_NAME})
endif()