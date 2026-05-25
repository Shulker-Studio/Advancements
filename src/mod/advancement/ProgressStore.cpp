#include "mod/advancement/ProgressStore.h"

#include "ll/api/io/FileUtils.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <ctime>
#include <format>

namespace advancements {
namespace {

std::string currentTimestamp() {
    auto const now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);

    char buffer[32]{};
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %z", &localTime) == 0) {
        return "1970-01-01 00:00:00 +0000";
    }
    return buffer;
}

void addError(std::vector<std::string>& errors, std::string message) { errors.emplace_back(std::move(message)); }

ProgressLoadResult loadFromPath(std::filesystem::path const& filePath) {
    ProgressLoadResult result;
    if (!std::filesystem::exists(filePath)) {
        return result;
    }

    auto content = ll::file_utils::readFile(filePath);
    if (!content) {
        addError(result.errors, std::format("failed to read {}", filePath.string()));
        return result;
    }

    auto json = nlohmann::json::parse(*content, nullptr, false, true);
    if (json.is_discarded() || !json.is_object()) {
        addError(result.errors, std::format("invalid progress JSON {}", filePath.string()));
        return result;
    }

    if (json.contains("DataVersion")) {
        if (!json.at("DataVersion").is_number_integer()) {
            addError(result.errors, "DataVersion must be an integer");
        } else {
            result.progress.dataVersion = json.at("DataVersion").get<int>();
        }
    }

    for (auto const& [id, value] : json.items()) {
        if (id == "DataVersion") {
            continue;
        }
        if (!value.is_object()) {
            addError(result.errors, std::format("{} must be an object", id));
            continue;
        }

        AdvancementProgress progress;
        if (value.contains("done") && !value.at("done").is_boolean()) {
            addError(result.errors, std::format("{}.done must be a boolean", id));
            continue;
        }
        progress.done = value.contains("done") ? value.at("done").get<bool>() : true;
        if (value.contains("criteria")) {
            auto const& criteria = value.at("criteria");
            if (!criteria.is_object()) {
                addError(result.errors, std::format("{}.criteria must be an object", id));
                continue;
            }
            for (auto const& [criterion, timestamp] : criteria.items()) {
                if (!timestamp.is_string()) {
                    addError(result.errors, std::format("{}.criteria.{} must be a string", id, criterion));
                    continue;
                }
                progress.criteria.emplace(criterion, timestamp.get<std::string>());
            }
        }

        if (!progress.criteria.empty() || progress.done) {
            result.progress.advancements.emplace(id, std::move(progress));
        }
    }

    return result;
}

bool isDone(AdvancementDefinition const& advancement, AdvancementProgress const& progress) {
    if (advancement.requirements.empty()) {
        return std::ranges::all_of(advancement.criteria, [&](auto const& item) {
            return progress.criteria.contains(item.first);
        });
    }

    return std::ranges::all_of(advancement.requirements, [&](auto const& group) {
        return std::ranges::any_of(group, [&](auto const& criterion) { return progress.criteria.contains(criterion); });
    });
}

bool saveToPath(std::filesystem::path const& filePath, PlayerProgress const& progress, std::vector<std::string>& errors) {
    std::filesystem::create_directories(filePath.parent_path());

    nlohmann::json json = nlohmann::json::object();
    for (auto const& [id, advancement] : progress.advancements) {
        if (advancement.criteria.empty() && !advancement.done) {
            continue;
        }

        nlohmann::json criteria = nlohmann::json::object();
        for (auto const& [criterion, timestamp] : advancement.criteria) {
            criteria[criterion] = timestamp;
        }
        json[id] = {
            {"criteria", std::move(criteria)},
            {    "done", advancement.done}
        };
    }
    json["DataVersion"] = PlayerProgressDataVersion;

    if (!ll::file_utils::writeFile(filePath, json.dump(4))) {
        addError(errors, std::format("failed to write {}", filePath.string()));
        return false;
    }
    return true;
}

ProgressMutationResult mutateProgress(
    AdvancementDefinition const&      advancement,
    PlayerProgress&                   playerProgress,
    std::optional<std::string> const& criterion,
    bool                              grant
) {
    ProgressMutationResult result;
    if (criterion && !advancement.criteria.contains(*criterion)) {
        addError(result.errors, std::format("{} has no criterion {}", advancement.id, *criterion));
        return result;
    }

    if (grant) {
        auto& entry          = playerProgress.advancements[advancement.id];
        auto const timestamp = currentTimestamp();
        if (criterion) {
            result.changed = entry.criteria.emplace(*criterion, timestamp).second;
        } else {
            for (auto const& [name, _] : advancement.criteria) {
                result.changed = entry.criteria.emplace(name, timestamp).second || result.changed;
            }
        }
        auto const previousDone = entry.done;
        entry.done             = isDone(advancement, entry);
        result.changed         = result.changed || previousDone != entry.done;
        result.done            = entry.done;
        result.becameDone      = !previousDone && entry.done;
    } else {
        auto advancementProgress = playerProgress.advancements.find(advancement.id);
        if (advancementProgress == playerProgress.advancements.end()) {
            return result;
        }

        if (criterion) {
            auto& entry     = advancementProgress->second;
            result.changed  = entry.criteria.erase(*criterion) > 0;
            auto const previousDone = entry.done;
            entry.done      = isDone(advancement, entry);
            result.changed  = result.changed || previousDone != entry.done;
            result.done     = entry.done;
            if (entry.criteria.empty() && !entry.done) {
                playerProgress.advancements.erase(advancement.id);
            }
        } else {
            result.changed = playerProgress.advancements.erase(advancement.id) > 0;
            result.done    = false;
        }
    }

    return result;
}

} // namespace

std::filesystem::path progressFilePath(std::filesystem::path const& worldDataDir, mce::UUID const& playerUuid) {
    return worldDataDir / "advancements" / std::format("{}.json", playerUuid.asString());
}

ProgressLoadResult loadPlayerProgress(std::filesystem::path const& worldDataDir, mce::UUID const& playerUuid) {
    return loadFromPath(progressFilePath(worldDataDir, playerUuid));
}

bool savePlayerProgress(
    std::filesystem::path const& worldDataDir,
    mce::UUID const&             playerUuid,
    PlayerProgress const&        progress,
    std::vector<std::string>&    errors
) {
    return saveToPath(progressFilePath(worldDataDir, playerUuid), progress, errors);
}

ProgressMutationResult grantProgress(
    AdvancementDefinition const&      advancement,
    PlayerProgress&                   playerProgress,
    std::optional<std::string> const& criterion
) {
    return mutateProgress(advancement, playerProgress, criterion, true);
}

ProgressMutationResult revokeProgress(
    AdvancementDefinition const&      advancement,
    PlayerProgress&                   playerProgress,
    std::optional<std::string> const& criterion
) {
    return mutateProgress(advancement, playerProgress, criterion, false);
}

} // namespace advancements
