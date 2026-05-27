#include "mod/event/item/PlayerFilledBucketEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/item/BucketItem.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockPos.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace advancements::event::item {
namespace {

std::unordered_map<uint64_t, std::string> gPendingBucketedEntities;

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
    BucketUseOnEntityFilledBucketEventHook,
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
    auto       result   = origin(instance, entity, pos, face, clickPos);
    if (result.mSwing) {
        gPendingBucketedEntities[entityId] = *bucketItemId;
    }
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerInteractFilledBucketEventHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    InteractionResult,
    Actor&      actor,
    Vec3 const& location
) {
    auto const entityId = actor.getOrCreateUniqueID().getHash();
    auto       result   = origin(actor, location);

    if (!result.mSuccess && !result.mSwing) {
        return result;
    }

    auto pending = gPendingBucketedEntities.find(entityId);
    if (pending == gPendingBucketedEntities.end()) {
        return result;
    }

    PlayerFilledBucketEvent event{*this, pending->second};
    ll::event::EventBus::getInstance().publish(event);
    gPendingBucketedEntities.erase(pending);
    return result;
}

std::unique_ptr<ll::event::EmitterBase> playerFilledBucketEventEmitterFactory();

class PlayerFilledBucketEventEmitter
    : public ll::event::Emitter<playerFilledBucketEventEmitterFactory, PlayerFilledBucketEvent> {
    ll::memory::HookRegistrar<BucketUseOnEntityFilledBucketEventHook> bucketUseOnEntityHook;
    ll::memory::HookRegistrar<PlayerInteractFilledBucketEventHook>    playerInteractHook;
};

std::unique_ptr<ll::event::EmitterBase> playerFilledBucketEventEmitterFactory() {
    return std::make_unique<PlayerFilledBucketEventEmitter>();
}

bool gPlayerFilledBucketEventSourceRegistered = false;

} // namespace

bool playerFilledBucketEventSourceRegistered() { return gPlayerFilledBucketEventSourceRegistered; }

void registerPlayerFilledBucketEventSource() {
    if (playerFilledBucketEventSourceRegistered()) {
        return;
    }

    (void)BucketUseOnEntityFilledBucketEventHook::_AutoHookCount;
    (void)PlayerInteractFilledBucketEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerFilledBucketEvent>(playerFilledBucketEventEmitterFactory);
    gPlayerFilledBucketEventSourceRegistered = true;
}

void unregisterPlayerFilledBucketEventSource() {
    gPendingBucketedEntities.clear();
    gPlayerFilledBucketEventSourceRegistered = false;
}

} // namespace advancements::event::item
