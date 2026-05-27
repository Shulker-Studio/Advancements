#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "mod/event/player/PlayerTickEvent.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/entity/components_json_legacy/TransformationComponent.h"
#include "mc/world/Container.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/monster/ZombieVillager.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/Level.h"

#include <mc/legacy/ActorUniqueID.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace advancements {
namespace {

constexpr int  CureZombieVillagerMaxTrackedTicks    = 20 * 60 * 6;
constexpr int  TemperateFrogVariant                 = 0;
constexpr int  ColdFrogVariant                      = 1;
constexpr int  WarmFrogVariant                      = 2;

std::unordered_map<uint64_t, std::string> gPendingBucketedEntities;

struct PendingZombieVillagerCure {
    ActorUniqueID zombieVillagerId;
    mce::UUID     playerId;
    int           ticksRemaining{CureZombieVillagerMaxTrackedTicks};
};

std::unordered_map<int64, PendingZombieVillagerCure> gPendingZombieVillagerCures;
ll::event::ListenerPtr                               gPlayerTickListener;

int countMatchingItems(Player const& player, std::string const& itemId) {
    auto const& inventory = player.getInventory();
    int         total     = 0;
    for (int slot = 0; slot < inventory.getContainerSize(); ++slot) {
        auto const& item = inventory.getItem(slot);
        if (item.isNull() || item.getTypeName() != itemId) {
            continue;
        }
        total += item.mCount;
    }
    return total;
}

void dispatchInventoryChangedForItem(Entry& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:inventory_changed",
            ItemTriggerPayload{itemId, countMatchingItems(player, itemId)},
        }
    );
}

void dispatchConsumeItem(Entry& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:consume_item",
            ItemTriggerPayload{itemId, std::nullopt},
        }
    );
}

bool isConsumeItemUseMethod(ItemUseMethod useMethod) {
    return useMethod == ItemUseMethod::Eat || useMethod == ItemUseMethod::Consume;
}

void dispatchFilledBucket(Entry& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:filled_bucket",
            ItemTriggerPayload{itemId, std::nullopt},
        }
    );
}

void dispatchCuredZombieVillager(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:cured_zombie_villager",
            NoTriggerPayload{},
        }
    );
}

void dispatchPlayerInteractedWithEntity(
    Entry&             mod,
    Player&            player,
    std::string const& itemId,
    std::string const& entityTypeId,
    std::string const& entityVariantId
) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:player_interacted_with_entity",
            PlayerInteractedWithEntityPayload{itemId, entityTypeId, entityVariantId},
        }
    );
}

std::optional<std::string> frogVariantIdForVariant(int variant) {
    switch (variant) {
    case TemperateFrogVariant:
        return std::string{"minecraft:temperate"};
    case ColdFrogVariant:
        return std::string{"minecraft:cold"};
    case WarmFrogVariant:
        return std::string{"minecraft:warm"};
    default:
        return std::nullopt;
    }
}

bool isZombieVillagerActorType(ActorType actorType) {
    return actorType == ActorType::ZombieVillager || actorType == ActorType::ZombieVillagerV2;
}

bool isZombieVillagerCureInteraction(Actor& actor) {
    if (!isZombieVillagerActorType(actor.getEntityTypeId())) {
        return false;
    }
    return static_cast<ZombieVillager&>(actor).villagerConversionTime > 0;
}

void trackZombieVillagerCure(Player& player, Actor& actor) {
    auto const zombieVillagerId = actor.getOrCreateUniqueID();
    gPendingZombieVillagerCures[zombieVillagerId.rawID] = PendingZombieVillagerCure{
        zombieVillagerId,
        player.getUuid(),
    };
}

void checkPendingZombieVillagerCures(Player& player) {
    for (auto it = gPendingZombieVillagerCures.begin(); it != gPendingZombieVillagerCures.end();) {
        auto& pending = it->second;
        if (pending.playerId != player.getUuid()) {
            ++it;
            continue;
        }

        --pending.ticksRemaining;
        if (pending.ticksRemaining <= 0) {
            it = gPendingZombieVillagerCures.erase(it);
        }
        else {
            ++it;
        }
    }
}

