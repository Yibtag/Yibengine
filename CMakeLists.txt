cmake_minimum_required(VERSION 3.10)

include(FetchContent)

project(Yibengine VERSION 1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(CLIENT_DIR "${SOURCE_DIR}/client")
set(SERVER_DIR "${SOURCE_DIR}/server")
set(SHARED_DIR "${SOURCE_DIR}/shared")

file(GLOB_RECURSE CLIENT_SOURCES "${CLIENT_DIR}/*.cpp" "${CLIENT_DIR}/*.h")
file(GLOB_RECURSE SERVER_SOURCES "${SERVER_DIR}/*.cpp" "${SERVER_DIR}/*.h")
file(GLOB_RECURSE SHARED_SOURCES "${SHARED_DIR}/*.cpp" "${SHARED_DIR}/*.h")

add_executable(YibengineClient ${SHARED_SOURCES} ${CLIENT_SOURCES})
add_executable(YibengineServer ${SHARED_SOURCES} ${SERVER_SOURCES})

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
FetchContent_MakeAvailable(glfw)
if (TARGET glfw)
    target_link_libraries(YibengineClient glfw)
endif()

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/icaven/glm.git)
FetchContent_MakeAvailable(glm)
if (TARGET glm)
    target_link_libraries(YibengineClient glm)
endif()

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
	target_link_libraries(YibengineClient Vulkan::Vulkan)
endif()