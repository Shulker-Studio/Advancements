#include "mod/trigger/triggers/EntityKilledPlayerTrigger.h"

#include "mod/Entry.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <optional>
#include <string>

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerKilledByEntityListener;

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

} // namespace

bool entityKilledPlayerTriggerRegistered() { return gPlayerKilledByEntityListener != nullptr; }

void registerEntityKilledPlayerTrigger(Entry& mod) {
    if (entityKilledPlayerTriggerRegistered()) {
        return;
    }

    gPlayerKilledByEntityListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerDieEvent>(
        [&mod](auto& event) {
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
        }
    );
}

void unregisterEntityKilledPlayerTrigger() {
    if (gPlayerKilledByEntityListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerKilledByEntityListener);
        gPlayerKilledByEntityListener.reset();
    }
}

} // namespace advancements
