#include "mod/event/entity/PlayerTamedAnimalEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/entity/components_json_legacy/TameableComponent.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorFlags.h"
#include "mc/world/actor/player/Player.h"

#include <memory>

namespace advancements::event::entity {
namespace {

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
    auto const entityTypeId = owner.getTypeName();

    origin(owner, player);

    if (!wasTamed && owner.getStatusFlag(ActorFlags::Tamed)) {
        PlayerTamedAnimalEvent event{player, entityTypeId};
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
