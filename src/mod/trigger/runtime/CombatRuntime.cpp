#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "mod/Entry.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/event/entity/MobDieEvent.h"
#include "ll/api/event/player/PlayerDieEvent.h"
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

ll::event::ListenerPtr gMobDieListener;
ll::event::ListenerPtr gPlayerDieListener;
ll::event::ListenerPtr gActorHurtListener;

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

std::optional<std::string> findKillerEntityTypeId(ActorDamageSource const& source) {
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* actor = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (actor == nullptr) {
        return std::nullopt;
    }
    if (source.isChildEntitySource()) {
        actor = actor->getOwner();
    }
    if (actor == nullptr) {
        return std::nullopt;
    }
    return actor->getTypeName();
}

std::optional<Player*> findHurtingPlayer(ActorDamageSource const& source) {
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* actor = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (actor == nullptr) {
        return std::nullopt;
    }
    if (source.isChildEntitySource()) {
        actor = actor->getOwner();
    }
    if (actor == nullptr) {
        return std::nullopt;
    }
    if (!actor->isPlayer()) {
        return std::nullopt;
    }
    return static_cast<Player*>(actor);
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

bool playerMainhandItemIsMace(Player const& player) {
    auto const& selectedItem = player.getSelectedItem();
    return !selectedItem.isNull() && selectedItem.getTypeName() == "minecraft:mace";
}

float horizontalDistance(Vec3 const& lhs, Vec3 const& rhs) {
    auto const dx = lhs.x - rhs.x;
    auto const dz = lhs.z - rhs.z;
    return std::sqrt(dx * dx + dz * dz);
}

} // namespace

bool combatRuntimeRegistered() { return gMobDieListener || gPlayerDieListener || gActorHurtListener; }

void registerCombatRuntime(Entry& mod) {
    if (combatRuntimeRegistered()) {
        return;
    }

    auto& eventBus = ll::event::EventBus::getInstance();
    gMobDieListener = eventBus.emplaceListener<ll::event::MobDieEvent>([&mod](auto& event) {
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
    });

    gPlayerDieListener = eventBus.emplaceListener<ll::event::player::PlayerDieEvent>([&mod](auto& event) {
        auto const killerEntityTypeId = findKillerEntityTypeId(event.source());
        if (!killerEntityTypeId) {
            return true;
        }

        dispatchTrigger(
            mod,
            TriggerContext{
                event.self(),
                "minecraft:entity_killed_player",
                EntityTriggerPayload{*killerEntityTypeId},
            }
        );
        return true;
    });

    gActorHurtListener = eventBus.emplaceListener<ll::event::ActorHurtEvent>([&mod](auto& event) {
        auto hurtingPlayer = findHurtingPlayer(event.source());
        if (!hurtingPlayer) {
            return true;
        }

        auto const mainhandItemIsMace = playerMainhandItemIsMace(**hurtingPlayer);

        dispatchTrigger(
            mod,
            TriggerContext{
                **hurtingPlayer,
                "minecraft:player_hurt_entity",
                PlayerHurtEntityPayload{
                    damageSourceDirectEntityIsArrow(event.source()),
                    findDirectDamagerTypeId(event.source()),
                    damageSourceIsSupportedProjectile(event.source()),
                    mainhandItemIsMace,
                    event.damage(),
                },
            }
        );

        return true;
    });
}

void unregisterCombatRuntime() {
    auto& eventBus = ll::event::EventBus::getInstance();
    if (gMobDieListener) {
        eventBus.removeListener(gMobDieListener);
        gMobDieListener.reset();
    }
    if (gPlayerDieListener) {
        eventBus.removeListener(gPlayerDieListener);
        gPlayerDieListener.reset();
    }
    if (gActorHurtListener) {
        eventBus.removeListener(gActorHurtListener);
        gActorHurtListener.reset();
    }
}

} // namespace advancements
