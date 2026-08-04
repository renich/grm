cmake_minimum_required(VERSION 3.25)
project(grm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Release optimizations & symbol stripping
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections -s")

# Debug unoptimized development build with full debug symbols
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -DDEBUG")




# Mandatory strict compiler flags
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Werror
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TDJSON REQUIRED tdjson)
pkg_check_modules(JSONC REQUIRED json-c)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${TDJSON_INCLUDE_DIRS}
    ${JSONC_INCLUDE_DIRS}
)

add_executable(grm
    src/main.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
    src/td_client.cpp
    src/app.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_extract.cpp
    src/cmd_file.cpp
    src/cmd_topic.cpp
    src/exporter.cpp
    src/uploader.cpp
)




set(LOCAL_TDJSON "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libtdjson.so")
if(EXISTS "${LOCAL_TDJSON}")
    set(TDJSON_LIB "${LOCAL_TDJSON}")
    set(CMAKE_BUILD_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib")
    set(CMAKE_INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib")
else()
    set(TDJSON_LIB ${TDJSON_LIBRARIES})
endif()

target_link_libraries(grm PRIVATE
    ${TDJSON_LIB}
    ${JSONC_LIBRARIES}
)




enable_testing()

add_executable(test_json_utils
    tests/test_json_utils.cpp
    src/json_utils.cpp
)
target_link_libraries(test_json_utils PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_json_utils COMMAND test_json_utils)

add_executable(test_config
    tests/test_config.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)

target_link_libraries(test_config PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_config COMMAND test_config)

add_executable(test_export
    tests/test_export.cpp
    src/exporter.cpp
    src/json_utils.cpp
)
target_link_libraries(test_export PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_export COMMAND test_export)

add_executable(test_file
    tests/test_file.cpp
    src/uploader.cpp
    src/json_utils.cpp
)
target_link_libraries(test_file PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_file COMMAND test_file)


add_executable(test_topic
    tests/test_topic.cpp
    src/uploader.cpp
    src/json_utils.cpp
)
target_link_libraries(test_topic PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_topic COMMAND test_topic)

install(TARGETS grm DESTINATION bin)




