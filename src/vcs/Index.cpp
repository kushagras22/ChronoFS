#include "vcs/Index.hpp"
#include "fs/FileOps.hpp"
#include <fstream>
#include <sstream>

namespace vcs
{

    Index::Index(const fs::path &repoDir) : repoDir_(repoDir) {}

    bool Index::load()
    {
        entries_.clear();
        std::ifstream f(indexPath());
        if (!f)
            return true; // empty ok
        std::string mode, path, hash;
        while (f >> mode >> path >> hash)
        {
            entries_[path] = IndexEntry{mode, hash};
        }
        return true;
    }

    bool Index::save() const
    {
        std::ofstream f(indexPath());
        if (!f)
            return false;
        for (auto &kv : entries_)
        {
            f << kv.second.mode << ' ' << kv.first << ' ' << kv.second.hash << '\n';
        }
        return true;
    }

    void Index::add(const std::string &path, const std::string &mode, const std::string &blobHash)
    {
        entries_[path] = IndexEntry{mode, blobHash};
    }

    void Index::remove(const std::string &path)
    {
        entries_.erase(path);
    }

}
