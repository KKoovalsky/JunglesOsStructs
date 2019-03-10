cmake_minimum_required(VERSION 3.5)

project(JunglesOsStructs-tests)
set(PRJ_NAME ${CMAKE_PROJECT_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -Wall)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wall)

set(TESTS ${CMAKE_SOURCE_DIR}/tests)
set(EXT_DEPS ${CMAKE_SOURCE_DIR}/ext_deps)
set(UNITY_DIR ${EXT_DEPS}/unity/src)
set(JUNGLES_DATA_STRUCTS ${EXT_DEPS}/jungles_data_structs/inc)
set(FREERTOS_DIR ${EXT_DEPS}/FreeRTOS/lib)
set(FREERTOS_PORT_DIR ${EXT_DEPS}/FreeRTOSAddons/Linux/portable/GCC/Linux)

set(HEADERS_PATH "${CMAKE_SOURCE_DIR}/inc")
set(SOURCES_PATH "${CMAKE_SOURCE_DIR}/src")

add_definitions(-D__FREERTOS__)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories("${HEADERS_PATH}" ${UNITY_DIR} ${JUNGLES_DATA_STRUCTS} ${FREERTOS_DIR}/include/private
    ${FREERTOS_DIR}/include ${TESTS} ${FREERTOS_PORT_DIR})

file(GLOB SOURCES ${TESTS}/*.c* ${FREERTOS_DIR}/FreeRTOS/*.c* ${FREERTOS_PORT_DIR}/*.c*)

add_executable(${PRJ_NAME} ${SOURCES} ${UNITY_DIR}/unity.c ${EXT_DEPS}/FreeRTOS/lib/FreeRTOS/portable/MemMang/heap_3.c
    ${SOURCES_PATH}/os_lockguard.cpp)

target_link_libraries(${PRJ_NAME} Threads::Threads)

add_custom_target(run-test
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./${PRJ_NAME}
    )
