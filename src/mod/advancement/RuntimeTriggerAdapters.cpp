#include "mod/advancement/RuntimeTriggerAdapters.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/MobDieEvent.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mod/MyMod.h"
#include "mod/advancement/TriggerDispatcher.h"

#include "mc/world/Container.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

#include <optional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace my_mod::advancement {
namespace {

ll::event::ListenerPtr gDestroyBlockListener;
ll::event::ListenerPtr gMobDieListener;
ll::event::ListenerPtr gPlayerDieListener;
MyMod*                  gRuntimeTriggerMod = nullptr;
std::unordered_map<uint64_t, std::string> gPendingBucketedEntities;

void logTriggerDispatch(MyMod& mod, TriggerContext const& context) {
    auto& logger = mod.getSelf().getLogger();
    std::visit(
        [&](auto const& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, BlockTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} block={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.blockId
                );
            } else if constexpr (std::is_same_v<Payload, ItemTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} item={} count={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.itemId,
                    payload.itemCount ? std::to_string(*payload.itemCount) : std::string{"-"}
                );
            } else if constexpr (std::is_same_v<Payload, EntityTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} entity={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.entityTypeId
                );
            } else if constexpr (std::is_same_v<Payload, DimensionTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} from={} to={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.fromDimension,
                    payload.toDimension
                );
            }
        },
        context.payload
    );
}

void dispatchTrigger(MyMod& mod, TriggerContext const& context) {
    auto worldDataDir = mod.getSelf().getWorldDataDir();
    if (!worldDataDir) {
        return;
    }

    logTriggerDispatch(mod, context);

    auto dispatcher = TriggerDispatcher{mod.getTriggerIndex(), mod.getProgressService()};
    dispatcher.dispatch(*worldDataDir, mod.getAdvancementLoadResult(), context);
}

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

void dispatchInventoryChangedForItem(MyMod& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:inventory_changed",
            ItemTriggerPayload{itemId, countMatchingItems(player, itemId)},
        }
    );
}

void dispatchConsumeItem(MyMod& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:consume_item",
            ItemTriggerPayload{itemId, std::nullopt},
        }
    );
}

void dispatchUsedTotem(MyMod& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:used_totem",
            ItemTriggerPayload{"minecraft:totem_of_undying", std::nullopt},
        }
    );
}

void dispatchFishingRodHooked(MyMod& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:fishing_rod_hooked",
            ItemTriggerPayload{itemId, std::nullopt},
        }
    );
}

void dispatchFilledBucket(MyMod& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:filled_bucket",
            ItemTriggerPayload{itemId, std::nullopt},
        }
    );
}

void dispatchSleptInBed(MyMod& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:slept_in_bed",
            NoTriggerPayload{},
        }
    );
}

std::string dimensionId(DimensionType dimension) {
    if (dimension == VanillaDimensions::Overworld()) {
        return "minecraft:overworld";
    }
    if (dimension == VanillaDimensions::Nether()) {
        return "minecraft:the_nether";
    }
    if (dimension == VanillaDimensions::TheEnd()) {
        return "minecraft:the_end";
    }
    return std::to_string(static_cast<int>(dimension));
}

void dispatchChangedDimension(MyMod& mod, Player& player, DimensionType fromDimension, DimensionType toDimension) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:changed_dimension",
            DimensionTriggerPayload{dimensionId(fromDimension), dimensionId(toDimension)},
        }
    );
}

MyMod* currentRuntimeTriggerMod() { return gRuntimeTriggerMod; }

