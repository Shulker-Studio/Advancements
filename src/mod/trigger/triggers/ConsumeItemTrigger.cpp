#include "mod/trigger/triggers/ConsumeItemTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/PlayerConsumedItemEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include <optional>

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerConsumedItemListener;

} // namespace

bool consumeItemTriggerRegistered() { return gPlayerConsumedItemListener != nullptr; }

void registerConsumeItemTrigger(Entry& mod) {
    if (consumeItemTriggerRegistered()) {
        return;
    }

    gPlayerConsumedItemListener = ll::event::EventBus::getInstance().emplaceListener<event::item::PlayerConsumedItemEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:consume_item",
                    ItemTriggerPayload{event.itemId(), std::nullopt},
                }
            );
        }
    );
}

void unregisterConsumeItemTrigger() {
    if (gPlayerConsumedItemListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerConsumedItemListener);
        gPlayerConsumedItemListener.reset();
    }
}

} // namespace advancements
