#!/bin/bash
set -e

# Create lib directory if it doesn't exist
mkdir -p lib

# Install Crow (header-only library)
echo "Installing Crow..."
if [ ! -d "lib/crow" ]; then
    git clone https://github.com/CrowCpp/Crow.git lib/crow
fi

# Install JUCE (for VST support)
echo "Installing JUCE..."
if [ ! -d "lib/JUCE" ]; then
    git clone https://github.com/juce-framework/JUCE.git lib/JUCE
fi

# Install Asio if we're on macOS with Homebrew
if [ "$(uname)" == "Darwin" ] && command -v brew &> /dev/null; then
    echo "Installing Asio using Homebrew..."
    brew install asio
else
    echo "NOTE: You may need to install the Asio library manually."
    echo "On Ubuntu/Debian: sudo apt install libasio-dev"
    echo "On CentOS/Fedora: sudo dnf install asio-devel"
    echo "On macOS (without Homebrew): Download from https://think-async.com/Asio/"
fi

# Create output directory
mkdir -p output

# Make script executable
chmod +x "$0"

echo "Dependencies installed successfully!"
echo ""
echo "To build with JUCE and VST support, use:"
echo "mkdir -p build && cd build && cmake -DUSE_JUCE=ON .. && make"
echo ""
echo "For minimal build without VST support:"
echo "mkdir -p build && cd build && cmake .. && make"