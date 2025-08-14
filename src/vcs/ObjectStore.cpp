#include "vcs/ObjectStore.hpp"
#include "fs/FileOps.hpp"
#include "util/Sha256.hpp"
#include <sstream>

namespace vcs
{

    ObjectStore::ObjectStore(const fs::path &repoDir)
        : repoDir_(repoDir), objectsDir_(repoDir / ".chronofs" / "objects")
    {
        std::filesystem::create_directories(objectsDir_);
    }

    bool ObjectStore::writeObject(const std::string &content, std::string &outHash)
    {
        outHash = util::Sha256::hashHex(content);
        auto path = objectsDir_ / outHash;
        if (!std::filesystem::exists(path))
        {
            if (!fsops::writeFile(path, content))
                return false;
        }
        return true;
    }

    bool ObjectStore::readObject(const std::string &hash, std::string &out) const
    {
        auto path = objectsDir_ / hash;
        return fsops::readFile(path, out);
    }

    std::string ObjectStore::writeBlob(const std::string &data)
    {
        std::string content = "blob\n" + data;
        std::string h;
        writeObject(content, h);
        return h;
    }

    bool ObjectStore::readBlob(const std::string &hash, std::string &out) const
    {
        std::string content;
        if (!readObject(hash, content))
            return false;
        if (content.rfind("blob\n", 0) != 0)
            return false;
        out = content.substr(5);
        return true;
    }

    std::string ObjectStore::writeTree(const std::vector<TreeEntry> &entries)
    {
        std::ostringstream oss;
        oss << "tree\n";
        for (auto &e : entries)
        {
            oss << e.mode << ' ' << e.name << ' ' << e.hash << '\n';
        }
        std::string h;
        writeObject(oss.str(), h);
        return h;
    }

    bool ObjectStore::readTree(const std::string &hash, std::vector<TreeEntry> &out) const
    {
        std::string content;
        if (!readObject(hash, content))
            return false;
        if (content.rfind("tree\n", 0) != 0)
            return false;
        std::istringstream iss(content.substr(5));
        std::string mode, name, hashv;
        out.clear();
        while (iss >> mode >> name >> hashv)
        {
            out.push_back({mode, name, hashv});
        }
        return true;
    }

    std::string ObjectStore::writeCommit(const std::string &treeHash,
                                         const std::string &parentHash,
                                         const std::string &author,
                                         long long timestamp,
                                         const std::string &message)
    {
        std::ostringstream oss;
        oss << "commit\n";
        oss << "tree " << treeHash << "\n";
        if (!parentHash.empty())
            oss << "parent " << parentHash << "\n";
        oss << "author " << author << "\n";
        oss << "time " << timestamp << "\n";
        oss << "message\n"
            << message << "\n";
        std::string h;
        writeObject(oss.str(), h);
        return h;
    }

    bool ObjectStore::readCommit(const std::string &hash, std::string &treeHash,
                                 std::string &parentHash, std::string &author,
                                 long long &timestamp, std::string &message) const
    {
        std::string content;
        if (!readObject(hash, content))
            return false;
        if (content.rfind("commit\n", 0) != 0)
            return false;
        std::istringstream iss(content.substr(7));
        std::string line;
        treeHash.clear();
        parentHash.clear();
        author.clear();
        timestamp = 0;
        message.clear();
        while (std::getline(iss, line))
        {
            if (line.rfind("tree ", 0) == 0)
            {
                treeHash = line.substr(5);
            }
            else if (line.rfind("parent ", 0) == 0)
            {
                parentHash = line.substr(7);
            }
            else if (line.rfind("author ", 0) == 0)
            {
                author = line.substr(7);
            }
            else if (line.rfind("time ", 0) == 0)
            {
                timestamp = std::stoll(line.substr(5));
            }
            else if (line == "message")
            {
                std::ostringstream msg;
                while (std::getline(iss, line))
                    msg << line << '\n';
                message = msg.str();
                if (!message.empty() && message.back() == '\n')
                    message.pop_back();
                break;
            }
        }
        return !treeHash.empty();
    }

} 
