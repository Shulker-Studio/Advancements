#include "mod/trigger/triggers/BeeNestDestroyedTrigger.h"

#include "mod/Entry.h"
#include "mod/event/block/BeeNestDestroyedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gBeeNestDestroyedListener;

} // namespace

bool beeNestDestroyedTriggerRegistered() { return gBeeNestDestroyedListener != nullptr; }

void registerBeeNestDestroyedTrigger(Entry& mod) {
    if (beeNestDestroyedTriggerRegistered()) {
        return;
    }

    gBeeNestDestroyedListener = ll::event::EventBus::getInstance().emplaceListener<event::block::BeeNestDestroyedEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:bee_nest_destroyed",
                    BeeNestDestroyedPayload{
                        event.blockId(),
                        event.numBeesInside(),
                    },
                }
            );
        }
    );
}

void unregisterBeeNestDestroyedTrigger() {
    if (gBeeNestDestroyedListener) {
        ll::event::EventBus::getInstance().removeListener(gBeeNestDestroyedListener);
        gBeeNestDestroyedListener.reset();
    }
}

} // namespace advancements
