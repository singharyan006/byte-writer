#include "editor/Editor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        Editor editor;
        editor.init();
        
        if (argc >= 2) {
            editor.openFile(argv[1]);
        }
        
        editor.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

