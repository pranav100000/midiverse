cmake_minimum_required(VERSION 3.10)
project(midiverse)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/crow/include)
include_directories(/opt/homebrew/include) # For Asio on macOS

# Check for JUCE
option(USE_JUCE "Build with JUCE for actual VST support" OFF)
if(USE_JUCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/JUCE)
    
    # Enable VST features
    add_compile_definitions(USE_JUCE=1)
endif()

# Add executable
add_executable(midiverse 
    src/main.cpp
    src/midi_processor.cpp
    src/vst_renderer.cpp
    src/audio_writer.cpp
    src/server.cpp
)

# Link libraries
target_link_libraries(midiverse PRIVATE 
    Threads::Threads
)

if(USE_JUCE)
    target_link_libraries(midiverse PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
    )
endif()

# Additional compiler options
if(APPLE)
    target_link_libraries(midiverse PRIVATE "-framework CoreFoundation" "-framework CoreAudio" "-framework AudioToolbox")
endif()

# Installation
install(TARGETS midiverse DESTINATION bin)

# Tests
add_subdirectory(tests)