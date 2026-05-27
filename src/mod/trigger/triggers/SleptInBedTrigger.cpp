#include "mod/trigger/triggers/SleptInBedTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerSleptInBedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gSleptInBedListener;

} // namespace

bool sleptInBedTriggerRegistered() { return gSleptInBedListener != nullptr; }

void registerSleptInBedTrigger(Entry& mod) {
    if (sleptInBedTriggerRegistered()) {
        return;
    }

    gSleptInBedListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerSleptInBedEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:slept_in_bed",
                    NoTriggerPayload{},
                }
            );
        }
    );
}

void unregisterSleptInBedTrigger() {
    if (gSleptInBedListener) {
        ll::event::EventBus::getInstance().removeListener(gSleptInBedListener);
        gSleptInBedListener.reset();
    }
}

} // namespace advancements
