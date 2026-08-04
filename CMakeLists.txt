cmake_minimum_required(VERSION 3.25)
project(grm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    src/json_utils.cpp
    src/td_client.cpp
    src/app.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_extract.cpp
)


target_link_libraries(grm PRIVATE
    ${TDJSON_LIBRARIES}
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
    src/json_utils.cpp
)
target_link_libraries(test_config PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_config COMMAND test_config)


install(TARGETS grm DESTINATION bin)