std::optional<Player*> findKillingPlayer(ll::event::MobDieEvent& event) {
    auto const& source = event.source();
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* player = event.self().getLastHurtByPlayer();
    if (player == nullptr) {
        return std::nullopt;
    }
    return player;
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

std::optional<std::string> findKillerEntityTypeId(ActorDamageSource const& source) {
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* actor = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (actor == nullptr) {
        return std::nullopt;
    }
    if (source.isChildEntitySource()) {
        actor = actor->getOwner();
    }
    if (actor == nullptr) {
        return std::nullopt;
    }
    return actor->getTypeName();
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
    if (mod == nullptr || !consumeItem || itemId.empty()) {
        return;
    }

    dispatchConsumeItem(*mod, *this, itemId);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerFireDimensionChangedEventHook,
    HookPriority::Normal,
    Player,
    &Player::fireDimensionChangedEvent,
    void,
    DimensionType fromDimension,
    DimensionType toDimension
) {
    origin(fromDimension, toDimension);

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }

    if (fromDimension != toDimension) {
        dispatchChangedDimension(*mod, *this, fromDimension, toDimension);
    }
}

LL_TYPE_INSTANCE_HOOK(PlayerConsumeTotemHook, HookPriority::Normal, Player, &Player::$consumeTotem, bool) {
    auto const consumed = origin();
    if (!consumed) {
        return false;
    }

    auto* mod = currentRuntimeTriggerMod();
    if (mod != nullptr) {
        dispatchUsedTotem(*mod, *this);
    }

    return true;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerStartSleepInBedHook,
    HookPriority::Normal,
    Player,
    &Player::$startSleepInBed,
    BedSleepingResult,
    BlockPos const& pos
) {
    auto const result = origin(pos);
    if (result != BedSleepingResult::Ok) {
        return result;
    }

    auto* mod = currentRuntimeTriggerMod();
    if (mod != nullptr) {
        dispatchSleptInBed(*mod, *this);
    }

    return result;
}

LL_TYPE_INSTANCE_HOOK(
    PullFishingHookHook,
    HookPriority::Normal,
    FishingHook,
    &FishingHook::_pullCloser,
    void,
    Actor& inEntity,
    float  inSpeed
) {
    auto* mod = currentRuntimeTriggerMod();
    auto* player = getPlayerOwner();
    std::optional<std::string> itemId;
    if (inEntity.isType(ActorType::ItemEntity)) {
        auto const& item = static_cast<ItemActor&>(inEntity).item();
        if (!item.isNull()) {
            itemId = item.getTypeName();
        }
    }

    origin(inEntity, inSpeed);

    if (mod == nullptr || player == nullptr || !itemId) {
        return;
    }

    dispatchFishingRodHooked(*mod, *player, *itemId);
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
    auto const entityId = actor.getOrCreateUniqueID().getHash();
    auto result = origin(actor, location);
    if (!result.mSuccess) {
        return result;
    }

    auto pending = gPendingBucketedEntities.find(entityId);
    if (pending == gPendingBucketedEntities.end()) {
        return result;
    }

    auto* mod = currentRuntimeTriggerMod();
    if (mod != nullptr) {
        dispatchFilledBucket(*mod, *this, pending->second);
    }
    gPendingBucketedEntities.erase(pending);
    return result;
}

void touchPlayerInventoryChangedHookAutoCount() {
    (void)PlayerInventoryChangedHook::_AutoHookCount;
}

void touchPlayerUseItemHookAutoCount() { (void)PlayerUseItemHook::_AutoHookCount; }

void touchPlayerFireDimensionChangedEventHookAutoCount() {
    (void)PlayerFireDimensionChangedEventHook::_AutoHookCount;
}

void touchPlayerConsumeTotemHookAutoCount() { (void)PlayerConsumeTotemHook::_AutoHookCount; }

void touchPlayerStartSleepInBedHookAutoCount() { (void)PlayerStartSleepInBedHook::_AutoHookCount; }

void touchPullFishingHookHookAutoCount() { (void)PullFishingHookHook::_AutoHookCount; }

void touchBucketUseOnEntityHookAutoCount() { (void)BucketUseOnEntityHook::_AutoHookCount; }

void touchPlayerInteractEntityHookAutoCount() { (void)PlayerInteractEntityHook::_AutoHookCount; }

