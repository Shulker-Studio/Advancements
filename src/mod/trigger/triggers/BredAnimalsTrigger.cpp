#include "mod/trigger/triggers/BredAnimalsTrigger.h"

#include "mod/Entry.h"
#include "mod/event/entity/PlayerBredAnimalsEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerBredAnimalsListener;

} // namespace

bool bredAnimalsTriggerRegistered() { return gPlayerBredAnimalsListener != nullptr; }

void registerBredAnimalsTrigger(Entry& mod) {
    if (bredAnimalsTriggerRegistered()) {
        return;
    }

    gPlayerBredAnimalsListener = ll::event::EventBus::getInstance().emplaceListener<event::entity::PlayerBredAnimalsEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:bred_animals",
                    BredAnimalsPayload{event.childTypeIds()},
                }
            );
        }
    );
}

void unregisterBredAnimalsTrigger() {
    if (gPlayerBredAnimalsListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerBredAnimalsListener);
        gPlayerBredAnimalsListener.reset();
    }
}

} // namespace advancements
