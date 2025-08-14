#include "fs/FileOps.hpp"
#include <fstream>

namespace fsops
{
    bool touch(const fs::path &p)
    {
        if (std::filesystem::exists(p))
            return true;
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());
        std::ofstream f(p, std::ios::binary);
        return (bool)f;
    }
    bool mkdirs(const fs::path &p)
    {
        if (std::filesystem::exists(p))
            return std::filesystem::is_directory(p);
        return std::filesystem::create_directories(p);
    }
    bool removePath(const fs::path &p)
    {
        std::error_code ec;
        std::filesystem::remove_all(p, ec);
        return !ec;
    }
    bool movePath(const fs::path &from, const fs::path &to)
    {
        std::error_code ec;
        if (to.has_parent_path())
            std::filesystem::create_directories(to.parent_path());
        std::filesystem::rename(from, to, ec);
        return !ec;
    }
    bool writeFile(const fs::path &p, const std::string &data)
    {
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());
        std::ofstream f(p, std::ios::binary);
        if (!f)
            return false;
        f.write(data.data(), (std::streamsize)data.size());
        return true;
    }
    bool readFile(const fs::path &p, std::string &out)
    {
        std::ifstream f(p, std::ios::binary);
        if (!f)
            return false;
        f.seekg(0, std::ios::end);
        std::streamsize size = f.tellg();
        f.seekg(0, std::ios::beg);
        out.resize((size_t)size);
        if (size > 0)
            f.read(&out[0], size);
        return true;
    }
}
