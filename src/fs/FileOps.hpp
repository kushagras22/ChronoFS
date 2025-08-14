#pragma once
#include <string>
#include <filesystem>

namespace fsops
{
    namespace fs = std::filesystem;

    bool touch(const fs::path &p);
    bool mkdirs(const fs::path &p);
    bool removePath(const fs::path &p);
    bool movePath(const fs::path &from, const fs::path &to);
    bool writeFile(const fs::path &p, const std::string &data);
    bool readFile(const fs::path &p, std::string &out);
}
