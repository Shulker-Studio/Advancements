#include "mod/commands/AdvancementsCommand.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/SoftEnum.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ServerCommandRegisterEvent.h"
#include "mod/MyMod.h"
#include "mod/advancement/ProgressStore.h"

#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandSelector.h"
#include "mc/world/actor/player/Player.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace my_mod::commands {

enum class AdvancementIds {};

using AdvancementIdParam = ll::command::SoftEnum<AdvancementIds>;

constexpr std::string_view AdvancementIdsEnumName = ll::command::enum_name_v<AdvancementIdParam>;

struct ProgressParams {
    CommandSelector<Player> target;
    AdvancementIdParam      advancement;
};

struct ProgressCriterionParams {
    CommandSelector<Player> target;
    AdvancementIdParam      advancement;
    std::string             criterion;
};

namespace {

bool gCommandRegistered = false;
std::vector<std::string> gPendingAdvancementIds;

void ensureAdvancementEnumRegistered() {
    auto& registrar = ll::command::CommandRegistrar::getServerInstance();
    registrar.tryRegisterSoftEnum(std::string{AdvancementIdsEnumName}, {});
}

std::vector<std::string> collectAdvancementIds(advancement::LoadResult const& result) {
    std::vector<std::string> ids;
    ids.reserve(result.advancements.size());
    for (auto const& [id, _] : result.advancements) {
        ids.emplace_back(id);
    }
    std::ranges::sort(ids);
    return ids;
}

void applyAdvancementEnumValues(std::vector<std::string> ids) {
    ensureAdvancementEnumRegistered();
    ll::command::CommandRegistrar::getServerInstance().setSoftEnumValues(std::string{AdvancementIdsEnumName}, std::move(ids));
}

void outputProgressResult(
    CommandOutput&                              output,
    advancement::ProgressMutationResult const& result,
    std::string_view                            action,
    Player const&                               player
) {
    if (!result.ok()) {
        for (auto const& error : result.errors) {
            output.error(error);
        }
        return;
    }
    if (result.changed) {
        output.success("{} {} progress. Advancement done: {}.", action, player.getRealName(), result.done);
        return;
    }
    output.success("{} progress already matched the requested state. Advancement done: {}.", player.getRealName(), result.done);
}

advancement::AdvancementDefinition const* findAdvancement(
    MyMod const&       mod,
    std::string const& advancementId,
    CommandOutput&     output
) {
    auto const& definitions = mod.getAdvancementLoadResult().advancements;
    auto const  found       = definitions.find(advancementId);
    if (found == definitions.end()) {
        output.error("Unknown advancement {}.", advancementId);
        return nullptr;
    }
    return &found->second;
}

std::optional<std::filesystem::path> getWorldDataDir(MyMod& mod, CommandOutput& output) {
    auto worldDataDir = mod.getSelf().getWorldDataDir();
    if (!worldDataDir) {
        output.error("World data directory is unavailable.");
    }
    return worldDataDir;
}

void executeProgressCommand(
    MyMod&                     mod,
    CommandOrigin const&       origin,
    CommandOutput&             output,
    CommandSelector<Player> const& target,
    std::string const&         advancementId,
    std::optional<std::string> const& criterion,
    bool                       grant
) {
    auto worldDataDir = getWorldDataDir(mod, output);
    if (!worldDataDir) {
        return;
    }

    auto const* advancement = findAdvancement(mod, advancementId, output);
    if (!advancement) {
        return;
    }

    auto players = target.results(origin);
    if (!players.size()) {
        output.error("No target players matched.");
        return;
    }

    for (auto* player : *players.data) {
        if (!player) {
            continue;
        }
        auto const& playerUuid = player->getUuid();
        auto result = grant ? advancement::grantProgress(*worldDataDir, playerUuid, *advancement, criterion)
                            : advancement::revokeProgress(*worldDataDir, playerUuid, *advancement, criterion);
        outputProgressResult(output, result, grant ? "Granted" : "Revoked", *player);
    }
}

} // namespace

void updateAdvancementCommandEnums(advancement::LoadResult const& result) {
    auto ids = collectAdvancementIds(result);
    if (!gCommandRegistered) {
        gPendingAdvancementIds = std::move(ids);
        return;
    }
    applyAdvancementEnumValues(std::move(ids));
}

void registerAdvancementsCommandNow(MyMod& mod) {
    gCommandRegistered = true;
    ensureAdvancementEnumRegistered();
    if (!gPendingAdvancementIds.empty()) {
        applyAdvancementEnumValues(std::move(gPendingAdvancementIds));
        gPendingAdvancementIds.clear();
    }

    auto& command = ll::command::CommandRegistrar::getServerInstance().getOrCreateCommand(
        "advancements",
        "Advancements management",
        CommandPermissionLevel::GameDirectors
    );
    command.overload().execute([&mod](CommandOrigin const&, CommandOutput& output) {
        auto const& result = mod.getAdvancementLoadResult();
        output.success("Loaded {} advancement definitions with {} errors.", result.loadedCount(), result.errorCount());
    });
    command.overload().text("reload").execute([&mod](CommandOrigin const&, CommandOutput& output) {
        mod.reloadAdvancements();
        auto const& result = mod.getAdvancementLoadResult();

        if (result.errorCount() == 0) {
            output.success("Reloaded {} advancement definitions with no errors.", result.loadedCount());
            return;
        }

        output.error("Reloaded {} advancement definitions with {} errors.", result.loadedCount(), result.errorCount());
    });
    command.overload<ProgressParams>()
        .text("grant")
        .required("target")
        .text("only")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            executeProgressCommand(mod, origin, output, params.target, params.advancement, std::nullopt, true);
        });
    command.overload<ProgressCriterionParams>()
        .text("grant")
        .required("target")
        .text("only")
        .required("advancement")
        .required("criterion")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressCriterionParams const& params) {
            executeProgressCommand(mod, origin, output, params.target, params.advancement, params.criterion, true);
        });
    command.overload<ProgressParams>()
        .text("revoke")
        .required("target")
        .text("only")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            executeProgressCommand(mod, origin, output, params.target, params.advancement, std::nullopt, false);
        });
    command.overload<ProgressCriterionParams>()
        .text("revoke")
        .required("target")
        .text("only")
        .required("advancement")
        .required("criterion")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressCriterionParams const& params) {
            executeProgressCommand(mod, origin, output, params.target, params.advancement, params.criterion, false);
        });
}

void registerAdvancementsCommand(MyMod& mod) {
    ll::event::EventBus::getInstance().emplaceListener<ll::event::ServerCommandRegisterEvent>([&mod](auto&) {
        registerAdvancementsCommandNow(mod);
        return true;
    });
}

} // namespace my_mod::commands
