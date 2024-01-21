/*
WARNING: UNSURE IF THIS WORKS
*/


#include <iostream>
#include <cstdlib>
#include <dlfcn.h> // For dynamic loading in Linux. Use <windows.h> for Windows.

int main() {
    // Retrieve the environment variable
    const char* qtPath = std::getenv("QT_PATH");
    if (!qtPath) {
        std::cerr << "Error: QT_PATH environment variable not set." << std::endl;
        return 1;
    }

    // Construct the path to the Qt library
    std::string qtLibPath = std::string(qtPath) + "/libQt5Core.so.5"; // Example for Linux

    // Attempt to dynamically load the Qt library
    void* handle = dlopen(qtLibPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load Qt library: " << dlerror() << std::endl;
        return 1;
    }

    // ... (Your application logic using Qt goes here)

    // Close the library handle when done
    dlclose(handle);
    return 0;
}