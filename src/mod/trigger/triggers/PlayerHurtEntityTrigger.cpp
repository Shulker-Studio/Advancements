#include "mod/trigger/triggers/PlayerHurtEntityTrigger.h"

#include "mod/Entry.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <optional>
#include <string>

namespace advancements {
namespace {

ll::event::ListenerPtr gEntityHurtByPlayerListener;

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
    if (actor == nullptr || !actor->isPlayer()) {
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

} // namespace

bool playerHurtEntityTriggerRegistered() { return gEntityHurtByPlayerListener != nullptr; }

void registerPlayerHurtEntityTrigger(Entry& mod) {
    if (playerHurtEntityTriggerRegistered()) {
        return;
    }

    gEntityHurtByPlayerListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::ActorHurtEvent>([&mod](auto& event) {
        auto hurtingPlayer = findHurtingPlayer(event.source());
        if (!hurtingPlayer) {
            return true;
        }

        dispatchTrigger(
            mod,
            TriggerContext{
                **hurtingPlayer,
                "minecraft:player_hurt_entity",
                PlayerHurtEntityPayload{
                    damageSourceDirectEntityIsArrow(event.source()),
                    findDirectDamagerTypeId(event.source()),
                    damageSourceIsSupportedProjectile(event.source()),
                    playerMainhandItemIsMace(**hurtingPlayer),
                    event.damage(),
                },
            }
        );
        return true;
    });
}

void unregisterPlayerHurtEntityTrigger() {
    if (gEntityHurtByPlayerListener) {
        ll::event::EventBus::getInstance().removeListener(gEntityHurtByPlayerListener);
        gEntityHurtByPlayerListener.reset();
    }
}

} // namespace advancements
