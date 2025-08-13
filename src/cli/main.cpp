#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

void printBanner() {
    const std::string green   = "\033[1;32m"; // Bold green
    const std::string cyan    = "\033[1;36m"; // Bold cyan
    const std::string reset   = "\033[0m";    // Reset color

    std::cout << green << R"(
   ____ _                             ______  _____ 
  / ___| |__  _ __ ___  _ __   ___   / ___\ \/ / __|
 | |   | '_ \| '__/ _ \| '_ \ / _ \  \___ \>  <\__ \
 | |___| | | | | | (_) | | | |  __/   ___) / . \__) |
  \____|_| |_|_|  \___/|_| |_|\___|  |____/_/\_\___/
)" << reset << "\n";

    std::cout << cyan << "    ChronoFS - Time-travel for your files\n" << reset << "\n";
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    enableANSI();
    #endif

    printBanner();

    if (argc < 2) {
        std::cout << "Usage: chronofs <command> [options]\n";
        std::cout << "Commands: init, add, commit, status, log, diff, checkout\n";
        return 0;
    }

    std::string command = argv[1];

    if (command == "init") {
        std::cout << "Initializing ChronoFS repository...\n";
        // TODO: Call Repository::init()
    }
    else if (command == "add") {
        std::cout << "Adding file to staging area...\n";
        // TODO: Call Index::add()
    }
    else if (command == "commit") {
        std::cout << "Committing changes...\n";
        // TODO: Call Repository::commit()
    }
    else if (command == "status") {
        std::cout << "Checking repository status...\n";
        // TODO: Call Repository::status()
    }
    else if (command == "log") {
        std::cout << "Displaying commit log...\n";
        // TODO: Call Repository::log()
    }
    else if (command == "diff") {
        std::cout << "Showing differences...\n";
        // TODO: Call Diff::show()
    }
    else if (command == "checkout") {
        std::cout << "Restoring file from commit...\n";
        // TODO: Call Repository::checkout()
    }
    else {
        std::cerr << "Unknown command: " << command << "\n";
    }

    return 0;
}
