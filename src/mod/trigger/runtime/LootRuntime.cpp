#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "mod/Entry.h"

#include "ll/api/memory/Hook.h"

#include "mc/util/LootTableUtils.h"
#include "mc/world/Container.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <string_view>

namespace advancements {
namespace {

constexpr std::array<std::string_view, 4> SupportedBastionLootTables{
    "minecraft:chests/bastion_bridge",
    "minecraft:chests/bastion_hoglin_stable",
    "minecraft:chests/bastion_other",
    "minecraft:chests/bastion_treasure",
};

std::string normalizeLootTableId(std::string_view lootTableId) {
    constexpr std::string_view BedrockPrefix = "loot_tables/";
    constexpr std::string_view JsonSuffix    = ".json";

    if (lootTableId.starts_with(BedrockPrefix) && lootTableId.ends_with(JsonSuffix)) {
        auto const name = lootTableId.substr(BedrockPrefix.size(), lootTableId.size() - BedrockPrefix.size() - JsonSuffix.size());
        return "minecraft:" + std::string{name};
    }

    return std::string{lootTableId};
}

void dispatchGeneratedContainerLoot(Entry& mod, Player& player, std::string const& lootTableId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:player_generates_container_loot",
            LootTablePayload{lootTableId},
        }
    );
}

bool isSupportedBastionLootTable(std::string_view lootTableId) {
    return std::ranges::find(SupportedBastionLootTables, lootTableId) != SupportedBastionLootTables.end();
}

LL_TYPE_STATIC_HOOK(
    FillContainerLootTableHook,
    HookPriority::Normal,
    Util::LootTableUtils,
    &Util::LootTableUtils::fillContainer,
    void,
    Level&             level,
    Container&         container,
    Random&            random,
    std::string const& tableName,
    DimensionType      dimensionId,
    Actor*             entity
) {
    origin(level, container, random, tableName, dimensionId, entity);

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }

    auto const normalizedLootTableId = normalizeLootTableId(tableName);

    mod->getSelf().getLogger().debug(
        "Advancements debug: fillContainer table={} normalized_table={} entity_type={} is_player={}",
        tableName,
        normalizedLootTableId,
        entity ? entity->getTypeName() : std::string{"<null>"},
        entity != nullptr && entity->isPlayer()
    );
    if (entity == nullptr || !entity->isPlayer()) {
        return;
    }

    if (!isSupportedBastionLootTable(normalizedLootTableId)) {
        return;
    }

    dispatchGeneratedContainerLoot(*mod, static_cast<Player&>(*entity), normalizedLootTableId);
}

struct LootRuntimeHookState {
    ll::memory::HookRegistrar<FillContainerLootTableHook> fillContainerLootTableHook;
};

std::unique_ptr<LootRuntimeHookState> gLootRuntimeHookState;

} // namespace

bool lootRuntimeRegistered() { return gLootRuntimeHookState != nullptr; }

void registerLootRuntime() {
    if (gLootRuntimeHookState) {
        return;
    }

    (void)FillContainerLootTableHook::_AutoHookCount;
    gLootRuntimeHookState = std::make_unique<LootRuntimeHookState>();
}

void unregisterLootRuntime() { gLootRuntimeHookState.reset(); }

} // namespace advancements
