#include "core/core_engine.hpp"
#include "loader/library_loader.hpp"
#include "loader/mod_loader.hpp"
#include "core/platform_abstraction.hpp"
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    std::cout << "Testing Core Engine..." << std::endl;
    
    CoreEngine engine;
    if (engine.initialize()) {
        std::cout << "Core engine initialized successfully" << std::endl;
        std::cout << engine.getPlatformInfo() << std::endl;
        engine.cleanup();
        std::cout << "All tests passed!" << std::endl;
        return 0;
    }
    
    std::cerr << "Core engine initialization failed" << std::endl;
    return 1;
}