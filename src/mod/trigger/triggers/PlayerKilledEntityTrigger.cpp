#include "mod/trigger/triggers/PlayerKilledEntityTrigger.h"

#include "mod/Entry.h"
#include "mod/event/entity/EntityKilledByPlayerEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gEntityKilledByPlayerListener;

} // namespace

bool playerKilledEntityTriggerRegistered() { return gEntityKilledByPlayerListener != nullptr; }

void registerPlayerKilledEntityTrigger(Entry& mod) {
    if (playerKilledEntityTriggerRegistered()) {
        return;
    }

    gEntityKilledByPlayerListener = ll::event::EventBus::getInstance().emplaceListener<event::entity::EntityKilledByPlayerEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:player_killed_entity",
                    PlayerKilledEntitySniperDuelPayload{
                        event.killedEntityTypeId(),
                        event.horizontalDistance(),
                        event.killingBlowIsProjectile(),
                        event.directEntityTypeId(),
                    },
                }
            );
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
