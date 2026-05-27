#include "mod/event/player/PlayerBlockUsingShieldEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"

#include <memory>

namespace advancements::event::player {
namespace {

LL_TYPE_INSTANCE_HOOK(
    PlayerBlockUsingShieldEventHook,
    HookPriority::Normal,
    Player,
    &Player::_blockUsingShield,
    bool,
    ::ActorDamageSource const& source,
    float                      damage
) {
    auto const blocked = origin(source, damage);
    if (!blocked) {
        return blocked;
    }

    PlayerBlockUsingShieldEvent event{*this, source};
    ll::event::EventBus::getInstance().publish(event);
    return blocked;
}

std::unique_ptr<ll::event::EmitterBase> playerBlockUsingShieldEventEmitterFactory();

class PlayerBlockUsingShieldEventEmitter
    : public ll::event::Emitter<playerBlockUsingShieldEventEmitterFactory, PlayerBlockUsingShieldEvent> {
    ll::memory::HookRegistrar<PlayerBlockUsingShieldEventHook> playerBlockUsingShieldHook;
};

std::unique_ptr<ll::event::EmitterBase> playerBlockUsingShieldEventEmitterFactory() {
    return std::make_unique<PlayerBlockUsingShieldEventEmitter>();
}

bool gPlayerBlockUsingShieldEventSourceRegistered = false;

} // namespace

bool playerBlockUsingShieldEventSourceRegistered() { return gPlayerBlockUsingShieldEventSourceRegistered; }

void registerPlayerBlockUsingShieldEventSource() {
    if (playerBlockUsingShieldEventSourceRegistered()) {
        return;
    }

    (void)PlayerBlockUsingShieldEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerBlockUsingShieldEvent>(playerBlockUsingShieldEventEmitterFactory);
    gPlayerBlockUsingShieldEventSourceRegistered = true;
}

void unregisterPlayerBlockUsingShieldEventSource() { gPlayerBlockUsingShieldEventSourceRegistered = false; }

} // namespace advancements::event::player
