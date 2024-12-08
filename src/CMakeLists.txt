cmake_minimum_required(VERSION 3.12)

# 使用 vcpkg 工具链文件 Warning  工具连设置必须是在project 前面
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(evFTP)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(MSVC)
  set(CMAKE_GENERATOR_PLATFORM x64)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug) 

# 查找 vcpkg 安装的库
find_package(Libevent CONFIG REQUIRED)
message("Found Libevent FOUND: ${Libevent_FOUND}")
message("Found Libevent VERSION: ${Libevent_VERSION}")
find_package(ZLIB REQUIRED)
message("Found ZLIB FOUND: ${ZLIB_FOUND}")
message("Found ZLIB VERSION: ${ZLIB_VERSION}")
find_package(protobuf CONFIG REQUIRED)
message("Found protobuf FOUND: ${protobuf_FOUND}")
message("Found protobuf VERSION: ${protobuf_VERSION}")
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc executable not found")
else()
    message(STATUS "Found protoc: ${PROTOC_EXECUTABLE}")
endif()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CommonTools)

# 外部依赖项
set(DPS_INCLUDES)
set(DPS_TARGETS)
set(DPS_LIBRARYS)


add_subdirectory(evserver)
add_subdirectory(evconf)
add_subdirectory(evtimer)
add_subdirectory(evsocket)
add_subdirectory(evbuffer_server)
add_subdirectory(evbuffer_client)
add_subdirectory(evbuffer_filter)
add_subdirectory(evbuffer_zlib_server)
add_subdirectory(evhttp)
add_subdirectory(evhttp_client)
add_subdirectory(evthreadpool)
add_subdirectory(evbuffer_protobuf)
add_subdirectory(evbuffer_protobuf_server)

if(NOT WIN32)
  add_subdirectory(evsignal)
  add_subdirectory(evfile)
  add_subdirectory(evloop)
endif()

# 拷贝asset目录到输出目录
set(ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/asset)

# 查找源目录中的所有文件
file(GLOB_RECURSE FILES_TO_COPY "${ASSET_PATH}/*")

# 循环复制文件到目标目录
foreach(FILE ${FILES_TO_COPY})
    # 获取仅文件名
    get_filename_component(FILENAME ${FILE} NAME)
    
    # 复制文件到目标目录
    file(COPY ${FILE} DESTINATION ${OUT_RUN_PATH})
	message("CPOY ${FILE}")
endforeach()
