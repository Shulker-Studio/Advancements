#include "mod/trigger/triggers/PlayerKilledEntityTrigger.h"

#include "mod/Entry.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/MobDieEvent.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <cmath>
#include <optional>
#include <string>

namespace advancements {
namespace {

ll::event::ListenerPtr gEntityKilledByPlayerListener;

std::optional<Player*> findKillingPlayer(ll::event::MobDieEvent& event) {
    auto const& source = event.source();
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* player = event.self().getLastHurtByPlayer();
    if (player == nullptr) {
        return std::nullopt;
    }
    return player;
}

std::optional<std::string> findDirectDamagerTypeId(ActorDamageSource const& source) {
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* actor = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (actor == nullptr) {
        return std::nullopt;
    }
    return actor->getTypeName();
}

bool damageSourceDirectEntityIsArrow(ActorDamageSource const& source) {
    return findDirectDamagerTypeId(source) == "minecraft:arrow";
}

bool damageSourceIsSupportedProjectile(ActorDamageSource const& source) {
    return source.isChildEntitySource() || damageSourceDirectEntityIsArrow(source);
}

float horizontalDistance(Vec3 const& lhs, Vec3 const& rhs) {
    auto const dx = lhs.x - rhs.x;
    auto const dz = lhs.z - rhs.z;
    return std::sqrt(dx * dx + dz * dz);
}

} // namespace

bool playerKilledEntityTriggerRegistered() { return gEntityKilledByPlayerListener != nullptr; }

void registerPlayerKilledEntityTrigger(Entry& mod) {
    if (playerKilledEntityTriggerRegistered()) {
        return;
    }

    gEntityKilledByPlayerListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::MobDieEvent>(
        [&mod](ll::event::MobDieEvent& event) {
            auto player = findKillingPlayer(event);
            if (!player) {
                return true;
            }

            dispatchTrigger(
                mod,
                TriggerContext{
                    **player,
                    "minecraft:player_killed_entity",
                    PlayerKilledEntitySniperDuelPayload{
                        event.self().getTypeName(),
                        horizontalDistance((**player).getPosition(), event.self().getPosition()),
                        damageSourceIsSupportedProjectile(event.source()),
                        findDirectDamagerTypeId(event.source()),
                    },
                }
            );
            return true;
        }
    );
}

void unregisterPlayerKilledEntityTrigger() {
    if (gEntityKilledByPlayerListener) {
        ll::event::EventBus::getInstance().removeListener(gEntityKilledByPlayerListener);
        gEntityKilledByPlayerListener.reset();
    }
}

} // namespace advancements
