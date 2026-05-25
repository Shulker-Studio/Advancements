#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/memory/Hook.h"

#include "mc/deps/shared_types/legacy/ContainerType.h"
#include "mc/world/Container.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/FullContainerName.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/inventory/network/ContainerScreenContext.h"
#include "mc/world/inventory/network/ItemStackNetManagerServer.h"
#include "mc/world/inventory/network/ItemStackNetResult.h"
#include "mc/world/inventory/network/ItemStackRequestActionHandler.h"
#include "mc/world/inventory/network/ItemStackRequestActionTransferBase.h"
#include "mc/world/inventory/network/ItemStackRequestSlotInfo.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/BlockPos.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace advancements {
namespace {

constexpr auto SuccessfulOutputContainer = ContainerEnumName::CreatedOutputContainer;

std::unordered_map<uint64_t, std::string> gPendingBucketedEntities;

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
            "bedrock:inventory_changed",
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

void dispatchUsedTotem(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:used_totem",
            NoTriggerPayload{},
        }
    );
}

void dispatchFishingRodHooked(Entry& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:fishing_rod_hooked",
            ItemTriggerPayload{itemId, std::nullopt},
        }
    );
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

void dispatchVillagerTrade(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:villager_trade",
            NoTriggerPayload{},
        }
    );
}

void dispatchEnchantedItem(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:enchanted_item",
            NoTriggerPayload{},
        }
    );
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
    VillagerTradeRemoveHook,
    HookPriority::Normal,
    ItemStackRequestActionHandler,
    &ItemStackRequestActionHandler::_handleRemove,
    ::ItemStackNetResult,
    ::ItemStackRequestActionTransferBase const& requestAction,
    ::ItemStack&                                removedItem,
    ::ItemStackRequestActionHandler::RemoveType removeType
) {
    return origin(requestAction, removedItem, removeType);
}

LL_TYPE_INSTANCE_HOOK(
    VillagerTradeTransferHook,
    HookPriority::Normal,
    ItemStackRequestActionHandler,
    &ItemStackRequestActionHandler::_handleTransfer,
    ::ItemStackNetResult,
    ::ItemStackRequestActionTransferBase const& requestAction,
    bool                                        isSrcHintSlot,
    bool                                        isDstHintSlot,
    bool                                        isSwap
) {
    auto const& sourceSlot = *requestAction.mSrc;
    bool const isSuccessfulOutputTransfer = sourceSlot.mFullContainerName.mName == SuccessfulOutputContainer;
    auto const screenType                 = mItemStackNetManager.getScreenContext().mScreenContainerType;

    auto const result = origin(requestAction, isSrcHintSlot, isDstHintSlot, isSwap);
    auto*      mod    = currentRuntimeTriggerMod();
    if (result != ItemStackNetResult::Success || !isSuccessfulOutputTransfer || mod == nullptr) {
        return result;
    }

    if (screenType == SharedTypes::Legacy::ContainerType::Trade) {
        dispatchVillagerTrade(*mod, mPlayer);
    }

    if (screenType == SharedTypes::Legacy::ContainerType::Enchantment) {
        dispatchEnchantedItem(*mod, mPlayer);
    }

    return result;
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

struct InventoryRuntimeHookState {
    ll::memory::HookRegistrar<VillagerTradeTransferHook> villagerTradeTransferHook;
    ll::memory::HookRegistrar<PlayerInventoryChangedHook> inventoryChangedHook;
    ll::memory::HookRegistrar<PlayerUseItemHook>          useItemHook;
    ll::memory::HookRegistrar<PlayerConsumeTotemHook>     consumeTotemHook;
    ll::memory::HookRegistrar<PullFishingHookHook>        pullFishingHook;
    ll::memory::HookRegistrar<BucketUseOnEntityHook>      bucketUseOnEntityHook;
    ll::memory::HookRegistrar<PlayerInteractEntityHook>   playerInteractEntityHook;
};

std::unique_ptr<InventoryRuntimeHookState> gInventoryRuntimeHookState;

} // namespace

bool inventoryRuntimeRegistered() { return gInventoryRuntimeHookState != nullptr; }

void registerInventoryRuntime() {
    if (gInventoryRuntimeHookState) {
        return;
    }

    (void)VillagerTradeRemoveHook::_AutoHookCount;
    (void)VillagerTradeTransferHook::_AutoHookCount;
    (void)PlayerInventoryChangedHook::_AutoHookCount;
    (void)PlayerUseItemHook::_AutoHookCount;
    (void)PlayerConsumeTotemHook::_AutoHookCount;
    (void)PullFishingHookHook::_AutoHookCount;
    (void)BucketUseOnEntityHook::_AutoHookCount;
    (void)PlayerInteractEntityHook::_AutoHookCount;
    gInventoryRuntimeHookState = std::make_unique<InventoryRuntimeHookState>();
}

void unregisterInventoryRuntime() {
    gPendingBucketedEntities.clear();
    gInventoryRuntimeHookState.reset();
}

} // namespace advancements
