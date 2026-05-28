#include "mod/event/entity/PlayerProjectileLightningHitEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/entity/components_json_legacy/ProjectileComponent.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Weather.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/phys/HitResult.h"

#include <memory>
#include <string>

namespace advancements::event::entity {
namespace {

bool currentDimensionIsLightning(Actor const& projectile) {
    auto const& weather = projectile.getDimension().mWeather;
    return weather != nullptr && weather->mLightningLevel > 0.0F;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerProjectileLightningHitEventHook,
    HookPriority::Normal,
    ProjectileComponent,
    &ProjectileComponent::_handleLightningOnHit,
    void,
    Actor& projectile
) {
    auto* player = projectile.getPlayerOwner();
    auto* struckEntity = mHitResult->getEntity();
    auto const hitBlockPos = BlockPos{mHitResult->mPos};
    auto const projectileTypeId = projectile.getTypeName();
    auto const projectileIsChanneling = static_cast<bool>(mChanneling);
    auto const struckEntityTypeId = struckEntity != nullptr ? struckEntity->getTypeName() : std::string{};
    auto const dimensionIsLightning = currentDimensionIsLightning(projectile);
    auto const hitPositionCanSeeSky = projectile.getDimensionBlockSourceConst().canSeeSky(hitBlockPos);

    origin(projectile);

    if (player == nullptr) {
        return;
    }

    PlayerProjectileLightningHitEvent event{
        *player,
        projectileTypeId,
        projectileIsChanneling,
        struckEntityTypeId,
        dimensionIsLightning,
        hitPositionCanSeeSky,
    };
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerProjectileLightningHitEventEmitterFactory();

class PlayerProjectileLightningHitEventEmitter
    : public ll::event::Emitter<playerProjectileLightningHitEventEmitterFactory, PlayerProjectileLightningHitEvent> {
    ll::memory::HookRegistrar<PlayerProjectileLightningHitEventHook> playerProjectileLightningHitHook;
};

std::unique_ptr<ll::event::EmitterBase> playerProjectileLightningHitEventEmitterFactory() {
    return std::make_unique<PlayerProjectileLightningHitEventEmitter>();
}

bool gPlayerProjectileLightningHitEventSourceRegistered = false;

} // namespace

bool playerProjectileLightningHitEventSourceRegistered() { return gPlayerProjectileLightningHitEventSourceRegistered; }

void registerPlayerProjectileLightningHitEventSource() {
    if (playerProjectileLightningHitEventSourceRegistered()) {
        return;
    }

    (void)PlayerProjectileLightningHitEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerProjectileLightningHitEvent>(
        playerProjectileLightningHitEventEmitterFactory
    );
    gPlayerProjectileLightningHitEventSourceRegistered = true;
}

void unregisterPlayerProjectileLightningHitEventSource() { gPlayerProjectileLightningHitEventSourceRegistered = false; }

} // namespace advancements::event::entity