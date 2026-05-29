#include "mod/event/entity/PlayerTamedAnimalEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/entity/components_json_legacy/TameableComponent.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorFlags.h"
#include "mc/world/actor/player/Player.h"

#include <memory>
#include <string>

namespace advancements::event::entity {
namespace {

std::string catVariantId(int variant) {
    switch (variant) {
    case 0:
        return "minecraft:white";
    case 1:
        return "minecraft:black";
    case 2:
        return "minecraft:red";
    case 3:
        return "minecraft:siamese";
    case 4:
        return "minecraft:british_shorthair";
    case 5:
        return "minecraft:calico";
    case 6:
        return "minecraft:persian";
    case 7:
        return "minecraft:ragdoll";
    case 8:
        return "minecraft:tabby";
    case 9:
        return "minecraft:all_black";
    case 10:
        return "minecraft:jellie";
    default:
        return {};
    }
}

std::string wolfVariantId(int variant) {
    switch (variant) {
    case 0:
        return "minecraft:pale";
    case 1:
        return "minecraft:ashen";
    case 2:
        return "minecraft:black";
    case 3:
        return "minecraft:chestnut";
    case 4:
        return "minecraft:rusty";
    case 5:
        return "minecraft:snowy";
    case 6:
        return "minecraft:spotted";
    case 7:
        return "minecraft:striped";
    case 8:
        return "minecraft:woods";
    default:
        return {};
    }
}

std::string entityVariantId(std::string const& entityTypeId, int variant) {
    if (entityTypeId == "minecraft:cat") {
        return catVariantId(variant);
    }
    if (entityTypeId == "minecraft:wolf") {
        return wolfVariantId(variant);
    }
    return {};
}

LL_TYPE_INSTANCE_HOOK(
    PlayerTamedAnimalEventHook,
    HookPriority::Normal,
    TameableComponent,
    &TameableComponent::tame,
    void,
    Actor&  owner,
    Player& player
) {
    auto const wasTamed = owner.getStatusFlag(ActorFlags::Tamed);
    auto const entityTypeId   = owner.getTypeName();
    auto const variant        = owner.getVariant();
    auto const variantId      = entityVariantId(entityTypeId, variant);

    origin(owner, player);

    if (!wasTamed && owner.getStatusFlag(ActorFlags::Tamed)) {
        PlayerTamedAnimalEvent event{player, entityTypeId, variantId};
        ll::event::EventBus::getInstance().publish(event);
    }
}

std::unique_ptr<ll::event::EmitterBase> playerTamedAnimalEventEmitterFactory();

class PlayerTamedAnimalEventEmitter
    : public ll::event::Emitter<playerTamedAnimalEventEmitterFactory, PlayerTamedAnimalEvent> {
    ll::memory::HookRegistrar<PlayerTamedAnimalEventHook> playerTamedAnimalHook;
};

std::unique_ptr<ll::event::EmitterBase> playerTamedAnimalEventEmitterFactory() {
    return std::make_unique<PlayerTamedAnimalEventEmitter>();
}

bool gPlayerTamedAnimalEventSourceRegistered = false;

} // namespace

bool playerTamedAnimalEventSourceRegistered() { return gPlayerTamedAnimalEventSourceRegistered; }

void registerPlayerTamedAnimalEventSource() {
    if (gPlayerTamedAnimalEventSourceRegistered) {
        return;
    }
    (void)PlayerTamedAnimalEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerTamedAnimalEvent>(playerTamedAnimalEventEmitterFactory);
    gPlayerTamedAnimalEventSourceRegistered = true;
}

void unregisterPlayerTamedAnimalEventSource() { gPlayerTamedAnimalEventSourceRegistered = false; }

} // namespace advancements::event::entity
