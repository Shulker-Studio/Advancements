#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/CrossbowItem.h"
#include "mc/world/level/Level.h"

#include <memory>

namespace advancements {
namespace {

void dispatchShotCrossbow(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:shot_crossbow",
            ItemTriggerPayload{"minecraft:crossbow", std::nullopt},
        }
    );
}

LL_TYPE_INSTANCE_HOOK(
    CrossbowShootArrowHook,
    HookPriority::Normal,
    CrossbowItem,
    &CrossbowItem::_shootArrow,
    void,
    ::ItemInstance const& crossbow,
    ::ItemInstance const& projectileInstance,
    ::Player&             player
) {
    origin(crossbow, projectileInstance, player);

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }
    dispatchShotCrossbow(*mod, player);
}

struct ProjectileRuntimeHookState {
    ll::memory::HookRegistrar<CrossbowShootArrowHook>         crossbowShootArrowHook;
};

std::unique_ptr<ProjectileRuntimeHookState> gProjectileRuntimeHookState;

} // namespace

bool projectileRuntimeRegistered() { return gProjectileRuntimeHookState != nullptr; }

void registerProjectileRuntime() {
    if (gProjectileRuntimeHookState) {
        return;
    }

    (void)CrossbowShootArrowHook::_AutoHookCount;
    gProjectileRuntimeHookState = std::make_unique<ProjectileRuntimeHookState>();
}

void unregisterProjectileRuntime() { gProjectileRuntimeHookState.reset(); }

} // namespace advancements
