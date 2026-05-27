#include "mod/event/player/PlayerInteractedWithEntityEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/InteractionResult.h"

#include <mc/legacy/ActorUniqueID.h>

#include <memory>
#include <optional>

namespace advancements::event::player {
namespace {

constexpr int TemperateFrogVariant = 0;
constexpr int ColdFrogVariant      = 1;
constexpr int WarmFrogVariant      = 2;

std::optional<std::string> frogVariantIdForVariant(int variant) {
    switch (variant) {
    case TemperateFrogVariant:
        return std::string{"minecraft:temperate"};
    case ColdFrogVariant:
        return std::string{"minecraft:cold"};
    case WarmFrogVariant:
        return std::string{"minecraft:warm"};
    default:
        return std::nullopt;
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerInteractedWithEntityEventHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    InteractionResult,
    Actor&      actor,
    Vec3 const& location
) {
    auto const selectedItemId = !getSelectedItem().isNull() ? std::optional<std::string>{getSelectedItem().getTypeName()}
                                                            : std::nullopt;
    auto const actorTypeName = actor.getTypeName();
    auto const actorWasLeashed = actor.isLeashed();
    auto const leashHolderBefore = actor.getLeashHolder();
    auto const frogVariantId = actorTypeName == "minecraft:frog" ? frogVariantIdForVariant(actor.getVariant()) : std::nullopt;
    auto result = origin(actor, location);

    if ((!result.mSuccess && !result.mSwing) || !selectedItemId || *selectedItemId != "minecraft:lead" || actorTypeName != "minecraft:frog"
        || !frogVariantId || actorWasLeashed || !actor.isLeashed()) {
        return result;
    }

    auto const leashHolderAfter = actor.getLeashHolder();
    if (leashHolderAfter != ActorUniqueID::INVALID_ID() && leashHolderAfter == getOrCreateUniqueID()
        && leashHolderBefore != leashHolderAfter) {
        PlayerInteractedWithEntityEvent event{*this, *selectedItemId, actorTypeName, *frogVariantId};
        ll::event::EventBus::getInstance().publish(event);
    }

    return result;
}

std::unique_ptr<ll::event::EmitterBase> playerInteractedWithEntityEventEmitterFactory();

class PlayerInteractedWithEntityEventEmitter
    : public ll::event::Emitter<playerInteractedWithEntityEventEmitterFactory, PlayerInteractedWithEntityEvent> {
    ll::memory::HookRegistrar<PlayerInteractedWithEntityEventHook> playerInteractedWithEntityHook;
};

std::unique_ptr<ll::event::EmitterBase> playerInteractedWithEntityEventEmitterFactory() {
    return std::make_unique<PlayerInteractedWithEntityEventEmitter>();
}

bool gPlayerInteractedWithEntityEventSourceRegistered = false;

} // namespace

bool playerInteractedWithEntityEventSourceRegistered() { return gPlayerInteractedWithEntityEventSourceRegistered; }

void registerPlayerInteractedWithEntityEventSource() {
    if (playerInteractedWithEntityEventSourceRegistered()) {
        return;
    }

    (void)PlayerInteractedWithEntityEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerInteractedWithEntityEvent>(playerInteractedWithEntityEventEmitterFactory);
    gPlayerInteractedWithEntityEventSourceRegistered = true;
}

void unregisterPlayerInteractedWithEntityEventSource() { gPlayerInteractedWithEntityEventSourceRegistered = false; }

} // namespace advancements::event::player
