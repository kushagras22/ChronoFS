#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>

namespace vcs
{
    namespace fs = std::filesystem;

    struct IndexEntry
    {
        std::string mode;
        std::string hash;
    };

    class Index
    {
    public:
        explicit Index(const fs::path &repoDir);

        bool load();
        bool save() const;

        void add(const std::string &path, const std::string &mode, const std::string &blobHash);
        void remove(const std::string &path);

        const std::unordered_map<std::string, IndexEntry> &entries() const { return entries_; }
        bool has(const std::string &path) const { return entries_.count(path) > 0; }

        fs::path indexPath() const { return repoDir_ / ".chronofs" / "index"; }

    private:
        fs::path repoDir_;
        std::unordered_map<std::string, IndexEntry> entries_;
    };

}
