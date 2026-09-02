cmake_minimum_required(VERSION 3.25)
project(grm VERSION 0.9.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Release optimizations & symbol stripping
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections -s")

# Debug unoptimized development build with full debug symbols
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -DDEBUG")

# Sanitizer build options (ASan + UBSan / TSan)
option(ENABLE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer and UndefinedBehaviorSanitizer enabled")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()




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

# Fallback search order: ~/.local first (for local upgraded TDLib build), then system directories
list(PREPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")
if(DEFINED ENV{PKG_CONFIG_PATH})
    set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/.local/lib64/pkgconfig:$ENV{HOME}/.local/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
else()
    set(ENV{PKG_CONFIG_PATH} "$ENV{HOME}/.local/lib64/pkgconfig:$ENV{HOME}/.local/lib/pkgconfig:/usr/lib64/pkgconfig:/usr/share/pkgconfig")
endif()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib64;\$ORIGIN/../lib;\$ORIGIN/../lib64;$ENV{HOME}/.local/lib;$ENV{HOME}/.local/lib64")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TDJSON REQUIRED tdjson>=1.8.20)
pkg_check_modules(JSONC REQUIRED json-c)

# Dynamic Git-based Versioning
find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    # Check if working directory has uncommitted changes
    execute_process(
        COMMAND ${GIT_EXECUTABLE} status --porcelain
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_STATUS_OUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(GIT_STATUS_OUT)
        set(GIT_DIRTY_SUFFIX "-dirty")
    else()
        set(GIT_DIRTY_SUFFIX "")
    endif()

    # Check if we are on a clean exact tag
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_EXACT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_TAG_RES
    )

    if(GIT_TAG_RES EQUAL 0 AND NOT GIT_STATUS_OUT)
        # Clean exact tagged release
        string(REGEX REPLACE "^v" "" CLEAN_VER "${GIT_EXACT_TAG}")
        set(GRM_VERSION_STR "${CLEAN_VER}")
        set(GRM_BANNER_VERSION_STR "${GIT_EXACT_TAG}")
    else()
        # Development / master build
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_SHORT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(NOT GIT_SHORT_HASH)
            set(GIT_SHORT_HASH "unknown")
        endif()
        set(GRM_VERSION_STR "devel (${GIT_SHORT_HASH}${GIT_DIRTY_SUFFIX})")
        set(GRM_BANNER_VERSION_STR "devel (${GIT_SHORT_HASH}${GIT_DIRTY_SUFFIX})")
    endif()
else()
    set(GRM_VERSION_STR "devel")
    set(GRM_BANNER_VERSION_STR "devel")
endif()

message(STATUS "Configured grm version: ${GRM_VERSION_STR}")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/grm/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/grm/version.hpp"
    @ONLY
)

link_directories(
    ${TDJSON_LIBRARY_DIRS}
    ${JSONC_LIBRARY_DIRS}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/include
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
    src/cmd_folder.cpp
    src/cmd_search.cpp
    src/cmd_story.cpp
    src/cmd_status.cpp
    src/cmd_completion.cpp
    src/command_registry.cpp

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

add_executable(test_cli_routing
    tests/test_cli_routing.cpp
    src/app.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/cmd_folder.cpp
    src/cmd_search.cpp
    src/cmd_story.cpp
    src/cmd_status.cpp
    src/cmd_completion.cpp
    src/command_registry.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_cli_routing PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_cli_routing COMMAND test_cli_routing)

add_executable(test_completion
    tests/test_completion.cpp
    src/app.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/cmd_folder.cpp
    src/cmd_search.cpp
    src/cmd_story.cpp
    src/cmd_status.cpp
    src/cmd_completion.cpp
    src/command_registry.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_completion PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_completion COMMAND test_completion)

add_executable(test_command_registry
    tests/test_command_registry.cpp
    src/command_registry.cpp
    src/app.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/cmd_folder.cpp
    src/cmd_search.cpp
    src/cmd_story.cpp
    src/cmd_status.cpp
    src/cmd_completion.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_command_registry PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_command_registry COMMAND test_command_registry)


add_executable(test_folder
    tests/test_folder.cpp
    src/cmd_folder.cpp
    src/cmd_search.cpp
    src/cmd_story.cpp
    src/cmd_status.cpp
    src/command_registry.cpp
    src/app.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/cmd_completion.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_folder PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_folder COMMAND test_folder)

add_executable(test_search
    tests/test_search.cpp
    src/cmd_search.cpp
    src/cmd_folder.cpp
    src/cmd_story.cpp
    src/cmd_status.cpp
    src/command_registry.cpp
    src/app.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/cmd_completion.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_search PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_search COMMAND test_search)


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

add_executable(test_td_client_flush
    tests/test_td_client_flush.cpp
    src/td_client.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_td_client_flush PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_td_client_flush COMMAND test_td_client_flush)
set_tests_properties(test_td_client_flush PROPERTIES RUN_SERIAL TRUE)

add_executable(test_cli_options
    tests/test_cli_options.cpp
)
add_test(NAME test_cli_options COMMAND test_cli_options)

add_executable(test_config
    tests/test_config.cpp
    src/config.cpp
    src/json_utils.cpp
)
target_link_libraries(test_config PRIVATE ${JSONC_LIBRARIES})
add_test(NAME test_config COMMAND test_config)

add_executable(test_story
    tests/test_story.cpp
    src/cmd_story.cpp
    src/app.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/cmd_folder.cpp
    src/cmd_search.cpp
    src/cmd_status.cpp
    src/cmd_completion.cpp
    src/command_registry.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_story PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_story COMMAND test_story)

add_executable(test_status
    tests/test_status.cpp
    src/cmd_status.cpp
    src/app.cpp
    src/td_client.cpp
    src/cmd_auth.cpp
    src/cmd_chat.cpp
    src/cmd_msg.cpp
    src/cmd_topic.cpp
    src/cmd_file.cpp
    src/cmd_folder.cpp
    src/cmd_search.cpp
    src/cmd_story.cpp
    src/cmd_completion.cpp
    src/command_registry.cpp
    src/list_options.cpp
    src/exporter.cpp
    src/uploader.cpp
    src/downloader.cpp
    src/formatter.cpp
    src/config.cpp
    src/logger.cpp
    src/json_utils.cpp
)
target_link_libraries(test_status PRIVATE ${TDJSON_LIBRARIES} ${JSONC_LIBRARIES})
add_test(NAME test_status COMMAND test_status)

endif()

install(TARGETS grm DESTINATION bin)

