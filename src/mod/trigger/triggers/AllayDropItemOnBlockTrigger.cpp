#include "mod/trigger/triggers/AllayDropItemOnBlockTrigger.h"

#include "mod/Entry.h"
#include "mod/event/entity/PlayerAllayDroppedItemOnBlockEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerAllayDroppedItemOnBlockListener;

} // namespace

bool allayDropItemOnBlockTriggerRegistered() { return gPlayerAllayDroppedItemOnBlockListener != nullptr; }

void registerAllayDropItemOnBlockTrigger(Entry& mod) {
    if (allayDropItemOnBlockTriggerRegistered()) {
        return;
    }

    gPlayerAllayDroppedItemOnBlockListener =
        ll::event::EventBus::getInstance().emplaceListener<event::entity::PlayerAllayDroppedItemOnBlockEvent>(
            [&mod](auto& event) {
                dispatchTrigger(
                    mod,
                    TriggerContext{
                        event.player(),
                        "minecraft:allay_drop_item_on_block",
                        ItemUsedOnBlockPayload{event.itemId(), event.blockId()},
                    }
                );
            }
        );
}

void unregisterAllayDropItemOnBlockTrigger() {
    if (gPlayerAllayDroppedItemOnBlockListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerAllayDroppedItemOnBlockListener);
        gPlayerAllayDroppedItemOnBlockListener.reset();
    }
}

} // namespace advancements
