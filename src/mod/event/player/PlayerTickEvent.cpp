#include "mod/event/player/PlayerTickEvent.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/Emitter.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"

#include <memory>

namespace advancements::event::player {
namespace {

LL_TYPE_INSTANCE_HOOK(PlayerTickEventHook, HookPriority::Normal, Player, &Player::$tickWorld, void, Tick const& currentTick) {
    origin(currentTick);
    PlayerTickEvent event{*this, currentTick};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerTickEventEmitterFactory();

class PlayerTickEventEmitter : public ll::event::Emitter<playerTickEventEmitterFactory, PlayerTickEvent> {
    ll::memory::HookRegistrar<PlayerTickEventHook> tickWorldHook;
};

std::unique_ptr<ll::event::EmitterBase> playerTickEventEmitterFactory() {
    return std::make_unique<PlayerTickEventEmitter>();
}

bool gPlayerTickEventSourceRegistered = false;

} // namespace

bool playerTickEventSourceRegistered() { return gPlayerTickEventSourceRegistered; }

void registerPlayerTickEventSource() {
    if (playerTickEventSourceRegistered()) {
        return;
    }

    (void)PlayerTickEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerTickEvent>(playerTickEventEmitterFactory);
    gPlayerTickEventSourceRegistered = true;
}

void unregisterPlayerTickEventSource() { gPlayerTickEventSourceRegistered = false; }

} // namespace advancements::event::player
