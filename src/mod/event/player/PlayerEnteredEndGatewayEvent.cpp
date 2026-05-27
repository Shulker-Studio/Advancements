#include "mod/event/player/PlayerEnteredEndGatewayEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/block/actor/EndGatewayBlockActor.h"

#include <memory>

namespace advancements::event::player {
namespace {

bool positionChanged(Vec3 const& before, Vec3 const& after) {
    return before.x != after.x || before.y != after.y || before.z != after.z;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerEnteredEndGatewayEventHook,
    HookPriority::Normal,
    EndGatewayBlockActor,
    &EndGatewayBlockActor::teleportEntity,
    void,
    Actor& entity
) {
    auto const isPlayer = entity.isPlayer();
    auto const positionBeforeTeleport = entity.getPosition();

    origin(entity);

    if (!isPlayer || !positionChanged(positionBeforeTeleport, entity.getPosition())) {
        return;
    }

    PlayerEnteredEndGatewayEvent event{static_cast<Player&>(entity)};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerEnteredEndGatewayEventEmitterFactory();

class PlayerEnteredEndGatewayEventEmitter
    : public ll::event::Emitter<playerEnteredEndGatewayEventEmitterFactory, PlayerEnteredEndGatewayEvent> {
    ll::memory::HookRegistrar<PlayerEnteredEndGatewayEventHook> playerEnteredEndGatewayHook;
};

std::unique_ptr<ll::event::EmitterBase> playerEnteredEndGatewayEventEmitterFactory() {
    return std::make_unique<PlayerEnteredEndGatewayEventEmitter>();
}

bool gPlayerEnteredEndGatewayEventSourceRegistered = false;

} // namespace

bool playerEnteredEndGatewayEventSourceRegistered() { return gPlayerEnteredEndGatewayEventSourceRegistered; }

void registerPlayerEnteredEndGatewayEventSource() {
    if (playerEnteredEndGatewayEventSourceRegistered()) {
        return;
    }

    (void)PlayerEnteredEndGatewayEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerEnteredEndGatewayEvent>(playerEnteredEndGatewayEventEmitterFactory);
    gPlayerEnteredEndGatewayEventSourceRegistered = true;
}

void unregisterPlayerEnteredEndGatewayEventSource() { gPlayerEnteredEndGatewayEventSourceRegistered = false; }

} // namespace advancements::event::player
