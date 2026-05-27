#include "mod/trigger/triggers/FilledBucketTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/PlayerFilledBucketEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include <optional>

namespace advancements {
namespace {

ll::event::ListenerPtr gFilledBucketListener;

} // namespace

bool filledBucketTriggerRegistered() { return gFilledBucketListener != nullptr; }

void registerFilledBucketTrigger(Entry& mod) {
    if (filledBucketTriggerRegistered()) {
        return;
    }

    gFilledBucketListener = ll::event::EventBus::getInstance().emplaceListener<event::item::PlayerFilledBucketEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:filled_bucket",
                    ItemTriggerPayload{event.itemId(), std::nullopt},
                }
            );
        }
    );
}

void unregisterFilledBucketTrigger() {
    if (gFilledBucketListener) {
        ll::event::EventBus::getInstance().removeListener(gFilledBucketListener);
        gFilledBucketListener.reset();
    }
}

} // namespace advancements
