#pragma once
#include "../vcs/ObjectStore.hpp"
#include "../vcs/Index.hpp"
#include <string>
#include <filesystem>
#include <optional>

namespace vcs
{
    namespace fs = std::filesystem;

    class Repository
    {
    public:
        explicit Repository(const fs::path &root);

        // Core repo ops
        bool init();
        bool isInitialized() const;
        std::string currentHeadRef() const;             // e.g., "refs/heads/main"
        std::optional<std::string> resolveHEAD() const; // commit hash

        bool setHeadRef(const std::string &refPath); // write HEAD: "ref: <refPath>"
        bool updateRef(const std::string &refPath, const std::string &commitHash);
        std::optional<std::string> readRef(const std::string &refPath) const;

        // Staging/commit
        bool addPath(const fs::path &relPath); // stage file
        std::optional<std::string> commit(const std::string &message, const std::string &author);

        // Checkout
        bool checkout(const std::string &commitHash);

        // Status & log & diff
        struct StatusEntry
        {
            std::string path;
            std::string state;
        }; // "staged", "modified", "deleted", "untracked", "clean"
        std::vector<StatusEntry> status() const;
        std::vector<std::string> log() const;
        std::string diff(const std::string &a, const std::string &b) const; // a,b: "WORKING", "INDEX", "HEAD" or commit hash

        // FS helpers (exposed via CLI)
        bool fsTouch(const fs::path &path) const;
        bool fsMkdirs(const fs::path &path) const;
        bool fsRemove(const fs::path &path) const;
        bool fsMove(const fs::path &from, const fs::path &to) const;

    private:
        fs::path root_;
        ObjectStore store_;
        mutable Index index_;

        fs::path dotDir() const { return root_ / ".chronofs"; }
        fs::path headFile() const { return dotDir() / "HEAD"; }
        fs::path refsHeadsDir() const { return dotDir() / "refs" / "heads"; }

        static bool readFile(const fs::path &p, std::string &out);
        static bool writeFile(const fs::path &p, const std::string &data);

        std::string buildTreeFromIndex() const;
        void writeTreeRecursive(const std::vector<std::pair<std::string, IndexEntry>> &items,
                                size_t start, size_t end, const std::string &dirPrefix,
                                std::string &treeHash) const;

        std::optional<std::string> workingBlobHash(const fs::path &relPath) const;
        std::optional<std::string> blobHashOfCommitPath(const std::string &commitHash, const std::string &relPath) const;
        bool materializeTree(const std::string &treeHash, const fs::path &dir) const;
        std::optional<std::string> headCommit() const { return resolveHEAD(); }
    };

}