std::optional<std::string> bucketItemIdForBucketedEntity(ActorType actorType) {
    switch (actorType) {
    case ActorType::Fish:
        return "minecraft:cod_bucket";
    case ActorType::Salmon:
        return "minecraft:salmon_bucket";
    case ActorType::Tropicalfish:
        return "minecraft:tropical_fish_bucket";
    case ActorType::Pufferfish:
        return "minecraft:pufferfish_bucket";
    case ActorType::Tadpole:
        return "minecraft:tadpole_bucket";
    case ActorType::Axolotl:
        return "minecraft:axolotl_bucket";
    default:
        return std::nullopt;
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerInventoryChangedHook,
    HookPriority::Normal,
    Player,
    &Player::inventoryChanged,
    void,
    Container&       container,
    int              slot,
    ItemStack const& oldItem,
    ItemStack const& newItem,
    bool             forceBalanced
) {
    std::optional<std::string> const currentItemId =
        !newItem.isNull() ? std::optional<std::string>{newItem.getTypeName()} : std::nullopt;
    std::optional<std::string> const oldItemId = !oldItem.isNull() ? std::optional<std::string>{oldItem.getTypeName()} : std::nullopt;

    origin(container, slot, oldItem, newItem, forceBalanced);

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }

    if (currentItemId) {
        dispatchInventoryChangedForItem(*mod, *this, *currentItemId);
    }

    if (oldItemId && oldItemId != currentItemId) {
        dispatchInventoryChangedForItem(*mod, *this, *oldItemId);
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerUseItemHook,
    HookPriority::Normal,
    Player,
    &Player::$useItem,
    void,
    ItemStackBase& item,
    ItemUseMethod  useMethod,
    bool           consumeItem
) {
    auto const itemId = item.isNull() ? std::string{} : item.getTypeName();

    origin(item, useMethod, consumeItem);

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr || !consumeItem || itemId.empty() || !isConsumeItemUseMethod(useMethod)) {
        return;
    }

    dispatchConsumeItem(*mod, *this, itemId);
}

