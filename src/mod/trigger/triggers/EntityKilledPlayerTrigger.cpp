#include "mod/trigger/triggers/EntityKilledPlayerTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerKilledByEntityEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerKilledByEntityListener;

} // namespace

bool entityKilledPlayerTriggerRegistered() { return gPlayerKilledByEntityListener != nullptr; }

void registerEntityKilledPlayerTrigger(Entry& mod) {
    if (entityKilledPlayerTriggerRegistered()) {
        return;
    }

    gPlayerKilledByEntityListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerKilledByEntityEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.self(),
                    "minecraft:entity_killed_player",
                    EntityTriggerPayload{event.killerEntityTypeId()},
                }
            );
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
