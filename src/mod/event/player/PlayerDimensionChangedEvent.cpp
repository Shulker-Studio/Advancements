#include "mod/event/player/PlayerDimensionChangedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"

#include <memory>

namespace advancements::event::player {
namespace {

LL_TYPE_INSTANCE_HOOK(
    PlayerDimensionChangedEventHook,
    HookPriority::Normal,
    Player,
    &Player::fireDimensionChangedEvent,
    void,
    DimensionType fromDimension,
    DimensionType toDimension
) {
    auto const positionBeforeChange = this->getPosition();
    origin(fromDimension, toDimension);

    if (fromDimension == toDimension) {
        return;
    }

    PlayerDimensionChangedEvent event{*this, fromDimension, toDimension, positionBeforeChange};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerDimensionChangedEventEmitterFactory();

class PlayerDimensionChangedEventEmitter
    : public ll::event::Emitter<playerDimensionChangedEventEmitterFactory, PlayerDimensionChangedEvent> {
    ll::memory::HookRegistrar<PlayerDimensionChangedEventHook> playerDimensionChangedHook;
};

std::unique_ptr<ll::event::EmitterBase> playerDimensionChangedEventEmitterFactory() {
    return std::make_unique<PlayerDimensionChangedEventEmitter>();
}

bool gPlayerDimensionChangedEventSourceRegistered = false;

} // namespace

bool playerDimensionChangedEventSourceRegistered() { return gPlayerDimensionChangedEventSourceRegistered; }

void registerPlayerDimensionChangedEventSource() {
    if (playerDimensionChangedEventSourceRegistered()) {
        return;
    }

    (void)PlayerDimensionChangedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerDimensionChangedEvent>(playerDimensionChangedEventEmitterFactory);
    gPlayerDimensionChangedEventSourceRegistered = true;
}

void unregisterPlayerDimensionChangedEventSource() { gPlayerDimensionChangedEventSourceRegistered = false; }

} // namespace advancements::event::player
