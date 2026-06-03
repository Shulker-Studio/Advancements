#include "mod/trigger/triggers/AllayDeliveredItemTrigger.h"

#include "mod/Entry.h"
#include "mod/event/entity/PlayerAllayDeliveredItemEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerAllayDeliveredItemListener;

} // namespace

bool allayDeliveredItemTriggerRegistered() { return gPlayerAllayDeliveredItemListener != nullptr; }

void registerAllayDeliveredItemTrigger(Entry& mod) {
    if (allayDeliveredItemTriggerRegistered()) {
        return;
    }

    gPlayerAllayDeliveredItemListener = ll::event::EventBus::getInstance().emplaceListener<event::entity::PlayerAllayDeliveredItemEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:thrown_item_picked_up_by_player",
                    EntityTriggerPayload{"minecraft:allay"},
                }
            );
        }
    );
}

void unregisterAllayDeliveredItemTrigger() {
    if (gPlayerAllayDeliveredItemListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerAllayDeliveredItemListener);
        gPlayerAllayDeliveredItemListener.reset();
    }
}

} // namespace advancements
