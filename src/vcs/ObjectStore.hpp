#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace vcs
{
    namespace fs = std::filesystem;

    struct TreeEntry
    {
        std::string mode; // "100644" or "040000"
        std::string name;
        std::string hash; // sha256
    };

    class ObjectStore
    {
    public:
        explicit ObjectStore(const fs::path &repoDir);

        std::string writeBlob(const std::string &data); // returns hash
        bool readBlob(const std::string &hash, std::string &out) const;

        std::string writeTree(const std::vector<TreeEntry> &entries);
        bool readTree(const std::string &hash, std::vector<TreeEntry> &out) const;

        std::string writeCommit(const std::string &treeHash,
                                const std::string &parentHash,
                                const std::string &author,
                                long long timestamp,
                                const std::string &message);
        bool readCommit(const std::string &hash, std::string &treeHash,
                        std::string &parentHash, std::string &author,
                        long long &timestamp, std::string &message) const;

        fs::path objectsDir() const { return objectsDir_; }

    private:
        fs::path repoDir_;
        fs::path objectsDir_;
        bool readObject(const std::string &hash, std::string &out) const;
        bool writeObject(const std::string &content, std::string &outHash);
    };

} 
