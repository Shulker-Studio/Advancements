#include "mod/trigger/triggers/EnchantedItemTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/ContainerOutputTakenEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gContainerOutputTakenListener;

} // namespace

bool enchantedItemTriggerRegistered() { return gContainerOutputTakenListener != nullptr; }

void registerEnchantedItemTrigger(Entry& mod) {
    if (enchantedItemTriggerRegistered()) {
        return;
    }

    gContainerOutputTakenListener = ll::event::EventBus::getInstance().emplaceListener<event::item::ContainerOutputTakenEvent>(
        [&mod](auto& event) {
            if (event.screenType() != SharedTypes::Legacy::ContainerType::Enchantment
                || event.sourceContainer() != ContainerEnumName::CreatedOutputContainer) {
                return;
            }

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
    if (gContainerOutputTakenListener) {
        ll::event::EventBus::getInstance().removeListener(gContainerOutputTakenListener);
        gContainerOutputTakenListener.reset();
    }
}

} // namespace advancements
