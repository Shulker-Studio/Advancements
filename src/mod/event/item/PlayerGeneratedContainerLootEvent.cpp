#include "mod/event/item/PlayerGeneratedContainerLootEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
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

namespace advancements::event::item {
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

bool isSupportedBastionLootTable(std::string_view lootTableId) {
    return std::ranges::find(SupportedBastionLootTables, lootTableId) != SupportedBastionLootTables.end();
}

LL_TYPE_STATIC_HOOK(
    PlayerGeneratedContainerLootEventHook,
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

    if (entity == nullptr || !entity->isPlayer()) {
        return;
    }

    auto const normalizedLootTableId = normalizeLootTableId(tableName);
    if (!isSupportedBastionLootTable(normalizedLootTableId)) {
        return;
    }

    PlayerGeneratedContainerLootEvent event{static_cast<Player&>(*entity), normalizedLootTableId};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerGeneratedContainerLootEventEmitterFactory();

class PlayerGeneratedContainerLootEventEmitter
    : public ll::event::Emitter<playerGeneratedContainerLootEventEmitterFactory, PlayerGeneratedContainerLootEvent> {
    ll::memory::HookRegistrar<PlayerGeneratedContainerLootEventHook> playerGeneratedContainerLootHook;
};

std::unique_ptr<ll::event::EmitterBase> playerGeneratedContainerLootEventEmitterFactory() {
    return std::make_unique<PlayerGeneratedContainerLootEventEmitter>();
}

bool gPlayerGeneratedContainerLootEventSourceRegistered = false;

} // namespace

bool playerGeneratedContainerLootEventSourceRegistered() { return gPlayerGeneratedContainerLootEventSourceRegistered; }

void registerPlayerGeneratedContainerLootEventSource() {
    if (playerGeneratedContainerLootEventSourceRegistered()) {
        return;
    }

    (void)PlayerGeneratedContainerLootEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerGeneratedContainerLootEvent>(
        playerGeneratedContainerLootEventEmitterFactory
    );
    gPlayerGeneratedContainerLootEventSourceRegistered = true;
}

void unregisterPlayerGeneratedContainerLootEventSource() { gPlayerGeneratedContainerLootEventSourceRegistered = false; }

} // namespace advancements::event::item
