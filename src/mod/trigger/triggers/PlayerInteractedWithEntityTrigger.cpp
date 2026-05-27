#include "mod/trigger/triggers/PlayerInteractedWithEntityTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerInteractedWithEntityEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerInteractedWithEntityListener;

} // namespace

bool playerInteractedWithEntityTriggerRegistered() { return gPlayerInteractedWithEntityListener != nullptr; }

void registerPlayerInteractedWithEntityTrigger(Entry& mod) {
    if (playerInteractedWithEntityTriggerRegistered()) {
        return;
    }

    gPlayerInteractedWithEntityListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerInteractedWithEntityEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:player_interacted_with_entity",
                    PlayerInteractedWithEntityPayload{event.itemId(), event.entityTypeId(), event.entityVariantId()},
                }
            );
        }
    );
}

void unregisterPlayerInteractedWithEntityTrigger() {
    if (gPlayerInteractedWithEntityListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerInteractedWithEntityListener);
        gPlayerInteractedWithEntityListener.reset();
    }
}

} // namespace advancements
