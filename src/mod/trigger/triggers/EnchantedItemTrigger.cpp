#include "mod/trigger/triggers/EnchantedItemTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/EnchantedItemEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gEnchantedItemListener;

} // namespace

bool enchantedItemTriggerRegistered() { return gEnchantedItemListener != nullptr; }

void registerEnchantedItemTrigger(Entry& mod) {
    if (enchantedItemTriggerRegistered()) {
        return;
    }

    gEnchantedItemListener = ll::event::EventBus::getInstance().emplaceListener<event::item::EnchantedItemEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:enchanted_item",
                    NoTriggerPayload{},
                }
            );
        }
    );
}

void unregisterEnchantedItemTrigger() {
    if (gEnchantedItemListener) {
        ll::event::EventBus::getInstance().removeListener(gEnchantedItemListener);
        gEnchantedItemListener.reset();
    }
}

} // namespace advancements
