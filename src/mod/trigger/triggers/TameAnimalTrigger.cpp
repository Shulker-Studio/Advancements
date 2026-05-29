#include "mod/trigger/triggers/TameAnimalTrigger.h"

#include "mod/Entry.h"
#include "mod/event/entity/PlayerTamedAnimalEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerTamedAnimalListener;

} // namespace

bool tameAnimalTriggerRegistered() { return gPlayerTamedAnimalListener != nullptr; }

void registerTameAnimalTrigger(Entry& mod) {
    if (tameAnimalTriggerRegistered()) {
        return;
    }

    gPlayerTamedAnimalListener = ll::event::EventBus::getInstance().emplaceListener<event::entity::PlayerTamedAnimalEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:tame_animal",
                    NoTriggerPayload{},
                }
            );
        }
    );
}

void unregisterTameAnimalTrigger() {
    if (gPlayerTamedAnimalListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerTamedAnimalListener);
        gPlayerTamedAnimalListener.reset();
    }
}

} // namespace advancements
