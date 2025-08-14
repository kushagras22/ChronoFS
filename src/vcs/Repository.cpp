#include "vcs/Repository.hpp"
#include "fs/FileOps.hpp"
#include "vcs/Diff.hpp"
#include <fstream>
#include <sstream>
#include <chrono>
#include <map>
#include <set>

namespace vcs
{

    Repository::Repository(const fs::path &root)
        : root_(fs::absolute(root)), store_(root_), index_(root_) {}

    bool Repository::init()
    {
        if (isInitialized())
            return true;
        std::filesystem::create_directories(store_.objectsDir());
        std::filesystem::create_directories(refsHeadsDir());
        writeFile(headFile(), "ref: refs/heads/main\n");
        writeFile(refsHeadsDir() / "main", ""); // unborn branch
        index_.load();
        index_.save();
        return true;
    }

    bool Repository::isInitialized() const { return std::filesystem::exists(dotDir()); }

    std::string Repository::currentHeadRef() const
    {
        std::string s;
        if (!readFile(headFile(), s))
            return "";
        if (s.rfind("ref: ", 0) == 0)
        {
            std::string ref = s.substr(5);
            if (!ref.empty() && ref.back() == '\n')
                ref.pop_back();
            return ref;
        }
        return "";
    }

    bool Repository::setHeadRef(const std::string &refPath)
    {
        return writeFile(headFile(), "ref: " + refPath + "\n");
    }

    bool Repository::updateRef(const std::string &refPath, const std::string &commitHash)
    {
        return writeFile(dotDir() / refPath, commitHash + "\n");
    }

    std::optional<std::string> Repository::readRef(const std::string &refPath) const
    {
        std::string s;
        if (!readFile(dotDir() / refPath, s))
            return std::nullopt;
        if (!s.empty() && s.back() == '\n')
            s.pop_back();
        if (s.empty())
            return std::nullopt;
        return s;
    }

    std::optional<std::string> Repository::resolveHEAD() const
    {
        auto ref = currentHeadRef();
        if (ref.empty())
            return std::nullopt;
        return readRef(ref);
    }

    bool Repository::readFile(const fs::path &p, std::string &out)
    {
        return fsops::readFile(p, out);
    }
    bool Repository::writeFile(const fs::path &p, const std::string &data)
    {
        return fsops::writeFile(p, data);
    }

    bool Repository::addPath(const fs::path &relPath)
    {
        auto abs = root_ / relPath;
        if (!std::filesystem::exists(abs) || std::filesystem::is_directory(abs))
            return false;
        std::string data;
        if (!fsops::readFile(abs, data))
            return false;
        auto blob = store_.writeBlob(data);
        index_.load();
        index_.add(relPath.generic_string(), "100644", blob);
        return index_.save();
    }

    static std::vector<std::pair<std::string, IndexEntry>> sortIndexEntries(const std::unordered_map<std::string, IndexEntry> &m)
    {
        std::vector<std::pair<std::string, IndexEntry>> v(m.begin(), m.end());
        std::sort(v.begin(), v.end(), [](auto &a, auto &b)
                  { return a.first < b.first; });
        return v;
    }

    void Repository::writeTreeRecursive(const std::vector<std::pair<std::string, IndexEntry>> &items,
                                        size_t start, size_t end, const std::string &dirPrefix,
                                        std::string &treeHash) const
    {
        std::map<std::string, std::vector<std::pair<std::string, IndexEntry>>> children;
        std::vector<TreeEntry> entries;
        for (size_t i = start; i < end; i++)
        {
            auto rel = items[i].first;
            if (rel.compare(0, dirPrefix.size(), dirPrefix) != 0)
                continue;
            auto rest = rel.substr(dirPrefix.size());
            auto pos = rest.find('/');
            if (pos == std::string::npos)
            {
                entries.push_back(TreeEntry{"100644", rest, items[i].second.hash});
            }
            else
            {
                std::string child = rest.substr(0, pos);
                children[child].push_back(items[i]);
            }
        }
        for (auto &kv : children)
        {
            std::string childDir = dirPrefix + kv.first + "/";
            std::string subHash;
            writeTreeRecursive(items, start, end, childDir, subHash);
            entries.push_back(TreeEntry{"040000", kv.first, subHash});
        }
        treeHash = store_.writeTree(entries);
    }

    std::string Repository::buildTreeFromIndex() const
    {
        auto items = sortIndexEntries(index_.entries());
        std::string rootHash;
        writeTreeRecursive(items, 0, items.size(), "", rootHash);
        return rootHash;
    }

