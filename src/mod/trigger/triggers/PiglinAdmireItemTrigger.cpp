#include "mod/trigger/triggers/PiglinAdmireItemTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/PlayerPiglinAdmireItemEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerPiglinAdmireItemListener;

} // namespace

bool piglinAdmireItemTriggerRegistered() { return gPlayerPiglinAdmireItemListener != nullptr; }

void registerPiglinAdmireItemTrigger(Entry& mod) {
    if (piglinAdmireItemTriggerRegistered()) {
        return;
    }

    gPlayerPiglinAdmireItemListener = ll::event::EventBus::getInstance().emplaceListener<event::item::PlayerPiglinAdmireItemEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:thrown_item_picked_up_by_entity",
                    PiglinAdmireItemPayload{event.itemId(), event.wasTargetingBarteringPlayer()},
                }
            );
        }
    );
}

void unregisterPiglinAdmireItemTrigger() {
    if (gPlayerPiglinAdmireItemListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerPiglinAdmireItemListener);
        gPlayerPiglinAdmireItemListener.reset();
    }
}

} // namespace advancements
