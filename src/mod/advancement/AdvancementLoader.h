#pragma once

#include "mod/advancement/Advancement.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace my_mod::advancement {

struct LoadIssue {
    std::filesystem::path path;
    std::string           message;
};

struct LoadResult {
    std::map<std::string, AdvancementDefinition> advancements;
    std::vector<LoadIssue>                       issues;

    [[nodiscard]] size_t loadedCount() const { return advancements.size(); }
    [[nodiscard]] size_t errorCount() const { return issues.size(); }
};

LoadResult loadAdvancements(std::filesystem::path const& modDir);

} // namespace my_mod::advancement
