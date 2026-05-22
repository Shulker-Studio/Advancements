#pragma once

#include "mod/advancement/Advancement.h"
#include "mod/advancement/Progress.h"

#include "mc/platform/UUID.h"

#include <filesystem>
#include <string>

namespace my_mod::advancement {

class ProgressService {
public:
    [[nodiscard]] ProgressMutationResult grantAdvancement(
        std::filesystem::path const& worldDataDir,
        mce::UUID const&             playerUuid,
        AdvancementDefinition const& advancement
    ) const;

    [[nodiscard]] ProgressMutationResult grantCriterion(
        std::filesystem::path const& worldDataDir,
        mce::UUID const&             playerUuid,
        AdvancementDefinition const& advancement,
        std::string const&           criterionName
    ) const;

    [[nodiscard]] ProgressMutationResult revokeAdvancement(
        std::filesystem::path const& worldDataDir,
        mce::UUID const&             playerUuid,
        AdvancementDefinition const& advancement
    ) const;

    [[nodiscard]] ProgressMutationResult revokeCriterion(
        std::filesystem::path const& worldDataDir,
        mce::UUID const&             playerUuid,
        AdvancementDefinition const& advancement,
        std::string const&           criterionName
    ) const;
};

} // namespace my_mod::advancement
