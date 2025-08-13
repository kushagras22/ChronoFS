#pragma once
#include <bits/stdc++.h>

namespace vcs {
namespace fs = std::filesystem;

struct TreeEnrty {
    std::string mode;
    std::string name;
    std::string hash;
};

class ObjectStore {
    public:
        explicit ObjectStore(const fs::path& repoDir);
}