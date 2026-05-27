#include "mod/event/block/TargetBlockHitEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/TargetBlock.h"

#include <memory>

namespace advancements::event::block {
namespace {

LL_TYPE_INSTANCE_HOOK(
    TargetBlockHitEventHook,
    HookPriority::Normal,
    TargetBlock,
    &TargetBlock::$onProjectileHit,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor const&    projectile
) {
    origin(region, pos, projectile);
    TargetBlockHitEvent event{region, pos, projectile};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> targetBlockHitEventEmitterFactory();

class TargetBlockHitEventEmitter : public ll::event::Emitter<targetBlockHitEventEmitterFactory, TargetBlockHitEvent> {
    ll::memory::HookRegistrar<TargetBlockHitEventHook> targetBlockHitHook;
};

std::unique_ptr<ll::event::EmitterBase> targetBlockHitEventEmitterFactory() {
    return std::make_unique<TargetBlockHitEventEmitter>();
}

bool gTargetBlockHitEventSourceRegistered = false;

} // namespace

bool targetBlockHitEventSourceRegistered() { return gTargetBlockHitEventSourceRegistered; }

void registerTargetBlockHitEventSource() {
    if (targetBlockHitEventSourceRegistered()) {
        return;
    }

    (void)TargetBlockHitEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<TargetBlockHitEvent>(targetBlockHitEventEmitterFactory);
    gTargetBlockHitEventSourceRegistered = true;
}

void unregisterTargetBlockHitEventSource() { gTargetBlockHitEventSourceRegistered = false; }

} // namespace advancements::event::block
