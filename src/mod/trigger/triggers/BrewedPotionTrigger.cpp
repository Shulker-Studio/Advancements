#include "mod/trigger/triggers/BrewedPotionTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/ContainerOutputTakenEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gContainerOutputTakenListener;

} // namespace

bool brewedPotionTriggerRegistered() { return gContainerOutputTakenListener != nullptr; }

void registerBrewedPotionTrigger(Entry& mod) {
    if (brewedPotionTriggerRegistered()) {
        return;
    }

    gContainerOutputTakenListener = ll::event::EventBus::getInstance().emplaceListener<event::item::ContainerOutputTakenEvent>(
        [&mod](auto& event) {
            if (event.screenType() != SharedTypes::Legacy::ContainerType::BrewingStand
                || event.sourceContainer() != ContainerEnumName::BrewingStandResultContainer) {
                return;
            }

            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:brewed_potion",
                    NoTriggerPayload{},
                }
            );
        }
    );
}

void unregisterBrewedPotionTrigger() {
    if (gContainerOutputTakenListener) {
        ll::event::EventBus::getInstance().removeListener(gContainerOutputTakenListener);
        gContainerOutputTakenListener.reset();
    }
}

} // namespace advancements
