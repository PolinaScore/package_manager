#include "model/packagemanager.h"
#include "view/consoleui.h"
#include <iostream>

int main() {
    try {
        
        PackageManager model;
        
        
        ConsoleUI ui(&model);
        ui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
