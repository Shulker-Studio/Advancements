#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/effect/MobEffectInstance.h"

#include <memory>

namespace advancements {
namespace {

void dispatchEffectsChanged(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:effects_changed",
            EffectsChangedPayload{},
        }
    );
}

LL_TYPE_INSTANCE_HOOK(ActorAddEffectHook, HookPriority::Normal, Actor, &Actor::addEffect, void, MobEffectInstance const& effect) {
    origin(effect);

    auto* mod = currentRuntimeTriggerMod();
    auto* actor = static_cast<Actor*>(this);
    if (mod != nullptr && actor->isPlayer()) {
        dispatchEffectsChanged(*mod, *static_cast<Player*>(actor));
    }
}

struct EffectRuntimeHookState {
    ll::memory::HookRegistrar<ActorAddEffectHook> addEffectHook;
};

std::unique_ptr<EffectRuntimeHookState> gEffectRuntimeHookState;

} // namespace

bool effectRuntimeRegistered() { return gEffectRuntimeHookState != nullptr; }

void registerEffectRuntime() {
    if (effectRuntimeRegistered()) {
        return;
    }

    (void)ActorAddEffectHook::_AutoHookCount;
    gEffectRuntimeHookState = std::make_unique<EffectRuntimeHookState>();
}

void unregisterEffectRuntime() { gEffectRuntimeHookState.reset(); }

} // namespace advancements
