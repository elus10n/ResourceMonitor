cmake_minimum_required(VERSION 3.22)
project(LinuxSystemMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    asio
    URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(asio)

add_executable(system_monitor
    backend/source/parser.cpp
    backend/source/data_manager.cpp
    backend/source/server.cpp
    main.cpp
)

target_compile_definitions(system_monitor PRIVATE CROW_ENABLE_ASIO)

find_package(Threads REQUIRED)
target_link_libraries(system_monitor PRIVATE Threads::Threads)

target_include_directories(system_monitor PRIVATE
    include
    libs
    ${asio_SOURCE_DIR}/asio/include
)

add_custom_command(TARGET system_monitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/index.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/static ${CMAKE_CURRENT_BINARY_DIR}/static
)