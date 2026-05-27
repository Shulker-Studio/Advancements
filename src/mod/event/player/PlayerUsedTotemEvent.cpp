#include "mod/event/player/PlayerUsedTotemEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"

#include <memory>

namespace advancements::event::player {
namespace {

LL_TYPE_INSTANCE_HOOK(PlayerUsedTotemEventHook, HookPriority::Normal, Player, &Player::$consumeTotem, bool) {
    auto const consumed = origin();
    if (!consumed) {
        return consumed;
    }

    PlayerUsedTotemEvent event{*this};
    ll::event::EventBus::getInstance().publish(event);
    return consumed;
}

std::unique_ptr<ll::event::EmitterBase> playerUsedTotemEventEmitterFactory();

class PlayerUsedTotemEventEmitter : public ll::event::Emitter<playerUsedTotemEventEmitterFactory, PlayerUsedTotemEvent> {
    ll::memory::HookRegistrar<PlayerUsedTotemEventHook> playerUsedTotemHook;
};

std::unique_ptr<ll::event::EmitterBase> playerUsedTotemEventEmitterFactory() {
    return std::make_unique<PlayerUsedTotemEventEmitter>();
}

bool gPlayerUsedTotemEventSourceRegistered = false;

} // namespace

bool playerUsedTotemEventSourceRegistered() { return gPlayerUsedTotemEventSourceRegistered; }

void registerPlayerUsedTotemEventSource() {
    if (playerUsedTotemEventSourceRegistered()) {
        return;
    }

    (void)PlayerUsedTotemEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerUsedTotemEvent>(playerUsedTotemEventEmitterFactory);
    gPlayerUsedTotemEventSourceRegistered = true;
}

void unregisterPlayerUsedTotemEventSource() { gPlayerUsedTotemEventSourceRegistered = false; }

} // namespace advancements::event::player
