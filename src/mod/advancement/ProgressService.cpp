#include "mod/advancement/ProgressService.h"

#include "mod/advancement/ProgressStore.h"

#include <optional>

namespace my_mod::advancement {

ProgressMutationResult ProgressService::grantAdvancement(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement
) const {
    return grantProgress(worldDataDir, playerUuid, advancement, std::nullopt);
}

ProgressMutationResult ProgressService::grantCriterion(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement,
    std::string const&           criterionName
) const {
    return grantProgress(worldDataDir, playerUuid, advancement, criterionName);
}

ProgressMutationResult ProgressService::revokeAdvancement(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement
) const {
    return revokeProgress(worldDataDir, playerUuid, advancement, std::nullopt);
}

ProgressMutationResult ProgressService::revokeCriterion(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement,
    std::string const&           criterionName
) const {
    return revokeProgress(worldDataDir, playerUuid, advancement, criterionName);
}

} // namespace my_mod::advancement
