#include "core/Application.h"
#include <spdlog/spdlog.h>
#include <exception>

int main(int argc, char *argv[]) {
    try {
        // Create and run application
        Chatbot::Application app(argc, argv);
        return app.run();
    }
    catch (const std::exception& e) {
        spdlog::critical("Fatal error: {}", e.what());
        return 1;
    }
    catch (...) {
        spdlog::critical("Unknown fatal error occurred");
        return 1;
    }
}
