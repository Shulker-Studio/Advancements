#include "mod/trigger/triggers/EntityHurtPlayerTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerBlockUsingShieldEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/level/Level.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerBlockUsingShieldListener;

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

} // namespace

bool entityHurtPlayerTriggerRegistered() { return gPlayerBlockUsingShieldListener != nullptr; }

void registerEntityHurtPlayerTrigger(Entry& mod) {
    if (entityHurtPlayerTriggerRegistered()) {
        return;
    }

    gPlayerBlockUsingShieldListener =
        ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerBlockUsingShieldEvent>([&mod](auto& event) {
            if (!damageSourceIsSupportedProjectile(event.source())) {
                return;
            }

            dispatchTrigger(
                mod,
                TriggerContext{
                    event.self(),
                    "minecraft:entity_hurt_player",
                    EntityHurtPlayerPayload{true, true},
                }
            );
        });
}

void unregisterEntityHurtPlayerTrigger() {
    if (gPlayerBlockUsingShieldListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerBlockUsingShieldListener);
        gPlayerBlockUsingShieldListener.reset();
    }
}

} // namespace advancements
