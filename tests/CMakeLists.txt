cmake_minimum_required(VERSION 3.10)
project(midiverse_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/crow/include)
include_directories(/opt/homebrew/include) # For Asio on macOS

# Add test executables
add_executable(simple_server simple_server.cpp)

# Link libraries
target_link_libraries(simple_server PRIVATE Threads::Threads)

# Additional compiler options
if(APPLE)
    target_link_libraries(simple_server PRIVATE "-framework CoreFoundation" "-framework CoreAudio" "-framework AudioToolbox")
endif()