struct RuntimeTriggerHookState {
    ll::memory::HookRegistrar<PlayerInventoryChangedHook> inventoryChangedHook;
    ll::memory::HookRegistrar<PlayerUseItemHook>          useItemHook;
    ll::memory::HookRegistrar<PlayerFireDimensionChangedEventHook> dimensionChangedEventHook;
    ll::memory::HookRegistrar<PlayerConsumeTotemHook>               consumeTotemHook;
    ll::memory::HookRegistrar<PlayerStartSleepInBedHook>            startSleepInBedHook;
    ll::memory::HookRegistrar<PullFishingHookHook>                  pullFishingHook;
    ll::memory::HookRegistrar<BucketUseOnEntityHook>                bucketUseOnEntityHook;
    ll::memory::HookRegistrar<PlayerInteractEntityHook>             playerInteractEntityHook;
};

std::unique_ptr<RuntimeTriggerHookState> gRuntimeTriggerHookState;

} // namespace

void registerRuntimeTriggerAdapters(MyMod& mod) {
    if (gDestroyBlockListener || gMobDieListener || gPlayerDieListener || gRuntimeTriggerMod != nullptr
        || gRuntimeTriggerHookState) {
        return;
    }

    auto& eventBus = ll::event::EventBus::getInstance();
    gRuntimeTriggerMod = &mod;
    touchPlayerInventoryChangedHookAutoCount();
    touchPlayerUseItemHookAutoCount();
    touchPlayerFireDimensionChangedEventHookAutoCount();
    touchPlayerConsumeTotemHookAutoCount();
    touchPlayerStartSleepInBedHookAutoCount();
    touchPullFishingHookHookAutoCount();
    touchBucketUseOnEntityHookAutoCount();
    touchPlayerInteractEntityHookAutoCount();
    gRuntimeTriggerHookState = std::make_unique<RuntimeTriggerHookState>();

    gDestroyBlockListener = eventBus.emplaceListener<ll::event::PlayerDestroyBlockEvent>([&mod](auto& event) {
        auto const& block = event.self().getDimensionBlockSource().getBlock(event.pos());
        dispatchTrigger(
            mod,
            TriggerContext{
                event.self(),
                "bedrock:player_destroy_block",
                BlockTriggerPayload{block.getTypeName()},
            }
        );
        return true;
    });

    gMobDieListener = eventBus.emplaceListener<ll::event::MobDieEvent>([&mod](auto& event) {
        auto player = findKillingPlayer(event);
        if (!player) {
            return true;
        }

        dispatchTrigger(
            mod,
            TriggerContext{
                **player,
                "minecraft:player_killed_entity",
                EntityTriggerPayload{event.self().getTypeName()},
            }
        );
        return true;
    });

    gPlayerDieListener = eventBus.emplaceListener<ll::event::player::PlayerDieEvent>([&mod](auto& event) {
        auto const killerEntityTypeId = findKillerEntityTypeId(event.source());
        if (!killerEntityTypeId) {
            return true;
        }

        dispatchTrigger(
            mod,
            TriggerContext{
                event.self(),
                "minecraft:entity_killed_player",
                EntityTriggerPayload{*killerEntityTypeId},
            }
        );
        return true;
    });
}

void unregisterRuntimeTriggerAdapters() {
    gRuntimeTriggerMod = nullptr;
    gPendingBucketedEntities.clear();

    auto& eventBus = ll::event::EventBus::getInstance();
    gRuntimeTriggerHookState.reset();
    if (gDestroyBlockListener) {
        eventBus.removeListener(gDestroyBlockListener);
        gDestroyBlockListener.reset();
    }
    if (gMobDieListener) {
        eventBus.removeListener(gMobDieListener);
        gMobDieListener.reset();
    }
    if (gPlayerDieListener) {
        eventBus.removeListener(gPlayerDieListener);
        gPlayerDieListener.reset();
    }
}

} // namespace my_mod::advancement
