cmake_minimum_required(VERSION 3.25)
project(grm VERSION 0.5.0 LANGUAGES CXX)

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

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TDJSON REQUIRED tdjson)
pkg_check_modules(JSONC REQUIRED json-c)

link_directories(
    ${TDJSON_LIBRARY_DIRS}
    ${JSONC_LIBRARY_DIRS}
)

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
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/list_options.cpp
    src/exporter.cpp



    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
)






target_link_libraries(grm PRIVATE
    ${TDJSON_LDFLAGS}
    ${JSONC_LIBRARIES}
)




include(CTest)
if(BUILD_TESTING)

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

add_executable(test_formatter
    tests/test_formatter.cpp
    src/formatter.cpp
    src/json_utils.cpp
)
target_link_libraries(test_formatter PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_formatter COMMAND test_formatter)

add_executable(test_render_engine
    tests/test_render_engine.cpp
    src/formatter.cpp
    src/json_utils.cpp
)
target_link_libraries(test_render_engine PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_render_engine COMMAND test_render_engine)

add_executable(test_cli_options
    tests/test_cli_options.cpp
)
add_test(NAME test_cli_options COMMAND test_cli_options)

add_executable(test_cli_routing
    tests/test_cli_routing.cpp
    src/app.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
)
target_link_libraries(test_cli_routing PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_cli_routing COMMAND test_cli_routing)

add_executable(test_downloader
    tests/test_downloader.cpp
    src/downloader.cpp
)
add_test(NAME test_downloader COMMAND test_downloader)

add_executable(test_chat_crud
    tests/test_chat_crud.cpp
    src/json_utils.cpp
)
target_link_libraries(test_chat_crud PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_chat_crud COMMAND test_chat_crud)

add_executable(test_topic_crud
    tests/test_topic_crud.cpp
    src/json_utils.cpp
)
target_link_libraries(test_topic_crud PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_topic_crud COMMAND test_topic_crud)

add_executable(test_msg_crud
    tests/test_msg_crud.cpp
    src/json_utils.cpp
)
target_link_libraries(test_msg_crud PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_msg_crud COMMAND test_msg_crud)

endif()

install(TARGETS grm DESTINATION bin)
