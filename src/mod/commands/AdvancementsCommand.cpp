#include "mod/commands/AdvancementsCommand.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/SoftEnum.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/command/ServerCommandRegisterEvent.h"
#include "mod/Entry.h"
#include "mod/advancement/AdvancementNotifier.h"
#include "mod/gui/AdvancementGui.h"

#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandOriginType.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandSelector.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace advancements::commands {

enum class AdvancementIds {};
enum class CriterionNames {};

using AdvancementIdParam = ll::command::SoftEnum<AdvancementIds>;
using CriterionParam     = ll::command::SoftEnum<CriterionNames>;

constexpr std::string_view AdvancementIdsEnumName = ll::command::enum_name_v<AdvancementIdParam>;
constexpr std::string_view CriterionNamesEnumName = ll::command::enum_name_v<CriterionParam>;

struct ProgressEverythingParams {
    CommandSelector<Player> target;
};

struct ProgressParams {
    CommandSelector<Player> target;
    AdvancementIdParam      advancement;
};

struct ProgressCriterionParams {
    CommandSelector<Player> target;
    AdvancementIdParam      advancement;
    CriterionParam          criterion;
};

namespace {

bool gCommandRegistered = false;
std::vector<std::string> gPendingAdvancementIds;
std::vector<std::string> gPendingCriterionNames;
ll::event::ListenerPtr gCommandRegisterListener;

bool ensureAdminPermission(CommandOrigin const& origin, CommandOutput& output) {
    if (origin.getPermissionsLevel() <= CommandPermissionLevel::Any) {
        output.error("You do not have permission to use this advancement command.");
        return false;
    }
    return true;
}

void ensureAdvancementEnumRegistered() {
    auto& registrar = ll::command::CommandRegistrar::getServerInstance();
    registrar.tryRegisterSoftEnum(std::string{AdvancementIdsEnumName}, {});
}

void ensureCriterionEnumRegistered() {
    auto& registrar = ll::command::CommandRegistrar::getServerInstance();
    registrar.tryRegisterSoftEnum(std::string{CriterionNamesEnumName}, {});
}

void applyAdvancementEnumValues(std::vector<std::string> ids) {
    ensureAdvancementEnumRegistered();
    ll::command::CommandRegistrar::getServerInstance().setSoftEnumValues(std::string{AdvancementIdsEnumName}, std::move(ids));
}

void applyCriterionEnumValues(std::vector<std::string> names) {
    ensureCriterionEnumRegistered();
    ll::command::CommandRegistrar::getServerInstance().setSoftEnumValues(std::string{CriterionNamesEnumName}, std::move(names));
}

void outputProgressResult(
    CommandOutput&                              output,
    ProgressMutationResult const& result,
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

AdvancementDefinition const* findAdvancement(
    Entry const&       mod,
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

std::optional<std::filesystem::path> getWorldDataDir(Entry& mod, CommandOutput& output) {
    auto worldDataDir = mod.getSelf().getWorldDataDir();
    if (!worldDataDir) {
        output.error("World data directory is unavailable.");
    }
    return worldDataDir;
}

std::vector<AdvancementDefinition const*> collectEverything(Entry const& mod) {
    std::vector<AdvancementDefinition const*> advancements;
    auto const& definitions = mod.getAdvancementLoadResult().advancements;
    advancements.reserve(definitions.size());
    for (auto const& entry : definitions) {
        advancements.emplace_back(&entry.second);
    }
    return advancements;
}

bool isDescendantOf(
    std::map<std::string, AdvancementDefinition> const& definitions,
    AdvancementDefinition const&                       advancement,
    std::string const&                                               ancestorId
) {
    if (advancement.id == ancestorId) {
        return true;
    }

    std::set<std::string> visited;
    auto                  parent = advancement.parent;
    while (parent) {
        if (*parent == ancestorId) {
            return true;
        }
        if (!visited.emplace(*parent).second) {
            return false;
        }

        auto const found = definitions.find(*parent);
        if (found == definitions.end()) {
            return false;
        }
        parent = found->second.parent;
    }
    return false;
}

std::vector<AdvancementDefinition const*> collectFrom(Entry const& mod, std::string const& advancementId) {
    std::vector<AdvancementDefinition const*> advancements;
    auto const& definitions = mod.getAdvancementLoadResult().advancements;
    for (auto const& entry : definitions) {
        if (isDescendantOf(definitions, entry.second, advancementId)) {
            advancements.emplace_back(&entry.second);
        }
    }
    return advancements;
}

std::vector<AdvancementDefinition const*> collectUntil(Entry const& mod, std::string const& advancementId) {
    std::vector<AdvancementDefinition const*> advancements;
    auto const& definitions = mod.getAdvancementLoadResult().advancements;
    auto        currentId   = advancementId;
    std::set<std::string> visited;
    while (visited.emplace(currentId).second) {
        auto const found = definitions.find(currentId);
        if (found == definitions.end()) {
            break;
        }
        advancements.emplace_back(&found->second);
        if (!found->second.parent) {
            break;
        }
        currentId = *found->second.parent;
    }
    std::ranges::reverse(advancements);
    return advancements;
}

std::vector<AdvancementDefinition const*> collectThrough(Entry const& mod, std::string const& advancementId) {
    auto advancements = collectUntil(mod, advancementId);
    std::set<std::string> included;
    for (auto const* advancement : advancements) {
        included.emplace(advancement->id);
    }

    for (auto const* advancement : collectFrom(mod, advancementId)) {
        if (included.emplace(advancement->id).second) {
            advancements.emplace_back(advancement);
        }
    }
    return advancements;
}

void executeProgressCommand(
    Entry&                                                        mod,
    CommandOrigin const&                                          origin,
    CommandOutput&                                                output,
    CommandSelector<Player> const&                                target,
std::vector<AdvancementDefinition const*> const& advancements,
    std::optional<std::string> const&                             criterion,
    bool                                                          grant
) {
    auto worldDataDir = getWorldDataDir(mod, output);
    if (!worldDataDir) {
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
        auto&       progressService = mod.getProgressService();
        for (auto const* advancement : advancements) {
            auto result = grant
                        ? (criterion ? progressService.grantCriterion(*worldDataDir, playerUuid, *advancement, *criterion)
                                     : progressService.grantAdvancement(*worldDataDir, playerUuid, *advancement))
                        : (criterion ? progressService.revokeCriterion(*worldDataDir, playerUuid, *advancement, *criterion)
                                     : progressService.revokeAdvancement(*worldDataDir, playerUuid, *advancement));
            outputProgressResult(output, result, grant ? "Granted" : "Revoked", *player);
            if (grant && result.becameDone) {
                notifyAdvancementCompleted(mod, *player, *advancement);
            }
        }
    }
}

void executeProgressCommand(
    Entry&                         mod,
    CommandOrigin const&           origin,
    CommandOutput&                 output,
    CommandSelector<Player> const& target,
    std::string const&             advancementId,
    std::optional<std::string> const& criterion,
    bool                           grant
) {
    auto const* advancement = findAdvancement(mod, advancementId, output);
    if (!advancement) {
        return;
    }
    executeProgressCommand(mod, origin, output, target, std::vector{advancement}, criterion, grant);
}

void executeGuiCommand(Entry& mod, CommandOrigin const& origin, CommandOutput& output) {
    auto& logger = mod.getSelf().getLogger();
    logger.debug(
        "Advancements debug: executeGuiCommand entered origin_type={} has_entity={}",
        static_cast<int>(origin.getOriginType()),
        origin.getEntity() != nullptr
    );
    if (origin.getOriginType() != CommandOriginType::Player) {
        logger.debug(
            "Advancements debug: executeGuiCommand rejected non-player origin origin_type={}",
            static_cast<int>(origin.getOriginType())
        );
        output.error("The advancement GUI command can only be used by a player.");
        return;
    }

    auto* actor = origin.getEntity();
    logger.debug(
        "Advancements debug: executeGuiCommand actor entity_exists={} entity_is_player={} entity_type={}",
        actor != nullptr,
        actor != nullptr && actor->isPlayer(),
        actor ? actor->getTypeName() : std::string{"<null>"}
    );
    if (!actor || !actor->isPlayer()) {
        logger.debug("Advancements debug: executeGuiCommand rejected actor before player resolution");
        output.error("The advancement GUI command can only be used by a player.");
        return;
    }

    auto* player = Player::tryGetFromEntity(actor->getEntityContext(), false);
    logger.debug(
        "Advancements debug: executeGuiCommand player resolution resolved={} player_name={}",
        player != nullptr,
        player ? player->getRealName() : std::string{"<none>"}
    );
    if (!player) {
        logger.debug("Advancements debug: executeGuiCommand rejected unresolved player");
        output.error("Unable to resolve the command player.");
        return;
    }

    logger.debug(
        "Advancements debug: executeGuiCommand opening GUI player={}",
        player->getRealName()
    );
    showAdvancementsGui(mod, *player);
}

} // namespace

void updateAdvancementCommandEnums(AdvancementCommandIndex const& index) {
    auto ids   = index.advancementIds;
    auto names = index.criterionNames;
    if (!gCommandRegistered) {
        gPendingAdvancementIds = std::move(ids);
        gPendingCriterionNames = std::move(names);
        return;
    }
    applyAdvancementEnumValues(std::move(ids));
    applyCriterionEnumValues(std::move(names));
}

void registerAdvancementsCommandNow(Entry& mod) {
    gCommandRegistered = true;
    auto& logger = mod.getSelf().getLogger();
    ensureAdvancementEnumRegistered();
    ensureCriterionEnumRegistered();
    if (!gPendingAdvancementIds.empty()) {
        applyAdvancementEnumValues(std::move(gPendingAdvancementIds));
        gPendingAdvancementIds.clear();
    }
    if (!gPendingCriterionNames.empty()) {
        applyCriterionEnumValues(std::move(gPendingCriterionNames));
        gPendingCriterionNames.clear();
    }

    auto& command = ll::command::CommandRegistrar::getServerInstance().getOrCreateCommand(
        "advancement",
        "Advancement management",
        CommandPermissionLevel::Any
    );
    logger.debug(
        "Advancements debug: registered command literal=advancement permission={}",
        static_cast<int>(CommandPermissionLevel::Any)
    );
    command.overload().execute([&mod](CommandOrigin const&, CommandOutput& output) {
        auto const& result = mod.getAdvancementLoadResult();
        output.success("Loaded {} advancement definitions with {} errors.", result.loadedCount(), result.errorCount());
    });
    command.overload().text("reload").execute([&mod](CommandOrigin const& origin, CommandOutput& output) {
        if (!ensureAdminPermission(origin, output)) {
            return;
        }
        mod.reloadAdvancements();
        auto const& result = mod.getAdvancementLoadResult();

        if (result.errorCount() == 0) {
            output.success("Reloaded {} advancement definitions with no errors.", result.loadedCount());
            return;
        }

        output.error("Reloaded {} advancement definitions with {} errors.", result.loadedCount(), result.errorCount());
    });
    command.overload().text("gui").execute([&mod](CommandOrigin const& origin, CommandOutput& output) {
        executeGuiCommand(mod, origin, output);
    });

    command.overload<ProgressEverythingParams>()
        .text("grant")
        .required("target")
        .text("everything")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressEverythingParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectEverything(mod), std::nullopt, true);
        });
    command.overload<ProgressParams>()
        .text("grant")
        .required("target")
        .text("from")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            if (!findAdvancement(mod, params.advancement, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectFrom(mod, params.advancement), std::nullopt, true);
        });
    command.overload<ProgressParams>()
        .text("grant")
        .required("target")
        .text("through")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            if (!findAdvancement(mod, params.advancement, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectThrough(mod, params.advancement), std::nullopt, true);
        });
    command.overload<ProgressParams>()
        .text("grant")
        .required("target")
        .text("until")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            if (!findAdvancement(mod, params.advancement, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectUntil(mod, params.advancement), std::nullopt, true);
        });
    command.overload<ProgressParams>()
        .text("grant")
        .required("target")
        .text("only")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, params.advancement, std::nullopt, true);
        });
    command.overload<ProgressCriterionParams>()
        .text("grant")
        .required("target")
        .text("only")
        .required("advancement")
        .required("criterion")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressCriterionParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, params.advancement, params.criterion, true);
        });
    command.overload<ProgressEverythingParams>()
        .text("revoke")
        .required("target")
        .text("everything")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressEverythingParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectEverything(mod), std::nullopt, false);
        });
    command.overload<ProgressParams>()
        .text("revoke")
        .required("target")
        .text("from")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            if (!findAdvancement(mod, params.advancement, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectFrom(mod, params.advancement), std::nullopt, false);
        });
    command.overload<ProgressParams>()
        .text("revoke")
        .required("target")
        .text("through")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            if (!findAdvancement(mod, params.advancement, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectThrough(mod, params.advancement), std::nullopt, false);
        });
    command.overload<ProgressParams>()
        .text("revoke")
        .required("target")
        .text("until")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            if (!findAdvancement(mod, params.advancement, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, collectUntil(mod, params.advancement), std::nullopt, false);
        });
    command.overload<ProgressParams>()
        .text("revoke")
        .required("target")
        .text("only")
        .required("advancement")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, params.advancement, std::nullopt, false);
        });
    command.overload<ProgressCriterionParams>()
        .text("revoke")
        .required("target")
        .text("only")
        .required("advancement")
        .required("criterion")
        .execute([&mod](CommandOrigin const& origin, CommandOutput& output, ProgressCriterionParams const& params) {
            if (!ensureAdminPermission(origin, output)) {
                return;
            }
            executeProgressCommand(mod, origin, output, params.target, params.advancement, params.criterion, false);
        });
}

void registerAdvancementsCommand(Entry& mod) {
    if (gCommandRegisterListener) {
        mod.getSelf().getLogger().debug("Advancements debug: registerAdvancementsCommand skipped existing listener");
        return;
    }

    mod.getSelf().getLogger().debug("Advancements debug: registerAdvancementsCommand installing listener");
    gCommandRegisterListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::ServerCommandRegisterEvent>([&mod](auto&) {
        mod.getSelf().getLogger().debug("Advancements debug: ServerCommandRegisterEvent received for command registration");
        registerAdvancementsCommandNow(mod);
        return true;
    });
}

void unregisterAdvancementsCommand() {
    auto& eventBus = ll::event::EventBus::getInstance();
    if (gCommandRegisterListener) {
        eventBus.removeListener(gCommandRegisterListener);
        gCommandRegisterListener.reset();
    }
}

} // namespace advancements::commands