    std::optional<std::string> Repository::blobHashOfCommitPath(const std::string &commitHash, const std::string &relPath) const
    {
        std::string treeHash, parent, author, msg;
        long long ts = 0;
        if (!store_.readCommit(commitHash, treeHash, parent, author, ts, msg))
            return std::nullopt;
        std::string currentTree = treeHash;
        std::istringstream pathSS(relPath);
        std::string segment;
        std::vector<std::string> parts;
        while (std::getline(pathSS, segment, '/'))
            if (!segment.empty())
                parts.push_back(segment);
        for (size_t i = 0; i < parts.size(); i++)
        {
            std::vector<TreeEntry> entries;
            if (!store_.readTree(currentTree, entries))
                return std::nullopt;
            bool found = false;
            for (auto &e : entries)
            {
                if (e.name == parts[i])
                {
                    if (i + 1 == parts.size())
                    {
                        if (e.mode == "100644")
                            return e.hash;
                        else
                            return std::nullopt;
                    }
                    else
                    {
                        if (e.mode == "040000")
                        {
                            currentTree = e.hash;
                            found = true;
                            break;
                        }
                        else
                            return std::nullopt;
                    }
                }
            }
            if (!found && i + 1 < parts.size())
                return std::nullopt;
        }
        return std::nullopt;
    }

    std::optional<std::string> Repository::workingBlobHash(const fs::path &relPath) const
    {
        auto abs = root_ / relPath;
        if (!std::filesystem::exists(abs) || std::filesystem::is_directory(abs))
            return std::nullopt;
        std::string data;
        if (!fsops::readFile(abs, data))
            return std::nullopt;
        return store_.writeBlob(data);
    }

    std::optional<std::string> Repository::commit(const std::string &message, const std::string &author)
    {
        index_.load();
        auto treeHash = buildTreeFromIndex();
        auto parent = headCommit().value_or("");
        long long ts = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
        auto commitHash = store_.writeCommit(treeHash, parent, author, ts, message);
        auto ref = currentHeadRef();
        if (ref.empty())
            setHeadRef("refs/heads/main");
        updateRef(currentHeadRef(), commitHash);
        return commitHash;
    }

    bool Repository::materializeTree(const std::string &treeHash, const fs::path &dir) const
    {
        std::vector<TreeEntry> entries;
        if (!store_.readTree(treeHash, entries))
            return false;
        std::filesystem::create_directories(dir);
        for (auto &e : entries)
        {
            if (e.mode == "040000")
            {
                materializeTree(e.hash, dir / e.name);
            }
            else
            {
                std::string data;
                store_.readBlob(e.hash, data);
                fsops::writeFile(dir / e.name, data);
            }
        }
        return true;
    }

    bool Repository::checkout(const std::string &commitHash)
    {
        std::string treeHash, parent, author, msg;
        long long ts = 0;
        if (!store_.readCommit(commitHash, treeHash, parent, author, ts, msg))
            return false;
        for (auto &p : std::filesystem::directory_iterator(root_))
        {
            if (p.path().filename() == ".chronofs")
                continue;
            fsops::removePath(p.path());
        }
        return materializeTree(treeHash, root_);
    }

    std::vector<Repository::StatusEntry> Repository::status() const
    {
        std::vector<StatusEntry> out;
        index_.load();
        std::set<std::string> working;
        for (auto &p : std::filesystem::recursive_directory_iterator(root_))
        {
            if (p.is_directory())
            {
                if (p.path().filename() == ".chronofs")
                {
                    p.disable_recursion_pending();
                    continue;
                }
                continue;
            }
            auto rel = std::filesystem::relative(p.path(), root_).generic_string();
            working.insert(rel);
        }
        for (auto &rel : working)
        {
            auto whash = workingBlobHash(rel).value_or("");
            auto it = index_.entries().find(rel);
            if (it == index_.entries().end())
            {
                out.push_back({rel, "untracked"});
            }
            else if (it->second.hash != whash)
            {
                out.push_back({rel, "modified"});
            }
            else
            {
                out.push_back({rel, "staged"});
            }
        }
        for (auto &kv : index_.entries())
        {
            if (!working.count(kv.first))
                out.push_back({kv.first, "deleted"});
        }
        if (out.empty())
            out.push_back({"", "clean"});
        return out;
    }

    std::vector<std::string> Repository::log() const
    {
        std::vector<std::string> lines;
        auto head = headCommit();
        std::string cur = head.value_or("");
        while (!cur.empty())
        {
            std::string tree, parent, author, msg;
            long long ts = 0;
            if (!store_.readCommit(cur, tree, parent, author, ts, msg))
                break;
            std::ostringstream oss;
            oss << "commit " << cur << "\n"
                << "Author: " << author << "\n"
                << "Date:   " << ts << "\n\n"
                << "    " << msg << "\n";
            lines.push_back(oss.str());
            cur = parent;
        }
        if (lines.empty())
            lines.push_back("(no commits yet)\n");
        return lines;
    }

