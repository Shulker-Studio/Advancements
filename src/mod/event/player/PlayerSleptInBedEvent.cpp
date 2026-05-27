#include "mod/event/player/PlayerSleptInBedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"

#include <memory>

namespace advancements::event::player {
namespace {

LL_TYPE_INSTANCE_HOOK(
    PlayerSleptInBedEventHook,
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

    PlayerSleptInBedEvent event{*this};
    ll::event::EventBus::getInstance().publish(event);
    return result;
}

std::unique_ptr<ll::event::EmitterBase> playerSleptInBedEventEmitterFactory();

class PlayerSleptInBedEventEmitter
    : public ll::event::Emitter<playerSleptInBedEventEmitterFactory, PlayerSleptInBedEvent> {
    ll::memory::HookRegistrar<PlayerSleptInBedEventHook> playerSleptInBedHook;
};

std::unique_ptr<ll::event::EmitterBase> playerSleptInBedEventEmitterFactory() {
    return std::make_unique<PlayerSleptInBedEventEmitter>();
}

bool gPlayerSleptInBedEventSourceRegistered = false;

} // namespace

bool playerSleptInBedEventSourceRegistered() { return gPlayerSleptInBedEventSourceRegistered; }

void registerPlayerSleptInBedEventSource() {
    if (playerSleptInBedEventSourceRegistered()) {
        return;
    }

    (void)PlayerSleptInBedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerSleptInBedEvent>(playerSleptInBedEventEmitterFactory);
    gPlayerSleptInBedEventSourceRegistered = true;
}

void unregisterPlayerSleptInBedEventSource() { gPlayerSleptInBedEventSourceRegistered = false; }

} // namespace advancements::event::player
