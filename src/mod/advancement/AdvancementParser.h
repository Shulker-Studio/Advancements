#pragma once

#include "mod/advancement/Advancement.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace my_mod::advancement {

struct ParseResult {
    std::optional<AdvancementDefinition> advancement;
    std::vector<std::string>             errors;
};

ParseResult parseAdvancement(nlohmann::json const& json, std::string id, std::filesystem::path sourcePath);

} // namespace my_mod::advancement