LL_TYPE_INSTANCE_HOOK(
    BucketUseOnEntityHook,
    HookPriority::Normal,
    BucketItem,
    &BucketItem::$_useOn,
    InteractionResult,
    ItemStack&  instance,
    Actor&      entity,
    BlockPos    pos,
    uchar       face,
    Vec3 const& clickPos
) {
    auto const bucketItemId = bucketItemIdForBucketedEntity(entity.getEntityTypeId());
    if (!bucketItemId) {
        return origin(instance, entity, pos, face, clickPos);
    }

    auto const entityId = entity.getOrCreateUniqueID().getHash();
    auto result = origin(instance, entity, pos, face, clickPos);
    if (result.mSwing) {
        gPendingBucketedEntities[entityId] = *bucketItemId;
    }
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerInteractEntityHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    InteractionResult,
    Actor&      actor,
    Vec3 const& location
) {
    auto const mayStartZombieVillagerCure = isZombieVillagerActorType(actor.getEntityTypeId())
                                         && !isZombieVillagerCureInteraction(actor)
                                         && !getSelectedItem().isNull()
                                         && getSelectedItem().getTypeName() == "minecraft:golden_apple";
    auto const selectedItemId = !getSelectedItem().isNull() ? std::optional<std::string>{getSelectedItem().getTypeName()}
                                                            : std::nullopt;
    auto const actorTypeName = actor.getTypeName();
    auto const actorWasLeashed = actor.isLeashed();
    auto const leashHolderBefore = actor.getLeashHolder();
    auto const frogVariantId = actorTypeName == "minecraft:frog" ? frogVariantIdForVariant(actor.getVariant())
                                                                  : std::nullopt;
    auto const entityId = actor.getOrCreateUniqueID().getHash();
    auto result = origin(actor, location);

    auto* mod = currentRuntimeTriggerMod();

    if (!result.mSuccess && !result.mSwing) {
        return result;
    }

    if (mayStartZombieVillagerCure) {
        trackZombieVillagerCure(*this, actor);
    }

    if (mod != nullptr && selectedItemId && *selectedItemId == "minecraft:lead" && actorTypeName == "minecraft:frog"
        && frogVariantId && !actorWasLeashed && actor.isLeashed()) {
        auto const leashHolderAfter = actor.getLeashHolder();
        if (leashHolderAfter != ActorUniqueID::INVALID_ID() && leashHolderAfter == getOrCreateUniqueID()
            && leashHolderBefore != leashHolderAfter) {
            dispatchPlayerInteractedWithEntity(*mod, *this, *selectedItemId, actorTypeName, *frogVariantId);
        }
    }

    auto pending = gPendingBucketedEntities.find(entityId);
    if (pending == gPendingBucketedEntities.end()) {
        return result;
    }

    if (mod != nullptr) {
        dispatchFilledBucket(*mod, *this, pending->second);
    }
    gPendingBucketedEntities.erase(pending);
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    ZombieVillagerMaintainOldDataHook,
    HookPriority::Normal,
    TransformationComponent,
    &TransformationComponent::maintainOldData,
    void,
    Actor&                           originalActor,
    Actor&                           transformed,
    TransformationDescription const& transformation,
    ActorUniqueID const&             ownerID,
    Level const&                     level
) {
    auto const originalType = originalActor.getTypeName();
    auto const transformedType = transformed.getTypeName();
    auto const originalRuntimeType = originalActor.getEntityTypeId();
    auto const originalUniqueId = originalActor.getOrCreateUniqueID().rawID;
    auto const maybeTrackedCure = isZombieVillagerActorType(originalRuntimeType)
                               ? gPendingZombieVillagerCures.find(originalUniqueId)
                               : gPendingZombieVillagerCures.end();

    auto* mod = currentRuntimeTriggerMod();
    origin(originalActor, transformed, transformation, ownerID, level);

    if (mod == nullptr || maybeTrackedCure == gPendingZombieVillagerCures.end()) {
        return;
    }

    auto const transformedRuntimeType = transformed.getEntityTypeId();
    if (transformedRuntimeType != ActorType::Villager && transformedRuntimeType != ActorType::VillagerV2) {
        return;
    }

    auto* player = level.getPlayer(maybeTrackedCure->second.playerId);
    if (player == nullptr) {
        return;
    }

    dispatchCuredZombieVillager(*mod, *player);
    gPendingZombieVillagerCures.erase(maybeTrackedCure);
}

struct InventoryRuntimeHookState {
    ll::memory::HookRegistrar<PlayerInventoryChangedHook> inventoryChangedHook;
    ll::memory::HookRegistrar<PlayerUseItemHook>          useItemHook;
    ll::memory::HookRegistrar<BucketUseOnEntityHook>      bucketUseOnEntityHook;
    ll::memory::HookRegistrar<PlayerInteractEntityHook>   playerInteractEntityHook;
    ll::memory::HookRegistrar<ZombieVillagerMaintainOldDataHook> zombieVillagerMaintainOldDataHook;
};

std::unique_ptr<InventoryRuntimeHookState> gInventoryRuntimeHookState;

} // namespace

bool inventoryRuntimeRegistered() { return gInventoryRuntimeHookState != nullptr; }

void registerInventoryRuntime() {
    if (gInventoryRuntimeHookState) {
        return;
    }

    (void)PlayerInventoryChangedHook::_AutoHookCount;
    (void)PlayerUseItemHook::_AutoHookCount;
    (void)BucketUseOnEntityHook::_AutoHookCount;
    (void)PlayerInteractEntityHook::_AutoHookCount;
    (void)ZombieVillagerMaintainOldDataHook::_AutoHookCount;
    gInventoryRuntimeHookState = std::make_unique<InventoryRuntimeHookState>();
    gPlayerTickListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerTickEvent>([](auto& event) {
        auto* mod = currentRuntimeTriggerMod();
        if (mod != nullptr && !gPendingZombieVillagerCures.empty()) {
            checkPendingZombieVillagerCures(event.self());
        }
    });
}

void unregisterInventoryRuntime() {
    gPendingBucketedEntities.clear();
    gPendingZombieVillagerCures.clear();
    gInventoryRuntimeHookState.reset();
    if (gPlayerTickListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerTickListener);
        gPlayerTickListener.reset();
    }
}

} // namespace advancements
