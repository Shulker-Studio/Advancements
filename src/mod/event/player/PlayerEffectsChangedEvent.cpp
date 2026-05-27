#include "mod/event/player/PlayerEffectsChangedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/effect/MobEffectInstance.h"

#include <memory>

namespace advancements::event::player {
namespace {

LL_TYPE_INSTANCE_HOOK(PlayerEffectsChangedEventHook, HookPriority::Normal, Actor, &Actor::addEffect, void, MobEffectInstance const& effect) {
    origin(effect);

    auto* actor = static_cast<Actor*>(this);
    if (!actor->isPlayer()) {
        return;
    }

    PlayerEffectsChangedEvent event{*static_cast<Player*>(actor)};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerEffectsChangedEventEmitterFactory();

class PlayerEffectsChangedEventEmitter
    : public ll::event::Emitter<playerEffectsChangedEventEmitterFactory, PlayerEffectsChangedEvent> {
    ll::memory::HookRegistrar<PlayerEffectsChangedEventHook> playerEffectsChangedHook;
};

std::unique_ptr<ll::event::EmitterBase> playerEffectsChangedEventEmitterFactory() {
    return std::make_unique<PlayerEffectsChangedEventEmitter>();
}

bool gPlayerEffectsChangedEventSourceRegistered = false;

} // namespace

bool playerEffectsChangedEventSourceRegistered() { return gPlayerEffectsChangedEventSourceRegistered; }

void registerPlayerEffectsChangedEventSource() {
    if (playerEffectsChangedEventSourceRegistered()) {
        return;
    }

    (void)PlayerEffectsChangedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerEffectsChangedEvent>(playerEffectsChangedEventEmitterFactory);
    gPlayerEffectsChangedEventSourceRegistered = true;
}

void unregisterPlayerEffectsChangedEventSource() { gPlayerEffectsChangedEventSourceRegistered = false; }

} // namespace advancements::event::player
