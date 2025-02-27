#include "server.h"
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <string>

Server* serverInstance = nullptr;

void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down..." << std::endl;
    if (serverInstance) {
        serverInstance->stop();
    }
    exit(signal);
}

int main(int argc, char* argv[]) {
    // Parse command line arguments (port, etc.)
    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Print debug info
    std::cout << "System information:" << std::endl;
    std::cout << "----------------" << std::endl;
    
    // Check if we can bind to ports
    #ifdef _WIN32
    std::cout << "Platform: Windows" << std::endl;
    #elif __APPLE__
    std::cout << "Platform: macOS" << std::endl;
    #elif __linux__
    std::cout << "Platform: Linux" << std::endl;
    #else
    std::cout << "Platform: Unknown" << std::endl;
    #endif
    
    std::cout << "Port: " << port << std::endl;
    std::cout << "----------------" << std::endl;
    
    try {
        Server server(port);
        serverInstance = &server;
        
        std::cout << "Midiverse server starting on port " << port << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // Start the server
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}