    std::string Repository::diff(const std::string &a, const std::string &b) const
    {
        auto readRefOrCommit = [&](const std::string &id) -> std::optional<std::string>
        {
            if (id == "HEAD")
            {
                auto hc = headCommit();
                if (!hc)
                    return std::nullopt;
                return *hc;
            }
            return id;
        };

        auto loadTreeFromCommit = [&](const std::string &commit, std::map<std::string, std::string> &pathToBlob)
        {
            std::string tree, parent, author, msg;
            long long ts = 0;
            if (!store_.readCommit(commit, tree, parent, author, ts, msg))
                return false;
            std::function<void(const std::string &, const std::string &)> walk = [&](const std::string &th, const std::string &prefix)
            {
                std::vector<TreeEntry> entries;
                if (!store_.readTree(th, entries))
                    return;
                for (auto &e : entries)
                {
                    if (e.mode == "040000")
                        walk(e.hash, prefix + e.name + "/");
                    else
                        pathToBlob[prefix + e.name] = e.hash;
                }
            };
            walk(tree, "");
            return true;
        };

        std::map<std::string, std::string> left, right;

        if (a == "WORKING")
        {
            for (auto &p : std::filesystem::recursive_directory_iterator(root_))
            {
                if (p.is_directory())
                {
                    if (p.path().filename() == ".chronofs")
                    {
                        p.disable_recursion_pending();
                        continue;
                    }
                    continue;
                }
                auto rel = std::filesystem::relative(p.path(), root_).generic_string();
                auto whash = workingBlobHash(rel);
                if (whash)
                    left[rel] = *whash;
            }
        }
        else if (a == "INDEX")
        {
            index_.load();
            for (auto &kv : index_.entries())
                left[kv.first] = kv.second.hash;
        }
        else
        {
            auto ca = readRefOrCommit(a);
            if (!ca)
                return "Left side not found\n";
            loadTreeFromCommit(*ca, left);
        }

        if (b == "WORKING")
        {
            for (auto &p : std::filesystem::recursive_directory_iterator(root_))
            {
                if (p.is_directory())
                {
                    if (p.path().filename() == ".chronofs")
                    {
                        p.disable_recursion_pending();
                        continue;
                    }
                    continue;
                }
                auto rel = std::filesystem::relative(p.path(), root_).generic_string();
                auto whash = workingBlobHash(rel);
                if (whash)
                    right[rel] = *whash;
            }
        }
        else if (b == "INDEX")
        {
            index_.load();
            for (auto &kv : index_.entries())
                right[kv.first] = kv.second.hash;
        }
        else
        {
            auto cb = readRefOrCommit(b);
            if (!cb)
                return "Right side not found\n";
            loadTreeFromCommit(*cb, right);
        }

        std::ostringstream out;
        std::set<std::string> all;
        for (auto &kv : left)
            all.insert(kv.first);
        for (auto &kv : right)
            all.insert(kv.first);

        for (auto &path : all)
        {
            auto itL = left.find(path), itR = right.find(path);
            if (itL == left.end())
            {
                out << "diff -- " << path << "\n";
                out << "--- a/" << path << "\n";
                out << "+++ b/" << path << "\n";
                std::string rb;
                store_.readBlob(itR->second, rb);
                auto h = diffText("", rb);
                for (auto &l : h)
                    out << l.tag << l.text << "\n";
            }
            else if (itR == right.end())
            {
                out << "diff -- " << path << "\n";
                out << "--- a/" << path << "\n";
                out << "+++ b/" << path << "\n";
                std::string lb;
                store_.readBlob(itL->second, lb);
                auto h = diffText(lb, "");
                for (auto &l : h)
                    out << l.tag << l.text << "\n";
            }
            else if (itL->second != itR->second)
            {
                out << "diff -- " << path << "\n";
                out << "--- a/" << path << "\n";
                out << "+++ b/" << path << "\n";
                std::string lb, rb;
                store_.readBlob(itL->second, lb);
                store_.readBlob(itR->second, rb);
                auto h = diffText(lb, rb);
                for (auto &l : h)
                    out << l.tag << l.text << "\n";
            }
        }
        std::string s = out.str();
        if (s.empty())
            s = "(no differences)\n";
        return s;
    }

    bool Repository::fsTouch(const fs::path &p) const { return fsops::touch(root_ / p); }
    bool Repository::fsMkdirs(const fs::path &p) const { return fsops::mkdirs(root_ / p); }
    bool Repository::fsRemove(const fs::path &p) const { return fsops::removePath(root_ / p); }
    bool Repository::fsMove(const fs::path &from, const fs::path &to) const { return fsops::movePath(root_ / from, root_ / to); }

} 
