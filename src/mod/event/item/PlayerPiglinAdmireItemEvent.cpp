#include "mod/event/item/PlayerPiglinAdmireItemEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/ai/goal/AdmireItemGoal.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"

#include <memory>
#include <string>

namespace advancements::event::item {
namespace {

LL_TYPE_INSTANCE_HOOK(
    PlayerPiglinAdmireItemEventHook,
    HookPriority::Normal,
    AdmireItemGoal,
    &AdmireItemGoal::$start,
    void
) {
    origin();

    auto& mob = mMob;
    if (mob.getTypeName() != "minecraft:piglin") {
        return;
    }

    auto const& pickedItem = mItemStackPicked.get();
    if (pickedItem.isNull()) {
        return;
    }

    auto itemOwner = mItemOwner.get().tryUnwrap<Actor>();
    auto* itemOwnerActor = itemOwner.as_ptr();
    if (itemOwnerActor == nullptr || !itemOwnerActor->isPlayer()) {
        return;
    }

    auto const itemId = pickedItem.getTypeName();
    auto*      player = static_cast<Player*>(itemOwnerActor);
    PlayerPiglinAdmireItemEvent event{*player, itemId, false};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerPiglinAdmireItemEventEmitterFactory();

class PlayerPiglinAdmireItemEventEmitter
    : public ll::event::Emitter<playerPiglinAdmireItemEventEmitterFactory, PlayerPiglinAdmireItemEvent> {
    ll::memory::HookRegistrar<PlayerPiglinAdmireItemEventHook> playerPiglinAdmireItemHook;
};

std::unique_ptr<ll::event::EmitterBase> playerPiglinAdmireItemEventEmitterFactory() {
    return std::make_unique<PlayerPiglinAdmireItemEventEmitter>();
}

bool gPlayerPiglinAdmireItemEventSourceRegistered = false;

} // namespace

bool playerPiglinAdmireItemEventSourceRegistered() { return gPlayerPiglinAdmireItemEventSourceRegistered; }

void registerPlayerPiglinAdmireItemEventSource() {
    if (playerPiglinAdmireItemEventSourceRegistered()) {
        return;
    }

    (void)PlayerPiglinAdmireItemEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerPiglinAdmireItemEvent>(playerPiglinAdmireItemEventEmitterFactory);
    gPlayerPiglinAdmireItemEventSourceRegistered = true;
}

void unregisterPlayerPiglinAdmireItemEventSource() { gPlayerPiglinAdmireItemEventSourceRegistered = false; }

} // namespace advancements::event::item
