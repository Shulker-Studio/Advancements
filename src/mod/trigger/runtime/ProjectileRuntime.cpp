#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/CrossbowItem.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/TargetBlock.h"

#include <cmath>
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

bool damageSourceDirectEntityIsArrow(ActorDamageSource const& source) {
    if (!source.isEntitySource()) {
        return false;
    }

    auto* directDamager = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (directDamager == nullptr) {
        return false;
    }
    return directDamager->getTypeName() == "minecraft:arrow";
}

bool damageSourceIsSupportedProjectile(ActorDamageSource const& source) {
    return source.isChildEntitySource() || damageSourceDirectEntityIsArrow(source);
}

float horizontalDistance(Vec3 const& lhs, Vec3 const& rhs) {
    auto const dx = lhs.x - rhs.x;
    auto const dz = lhs.z - rhs.z;
    return std::sqrt(dx * dx + dz * dz);
}

LL_TYPE_INSTANCE_HOOK(
    TargetBlockOnProjectileHitHook,
    HookPriority::Normal,
    TargetBlock,
    &TargetBlock::$onProjectileHit,
    void,
    ::BlockSource&    region,
    ::BlockPos const& pos,
    ::Actor const&    projectile
) {
    origin(region, pos, projectile);

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }

    auto* owner = projectile.getPlayerOwner();
    if (owner == nullptr) {
        return;
    }

    auto const projectileDistance = horizontalDistance(owner->getPosition(), projectile.getPosition());
    auto const signalStrength     = projectileDistance >= 30.0F ? 15 : 0;

    dispatchTrigger(
        *mod,
        TriggerContext{
            *owner,
            "minecraft:target_hit",
            TargetHitPayload{signalStrength, projectileDistance},
        }
    );
}

LL_TYPE_INSTANCE_HOOK(
    PlayerBlockUsingShieldHook,
    HookPriority::Normal,
    Player,
    &Player::_blockUsingShield,
    bool,
    ::ActorDamageSource const& source,
    float                      damage
) {
    auto const blocked = origin(source, damage);
    if (!blocked) {
        return blocked;
    }

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return blocked;
    }

    if (!damageSourceIsSupportedProjectile(source)) {
        return blocked;
    }

    dispatchTrigger(
        *mod,
        TriggerContext{
            *this,
            "minecraft:entity_hurt_player",
            EntityHurtPlayerPayload{true, true},
        }
    );
    return blocked;
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
    ll::memory::HookRegistrar<TargetBlockOnProjectileHitHook> targetBlockOnProjectileHitHook;
    ll::memory::HookRegistrar<PlayerBlockUsingShieldHook>     playerBlockUsingShieldHook;
    ll::memory::HookRegistrar<CrossbowShootArrowHook>         crossbowShootArrowHook;
};

std::unique_ptr<ProjectileRuntimeHookState> gProjectileRuntimeHookState;

} // namespace

bool projectileRuntimeRegistered() { return gProjectileRuntimeHookState != nullptr; }

void registerProjectileRuntime() {
    if (gProjectileRuntimeHookState) {
        return;
    }

    (void)TargetBlockOnProjectileHitHook::_AutoHookCount;
    (void)PlayerBlockUsingShieldHook::_AutoHookCount;
    (void)CrossbowShootArrowHook::_AutoHookCount;
    gProjectileRuntimeHookState = std::make_unique<ProjectileRuntimeHookState>();
}

void unregisterProjectileRuntime() { gProjectileRuntimeHookState.reset(); }

} // namespace advancements
