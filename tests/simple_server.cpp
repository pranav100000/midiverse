#include <crow.h>
#include <iostream>

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello, World!";
    });

    CROW_ROUTE(app, "/health")([]() {
        return "OK";
    });

    std::cout << "Starting test server on http://127.0.0.1:8888" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    // Explicitly bind to localhost only for testing
    app.bindaddr("127.0.0.1").port(8888).run();
    
    return 0;
}