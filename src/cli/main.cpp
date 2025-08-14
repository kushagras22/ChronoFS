#include "../vcs/Repository.hpp"
#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
static void enableANSI()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

static void printBanner()
{
    const std::string green = "\033[1;32m";
    const std::string cyan = "\033[1;36m";
    const std::string reset = "\033[0m";
    std::cout << green << R"(
   ____ _                             ______  _____ 
  / ___| |__  _ __ ___  _ __   ___   / ___\ \/ / __|
 | |   | '_ \| '__/ _ \| '_ \ / _ \  \___ \>  <\__ \
 | |___| | | | | | (_) | | | |  __/   ___) / . \__) |
  \____|_| |_|_|  \___/|_| |_|\___|  |____/_/\_\___/
)" << reset << "\n";
    std::cout << cyan << "    ChronoFS - Time-travel for your files\n"
              << reset << "\n";
}

using namespace vcs;

static void printUsage()
{
    std::cout <<
        R"(chronofs - simple C++ versioned file manager

Commands:
  init
  add <path>...
  commit -m "<message>" [-a author]
  checkout <commit-hash>
  status
  log
  diff <LEFT> <RIGHT>     # LEFT/RIGHT: WORKING | INDEX | HEAD | <commitHash>

FS helpers:
  fs-mkdir <dir>
  fs-touch <file>
  fs-rm <path>
  fs-mv <from> <to>
)";
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    enableANSI();
#endif
    printBanner();

    std::filesystem::path root = std::filesystem::current_path();
    Repository repo(root);

    if (argc < 2)
    {
        printUsage();
        return 0;
    }
    std::string cmd = argv[1];

    if (cmd == "init")
    {
        if (repo.init())
            std::cout << "Initialized empty repository in .chronofs\n";
        else
            std::cout << "Init failed\n";
        return 0;
    }

    if (!repo.isInitialized())
    {
        std::cerr << "Not a chronofs repository (run `chronofs init`)\n";
        return 1;
    }

    if (cmd == "add")
    {
        bool ok = true;
        for (int i = 2; i < argc; i++)
        {
            ok &= repo.addPath(argv[i]);
        }
        std::cout << (ok ? "Added\n" : "Add failed\n");
        return ok ? 0 : 1;
    }
    else if (cmd == "commit")
    {
        std::string msg, author = "user";
        for (int i = 2; i < argc; i++)
        {
            std::string a = argv[i];
            if (a == "-m" && i + 1 < argc)
            {
                msg = argv[++i];
            }
            else if ((a == "-a" || a == "--author") && i + 1 < argc)
            {
                author = argv[++i];
            }
        }
        if (msg.empty())
        {
            std::cerr << "commit requires -m \"message\"\n";
            return 1;
        }
        auto h = repo.commit(msg, author);
        if (h)
        {
            std::cout << "Committed " << *h << "\n";
            return 0;
        }
        std::cout << "Commit failed\n";
        return 1;
    }
    else if (cmd == "checkout")
    {
        if (argc < 3)
        {
            std::cerr << "checkout <commit-hash>\n";
            return 1;
        }
        if (repo.checkout(argv[2]))
            std::cout << "Checked out " << argv[2] << "\n";
        else
            std::cout << "Checkout failed\n";
        return 0;
    }
    else if (cmd == "status")
    {
        auto s = repo.status();
        for (auto &e : s)
        {
            if (e.state == "clean")
            {
                std::cout << "clean\n";
                break;
            }
            std::cout << e.state << "\t" << e.path << "\n";
        }
        return 0;
    }
    else if (cmd == "log")
    {
        auto lines = repo.log();
        for (auto &l : lines)
            std::cout << l;
        return 0;
    }
    else if (cmd == "diff")
    {
        if (argc < 4)
        {
            std::cerr << "diff <LEFT> <RIGHT>\n";
            return 1;
        }
        std::cout << repo.diff(argv[2], argv[3]);
        return 0;
    }
    else if (cmd == "fs-mkdir")
    {
        if (argc < 3)
        {
            std::cerr << "fs-mkdir <dir>\n";
            return 1;
        }
        std::cout << (repo.fsMkdirs(argv[2]) ? "ok\n" : "fail\n");
        return 0;
    }
    else if (cmd == "fs-touch")
    {
        if (argc < 3)
        {
            std::cerr << "fs-touch <file>\n";
            return 1;
        }
        std::cout << (repo.fsTouch(argv[2]) ? "ok\n" : "fail\n");
        return 0;
    }
    else if (cmd == "fs-rm")
    {
        if (argc < 3)
        {
            std::cerr << "fs-rm <path>\n";
            return 1;
        }
        std::cout << (repo.fsRemove(argv[2]) ? "ok\n" : "fail\n");
        return 0;
    }
    else if (cmd == "fs-mv")
    {
        if (argc < 4)
        {
            std::cerr << "fs-mv <from> <to>\n";
            return 1;
        }
        std::cout << (repo.fsMove(argv[2], argv[3]) ? "ok\n" : "fail\n");
        return 0;
    }
    else
    {
        printUsage();
    }
    return 0;
}
