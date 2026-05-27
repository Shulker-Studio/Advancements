#include "mod/event/player/DragonRespawnedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/level/dimension/end/EndDragonFight.h"
#include "mc/world/level/dimension/end/RespawnAnimation.h"

#include <memory>

namespace advancements::event::player {
namespace {

LL_TYPE_INSTANCE_HOOK(DragonRespawnedEventHook, HookPriority::Normal, EndDragonFight, &EndDragonFight::tryRespawn, void) {
    auto const hadRespawnStage = this->mRespawnStage != RespawnAnimation::None;
    auto const hadCrystals     = !this->mRespawnCrystals->empty();
    origin();

    auto const enteredRespawnStage = !hadRespawnStage && this->mRespawnStage != RespawnAnimation::None;
    auto const recordedCrystals    = !hadCrystals && !this->mRespawnCrystals->empty();
    if (!enteredRespawnStage && !recordedCrystals) {
        return;
    }

    DragonRespawnedEvent event;
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> dragonRespawnedEventEmitterFactory();

class DragonRespawnedEventEmitter : public ll::event::Emitter<dragonRespawnedEventEmitterFactory, DragonRespawnedEvent> {
    ll::memory::HookRegistrar<DragonRespawnedEventHook> dragonRespawnedHook;
};

std::unique_ptr<ll::event::EmitterBase> dragonRespawnedEventEmitterFactory() {
    return std::make_unique<DragonRespawnedEventEmitter>();
}

bool gDragonRespawnedEventSourceRegistered = false;

} // namespace

bool dragonRespawnedEventSourceRegistered() { return gDragonRespawnedEventSourceRegistered; }

void registerDragonRespawnedEventSource() {
    if (dragonRespawnedEventSourceRegistered()) {
        return;
    }

    (void)DragonRespawnedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<DragonRespawnedEvent>(dragonRespawnedEventEmitterFactory);
    gDragonRespawnedEventSourceRegistered = true;
}

void unregisterDragonRespawnedEventSource() { gDragonRespawnedEventSourceRegistered = false; }

} // namespace advancements::event::player
