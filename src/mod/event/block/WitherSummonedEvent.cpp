#include "mod/event/block/WitherSummonedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/SkullBlock.h"

#include <memory>

namespace advancements::event::block {
namespace {

LL_TYPE_INSTANCE_HOOK(
    WitherSummonedEventHook,
    HookPriority::Normal,
    SkullBlock,
    &SkullBlock::checkMobSpawn,
    bool,
    ::Level&       level,
    ::BlockSource& region,
    ::BlockPos const& pos
) {
    auto const spawned = origin(level, region, pos);
    if (spawned) {
        WitherSummonedEvent event{region, pos.center()};
        ll::event::EventBus::getInstance().publish(event);
    }
    return spawned;
}

std::unique_ptr<ll::event::EmitterBase> witherSummonedEventEmitterFactory();

class WitherSummonedEventEmitter : public ll::event::Emitter<witherSummonedEventEmitterFactory, WitherSummonedEvent> {
    ll::memory::HookRegistrar<WitherSummonedEventHook> witherSummonedHook;
};

std::unique_ptr<ll::event::EmitterBase> witherSummonedEventEmitterFactory() {
    return std::make_unique<WitherSummonedEventEmitter>();
}

bool gWitherSummonedEventSourceRegistered = false;

} // namespace

bool witherSummonedEventSourceRegistered() { return gWitherSummonedEventSourceRegistered; }

void registerWitherSummonedEventSource() {
    if (witherSummonedEventSourceRegistered()) {
        return;
    }

    (void)WitherSummonedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<WitherSummonedEvent>(witherSummonedEventEmitterFactory);
    gWitherSummonedEventSourceRegistered = true;
}

void unregisterWitherSummonedEventSource() { gWitherSummonedEventSourceRegistered = false; }

} // namespace advancements::event::block
