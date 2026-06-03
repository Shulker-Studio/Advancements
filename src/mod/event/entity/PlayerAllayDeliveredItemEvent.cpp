#include "mod/event/entity/PlayerAllayDeliveredItemEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/ai/goal/GoAndGiveItemsToOwnerGoal.h"

#include <memory>

namespace advancements::event::entity {
namespace {

LL_TYPE_INSTANCE_HOOK(
    PlayerAllayDeliveredItemEventHook,
    HookPriority::Normal,
    GoAndGiveItemsToOwnerGoal,
    &GoAndGiveItemsToOwnerGoal::_attemptToGiveItem,
    bool
) {
    auto& mob = mMob;
    if (mob.getTypeName() != "minecraft:allay") {
        return origin();
    }

    auto* player = mob.getPlayerOwner();
    if (player == nullptr) {
        return origin();
    }

    auto const delivered = origin();
    if (!delivered) {
        return false;
    }

    PlayerAllayDeliveredItemEvent event{*player};
    ll::event::EventBus::getInstance().publish(event);
    return true;
}

std::unique_ptr<ll::event::EmitterBase> playerAllayDeliveredItemEventEmitterFactory();

class PlayerAllayDeliveredItemEventEmitter
    : public ll::event::Emitter<playerAllayDeliveredItemEventEmitterFactory, PlayerAllayDeliveredItemEvent> {
    ll::memory::HookRegistrar<PlayerAllayDeliveredItemEventHook> playerAllayDeliveredItemHook;
};

std::unique_ptr<ll::event::EmitterBase> playerAllayDeliveredItemEventEmitterFactory() {
    return std::make_unique<PlayerAllayDeliveredItemEventEmitter>();
}

bool gPlayerAllayDeliveredItemEventSourceRegistered = false;

} // namespace

bool playerAllayDeliveredItemEventSourceRegistered() { return gPlayerAllayDeliveredItemEventSourceRegistered; }

void registerPlayerAllayDeliveredItemEventSource() {
    if (playerAllayDeliveredItemEventSourceRegistered()) {
        return;
    }

    (void)PlayerAllayDeliveredItemEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerAllayDeliveredItemEvent>(
        playerAllayDeliveredItemEventEmitterFactory
    );
    gPlayerAllayDeliveredItemEventSourceRegistered = true;
}

void unregisterPlayerAllayDeliveredItemEventSource() { gPlayerAllayDeliveredItemEventSourceRegistered = false; }

} // namespace advancements::event::entity
