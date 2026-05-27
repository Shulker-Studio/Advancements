#include "mod/trigger/triggers/VillagerTradeTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/ContainerOutputTakenEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gContainerOutputTakenListener;

} // namespace

bool villagerTradeTriggerRegistered() { return gContainerOutputTakenListener != nullptr; }

void registerVillagerTradeTrigger(Entry& mod) {
    if (villagerTradeTriggerRegistered()) {
        return;
    }

    gContainerOutputTakenListener = ll::event::EventBus::getInstance().emplaceListener<event::item::ContainerOutputTakenEvent>(
        [&mod](auto& event) {
            if (event.screenType() != SharedTypes::Legacy::ContainerType::Trade
                || event.sourceContainer() != ContainerEnumName::CreatedOutputContainer) {
                return;
            }

            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:villager_trade",
                    NoTriggerPayload{},
                }
            );
        }
    );
}

void unregisterVillagerTradeTrigger() {
    if (gContainerOutputTakenListener) {
        ll::event::EventBus::getInstance().removeListener(gContainerOutputTakenListener);
        gContainerOutputTakenListener.reset();
    }
}

} // namespace advancements
