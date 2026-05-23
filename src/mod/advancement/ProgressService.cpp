#include "mod/advancement/ProgressService.h"

#include "mod/advancement/ProgressStore.h"

#include <format>
#include <optional>

namespace my_mod::advancement {

std::optional<ProgressLoadResult> ProgressService::ensureCached(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid
) const {
    auto const key = playerUuid.asString();
    if (mCache.contains(key)) {
        return std::nullopt;
    }

    auto loadResult = loadPlayerProgress(worldDataDir, playerUuid);
    if (!loadResult.ok()) {
        return loadResult;
    }

    mCache.emplace(key, CachedProgress{playerUuid, std::move(loadResult.progress), false});
    return std::nullopt;
}

ProgressMutationResult ProgressService::mutate(
    std::filesystem::path const&      worldDataDir,
    mce::UUID const&                  playerUuid,
    AdvancementDefinition const&      advancement,
    std::optional<std::string> const& criterion,
    bool                              grant
) const {
    if (auto loadResult = ensureCached(worldDataDir, playerUuid); loadResult) {
        return ProgressMutationResult{false, false, false, std::move(loadResult->errors)};
    }

    auto const key = playerUuid.asString();
    auto&      cachedProgress = mCache.at(key);
    auto       result = grant ? grantProgress(advancement, cachedProgress.progress, criterion)
                              : revokeProgress(advancement, cachedProgress.progress, criterion);
    if (result.changed) {
        cachedProgress.dirty = true;
    }
    return result;
}

ProgressLoadResult ProgressService::getProgress(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid
) const {
    if (auto loadResult = ensureCached(worldDataDir, playerUuid); loadResult) {
        return *std::move(loadResult);
    }

    ProgressLoadResult result;
    result.progress = mCache.at(playerUuid.asString()).progress;
    return result;
}

ProgressMutationResult ProgressService::grantAdvancement(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement
) const {
    return mutate(worldDataDir, playerUuid, advancement, std::nullopt, true);
}

ProgressMutationResult ProgressService::grantCriterion(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement,
    std::string const&           criterionName
) const {
    return mutate(worldDataDir, playerUuid, advancement, criterionName, true);
}

ProgressMutationResult ProgressService::revokeAdvancement(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement
) const {
    return mutate(worldDataDir, playerUuid, advancement, std::nullopt, false);
}

ProgressMutationResult ProgressService::revokeCriterion(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    AdvancementDefinition const& advancement,
    std::string const&           criterionName
) const {
    return mutate(worldDataDir, playerUuid, advancement, criterionName, false);
}

std::vector<std::string> ProgressService::flushAll(std::filesystem::path const& worldDataDir) const {
    std::vector<std::string> flushErrors;

    for (auto& [_, cachedProgress] : mCache) {
        if (!cachedProgress.dirty) {
            continue;
        }

        std::vector<std::string> errors;
        if (savePlayerProgress(worldDataDir, cachedProgress.playerUuid, cachedProgress.progress, errors)) {
            cachedProgress.dirty = false;
            continue;
        }

        auto const playerUuid = cachedProgress.playerUuid.asString();
        if (errors.empty()) {
            flushErrors.emplace_back(std::format("failed to flush progress for {}", playerUuid));
            continue;
        }
        for (auto const& error : errors) {
            flushErrors.emplace_back(std::format("failed to flush progress for {}: {}", playerUuid, error));
        }
    }

    return flushErrors;
}

} // namespace my_mod::advancement
