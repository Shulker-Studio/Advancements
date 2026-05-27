#include "mod/trigger/triggers/PlayerHurtEntityTrigger.h"

#include "mod/Entry.h"
#include "mod/event/entity/EntityHurtByPlayerEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gEntityHurtByPlayerListener;

} // namespace

bool playerHurtEntityTriggerRegistered() { return gEntityHurtByPlayerListener != nullptr; }

void registerPlayerHurtEntityTrigger(Entry& mod) {
    if (playerHurtEntityTriggerRegistered()) {
        return;
    }

    gEntityHurtByPlayerListener = ll::event::EventBus::getInstance().emplaceListener<event::entity::EntityHurtByPlayerEvent>([&mod](auto& event) {
        dispatchTrigger(
            mod,
            TriggerContext{
                event.self(),
                "minecraft:player_hurt_entity",
                PlayerHurtEntityPayload{
                    event.directEntityIsArrow(),
                    event.directEntityTypeId(),
                    event.isProjectileDamage(),
                    event.mainhandItemIsMace(),
                    event.damageDealt(),
                },
            }
        );
    });
}

void unregisterPlayerHurtEntityTrigger() {
    if (gEntityHurtByPlayerListener) {
        ll::event::EventBus::getInstance().removeListener(gEntityHurtByPlayerListener);
        gEntityHurtByPlayerListener.reset();
    }
}

} // namespace advancements
