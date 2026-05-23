#pragma once

#include "mod/advancement/Advancement.h"
#include "mod/advancement/Progress.h"

#include "mc/platform/UUID.h"

#include <filesystem>
#include <optional>
#include <string>

namespace my_mod::advancement {

[[nodiscard]] std::filesystem::path progressFilePath(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid
);

[[nodiscard]] ProgressLoadResult loadPlayerProgress(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid
);

[[nodiscard]] bool savePlayerProgress(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    PlayerProgress const&        progress,
    std::vector<std::string>&    errors
);

[[nodiscard]] ProgressMutationResult grantProgress(
    AdvancementDefinition const&          advancement,
    PlayerProgress&                       playerProgress,
    std::optional<std::string> const&     criterion
);

[[nodiscard]] ProgressMutationResult revokeProgress(
    AdvancementDefinition const&          advancement,
    PlayerProgress&                       playerProgress,
    std::optional<std::string> const&     criterion
);

} // namespace my_mod::advancement
