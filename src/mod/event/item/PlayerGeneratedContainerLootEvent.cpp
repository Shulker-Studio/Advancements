#include "mod/event/item/PlayerGeneratedContainerLootEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/util/LootTableUtils.h"
#include "mc/world/Container.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/Level.h"
#include "mc/deps/core/math/Vec3.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class BrushItem : public ::Item {
public:
    // NOLINTNEXTLINE(bugprone-virtual-near-miss) - local declaration for a generated Bedrock symbol.
    MCAPI ::InteractionResult
    $_useOn(::ItemStack& instance, ::Actor& entity, ::BlockPos pos, uchar face, ::Vec3 const& clickPos) const;
};

namespace advancements::event::item {
namespace {

thread_local Player* gCurrentBrushPlayer = nullptr;

class ScopedBrushPlayerContext {
public:
    explicit ScopedBrushPlayerContext(Player* player)
    : mPrevious(gCurrentBrushPlayer) {
        gCurrentBrushPlayer = player;
    }

    ~ScopedBrushPlayerContext() { gCurrentBrushPlayer = mPrevious; }

    ScopedBrushPlayerContext(ScopedBrushPlayerContext const&)            = delete;
    ScopedBrushPlayerContext& operator=(ScopedBrushPlayerContext const&) = delete;
    ScopedBrushPlayerContext(ScopedBrushPlayerContext&&)                 = delete;
    ScopedBrushPlayerContext& operator=(ScopedBrushPlayerContext&&)      = delete;

private:
    Player* mPrevious;
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

std::vector<std::string> generatedItemIds(Container const& container) {
    std::vector<std::string> itemIds;
    for (auto slot = 0; slot < container.getContainerSize(); ++slot) {
        auto const& item = container.getItem(slot);
        if (!item.isNull()) {
            itemIds.push_back(item.getTypeName());
        }
    }
    return itemIds;
}

void publishPlayerGeneratedContainerLoot(Player& player, std::string lootTableId, Container const& container) {
    PlayerGeneratedContainerLootEvent event{player, std::move(lootTableId), generatedItemIds(container)};
    ll::event::EventBus::getInstance().publish(event);
}

LL_TYPE_INSTANCE_HOOK(
    BrushItemUseOnLootContextHook,
    HookPriority::Normal,
    BrushItem,
    &BrushItem::$_useOn,
    InteractionResult,
    ItemStack&  instance,
    Actor&      entity,
    BlockPos    pos,
    uchar       face,
    Vec3 const& clickPos
) {
    ScopedBrushPlayerContext context{entity.isPlayer() ? static_cast<Player*>(&entity) : nullptr};
    return origin(instance, entity, pos, face, clickPos);
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

    auto const normalizedLootTableId = normalizeLootTableId(tableName);

    if (gCurrentBrushPlayer != nullptr) {
        publishPlayerGeneratedContainerLoot(*gCurrentBrushPlayer, normalizedLootTableId, container);
        return;
    }

    if (entity == nullptr || !entity->isPlayer()) {
        return;
    }

    publishPlayerGeneratedContainerLoot(static_cast<Player&>(*entity), normalizedLootTableId, container);
}

std::unique_ptr<ll::event::EmitterBase> playerGeneratedContainerLootEventEmitterFactory();

class PlayerGeneratedContainerLootEventEmitter
    : public ll::event::Emitter<playerGeneratedContainerLootEventEmitterFactory, PlayerGeneratedContainerLootEvent> {
    ll::memory::HookRegistrar<PlayerGeneratedContainerLootEventHook> playerGeneratedContainerLootHook;
    ll::memory::HookRegistrar<BrushItemUseOnLootContextHook>         brushItemUseOnLootContextHook;
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
    (void)BrushItemUseOnLootContextHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerGeneratedContainerLootEvent>(
        playerGeneratedContainerLootEventEmitterFactory
    );
    gPlayerGeneratedContainerLootEventSourceRegistered = true;
}

void unregisterPlayerGeneratedContainerLootEventSource() { gPlayerGeneratedContainerLootEventSourceRegistered = false; }

} // namespace advancements::event::item
