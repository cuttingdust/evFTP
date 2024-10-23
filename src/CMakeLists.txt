cmake_minimum_required(VERSION 3.12)

# 使用 vcpkg 工具链文件 Warning  工具连设置必须是在project 前面
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(evFTP)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_GENERATOR_PLATFORM x64)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug) 

# 查找 vcpkg 安装的库
find_package(Libevent CONFIG REQUIRED)
message("Found libevent FOUND: ${Libevent_FOUND}")
message("Found libevent VERSION: ${Libevent_VERSION}")
message("Found libevent include: ${Libevent_INCLUDE_DIR}")
message("Found libevent include: ${Libevent_INCLUDES}")
message("Found libevent Library: ${Libevent_LIBRARY}")
message("Found libevent LIbRary: ${Libevent_LIBRARIES}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CommonTools)

add_subdirectory(evserver)
add_subdirectory(evconf)
