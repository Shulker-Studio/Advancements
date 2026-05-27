#include "mod/trigger/triggers/TargetHitTrigger.h"

#include "mod/Entry.h"
#include "mod/event/block/TargetBlockHitEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"

#include <cmath>

namespace advancements {
namespace {

ll::event::ListenerPtr gTargetBlockHitListener;

float horizontalDistance(Vec3 const& lhs, Vec3 const& rhs) {
    auto const dx = lhs.x - rhs.x;
    auto const dz = lhs.z - rhs.z;
    return std::sqrt(dx * dx + dz * dz);
}

void dispatchTargetHit(Entry& mod, Player& player, float projectileDistance) {
    auto const signalStrength = projectileDistance >= 30.0F ? 15 : 0;
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:target_hit",
            TargetHitPayload{signalStrength, projectileDistance},
        }
    );
}

} // namespace

bool targetHitTriggerRegistered() { return gTargetBlockHitListener != nullptr; }

void registerTargetHitTrigger(Entry& mod) {
    if (targetHitTriggerRegistered()) {
        return;
    }

    gTargetBlockHitListener = ll::event::EventBus::getInstance().emplaceListener<event::block::TargetBlockHitEvent>([&mod](auto& event) {
        auto* owner = event.projectile().getPlayerOwner();
        if (owner == nullptr) {
            return;
        }

        auto const projectileDistance = horizontalDistance(owner->getPosition(), event.projectile().getPosition());
        dispatchTargetHit(mod, *owner, projectileDistance);
    });
}

void unregisterTargetHitTrigger() {
    if (gTargetBlockHitListener) {
        ll::event::EventBus::getInstance().removeListener(gTargetBlockHitListener);
        gTargetBlockHitListener.reset();
    }
}

} // namespace advancements
