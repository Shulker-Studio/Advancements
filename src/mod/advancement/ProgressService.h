#pragma once

#include "mod/advancement/Advancement.h"
#include "mod/advancement/Progress.h"

#include "mc/platform/UUID.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace my_mod::advancement {

class ProgressService {
public:
    [[nodiscard]] ProgressLoadResult getProgress(
        std::filesystem::path const& worldDataDir,
        mce::UUID const&             playerUuid
    ) const;

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

    [[nodiscard]] std::vector<std::string> flushAll(std::filesystem::path const& worldDataDir) const;

private:
    struct CachedProgress {
        mce::UUID      playerUuid;
        PlayerProgress progress;
        bool           dirty{false};
    };

    [[nodiscard]] std::optional<ProgressLoadResult> ensureCached(
        std::filesystem::path const& worldDataDir,
        mce::UUID const&             playerUuid
    ) const;

    [[nodiscard]] ProgressMutationResult mutate(
        std::filesystem::path const&      worldDataDir,
        mce::UUID const&                  playerUuid,
        AdvancementDefinition const&      advancement,
        std::optional<std::string> const& criterion,
        bool                              grant
    ) const;

    mutable std::map<std::string, CachedProgress> mCache;
};

} // namespace my_mod::advancement
