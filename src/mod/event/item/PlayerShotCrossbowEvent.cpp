#include "mod/event/item/PlayerShotCrossbowEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/item/CrossbowItem.h"

#include <memory>

namespace advancements::event::item {
namespace {

LL_TYPE_INSTANCE_HOOK(
    PlayerShotCrossbowEventHook,
    HookPriority::Normal,
    CrossbowItem,
    &CrossbowItem::_shootArrow,
    void,
    ::ItemInstance const& crossbow,
    ::ItemInstance const& projectileInstance,
    ::Player&             player
) {
    origin(crossbow, projectileInstance, player);

    PlayerShotCrossbowEvent event{player};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerShotCrossbowEventEmitterFactory();

class PlayerShotCrossbowEventEmitter
    : public ll::event::Emitter<playerShotCrossbowEventEmitterFactory, PlayerShotCrossbowEvent> {
    ll::memory::HookRegistrar<PlayerShotCrossbowEventHook> playerShotCrossbowHook;
};

std::unique_ptr<ll::event::EmitterBase> playerShotCrossbowEventEmitterFactory() {
    return std::make_unique<PlayerShotCrossbowEventEmitter>();
}

bool gPlayerShotCrossbowEventSourceRegistered = false;

} // namespace

bool playerShotCrossbowEventSourceRegistered() { return gPlayerShotCrossbowEventSourceRegistered; }

void registerPlayerShotCrossbowEventSource() {
    if (playerShotCrossbowEventSourceRegistered()) {
        return;
    }

    (void)PlayerShotCrossbowEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerShotCrossbowEvent>(playerShotCrossbowEventEmitterFactory);
    gPlayerShotCrossbowEventSourceRegistered = true;
}

void unregisterPlayerShotCrossbowEventSource() { gPlayerShotCrossbowEventSourceRegistered = false; }

} // namespace advancements::event::item
