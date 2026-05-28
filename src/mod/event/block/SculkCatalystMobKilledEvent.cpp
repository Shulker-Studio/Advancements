#include "mod/event/block/SculkCatalystMobKilledEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/SculkSpreader.h"
#include "mc/world/level/block/actor/SculkCatalystBlockActor.h"

#include <memory>

namespace advancements::event::block {
namespace {

struct ActiveSculkCatalystDeath {
    Player* player;
    bool    cursorAdded;
};

thread_local ActiveSculkCatalystDeath* gActiveSculkCatalystDeath = nullptr;

LL_TYPE_INSTANCE_HOOK(
    SculkCatalystAddCursorsEventHook,
    HookPriority::Normal,
    SculkSpreader,
    &SculkSpreader::addCursors,
    void,
    BlockPos const& pos,
    int charge
) {
    if (gActiveSculkCatalystDeath != nullptr && charge > 0) {
        gActiveSculkCatalystDeath->cursorAdded = true;
    }

    origin(pos, charge);
}

LL_TYPE_INSTANCE_HOOK(
    SculkCatalystMobKilledEventHook,
    HookPriority::Normal,
    SculkCatalystBlockActor,
    &SculkCatalystBlockActor::_tryConsumeOnDeathExperience,
    void,
    Level& level,
    Actor& actor
) {
    auto* player = actor.getIsExperienceDropEnabled() ? actor.getLastHurtByPlayer() : nullptr;
    auto activeDeath = ActiveSculkCatalystDeath{player, false};
    auto* previousActiveDeath = gActiveSculkCatalystDeath;

    gActiveSculkCatalystDeath = &activeDeath;
    origin(level, actor);
    gActiveSculkCatalystDeath = previousActiveDeath;

    if (player == nullptr || !activeDeath.cursorAdded || actor.getIsExperienceDropEnabled()) {
        return;
    }

    SculkCatalystMobKilledEvent event{*player, actor};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> sculkCatalystMobKilledEventEmitterFactory();

class SculkCatalystMobKilledEventEmitter
    : public ll::event::Emitter<sculkCatalystMobKilledEventEmitterFactory, SculkCatalystMobKilledEvent> {
    ll::memory::HookRegistrar<SculkCatalystAddCursorsEventHook> sculkCatalystAddCursorsHook;
    ll::memory::HookRegistrar<SculkCatalystMobKilledEventHook> sculkCatalystMobKilledHook;
};

std::unique_ptr<ll::event::EmitterBase> sculkCatalystMobKilledEventEmitterFactory() {
    return std::make_unique<SculkCatalystMobKilledEventEmitter>();
}

bool gSculkCatalystMobKilledEventSourceRegistered = false;

} // namespace

bool sculkCatalystMobKilledEventSourceRegistered() { return gSculkCatalystMobKilledEventSourceRegistered; }

void registerSculkCatalystMobKilledEventSource() {
    if (sculkCatalystMobKilledEventSourceRegistered()) {
        return;
    }

    (void)SculkCatalystMobKilledEventHook::_AutoHookCount;
    (void)SculkCatalystAddCursorsEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<SculkCatalystMobKilledEvent>(sculkCatalystMobKilledEventEmitterFactory);
    gSculkCatalystMobKilledEventSourceRegistered = true;
}

void unregisterSculkCatalystMobKilledEventSource() { gSculkCatalystMobKilledEventSourceRegistered = false; }

} // namespace advancements::event::block
