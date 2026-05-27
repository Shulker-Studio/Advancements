#include "mod/event/block/BeaconLevelChangedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/level/block/actor/BeaconBlockActor.h"

#include <memory>

namespace advancements::event::block {
namespace {

LL_TYPE_INSTANCE_HOOK(BeaconLevelChangedEventHook, HookPriority::Normal, BeaconBlockActor, &BeaconBlockActor::checkShape, void, BlockSource& region) {
    auto const previousLevel = this->mNumLevels;
    origin(region);

    auto const currentLevel = this->mNumLevels;
    if (currentLevel <= 0 || currentLevel <= previousLevel) {
        return;
    }

    BeaconLevelChangedEvent event{region, this->mPosition, currentLevel};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> beaconLevelChangedEventEmitterFactory();

class BeaconLevelChangedEventEmitter
    : public ll::event::Emitter<beaconLevelChangedEventEmitterFactory, BeaconLevelChangedEvent> {
    ll::memory::HookRegistrar<BeaconLevelChangedEventHook> beaconLevelChangedHook;
};

std::unique_ptr<ll::event::EmitterBase> beaconLevelChangedEventEmitterFactory() {
    return std::make_unique<BeaconLevelChangedEventEmitter>();
}

bool gBeaconLevelChangedEventSourceRegistered = false;

} // namespace

bool beaconLevelChangedEventSourceRegistered() { return gBeaconLevelChangedEventSourceRegistered; }

void registerBeaconLevelChangedEventSource() {
    if (beaconLevelChangedEventSourceRegistered()) {
        return;
    }

    (void)BeaconLevelChangedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<BeaconLevelChangedEvent>(beaconLevelChangedEventEmitterFactory);
    gBeaconLevelChangedEventSourceRegistered = true;
}

void unregisterBeaconLevelChangedEventSource() { gBeaconLevelChangedEventSourceRegistered = false; }

} // namespace advancements::event::block
