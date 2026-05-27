#include "mod/trigger/triggers/FishingRodHookedTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/FishingRodHookedItemEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include <optional>

namespace advancements {
namespace {

ll::event::ListenerPtr gFishingRodHookedListener;

} // namespace

bool fishingRodHookedTriggerRegistered() { return gFishingRodHookedListener != nullptr; }

void registerFishingRodHookedTrigger(Entry& mod) {
    if (fishingRodHookedTriggerRegistered()) {
        return;
    }

    gFishingRodHookedListener = ll::event::EventBus::getInstance().emplaceListener<event::item::FishingRodHookedItemEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:fishing_rod_hooked",
                    ItemTriggerPayload{event.itemId(), std::nullopt},
                }
            );
        }
    );
}

void unregisterFishingRodHookedTrigger() {
    if (gFishingRodHookedListener) {
        ll::event::EventBus::getInstance().removeListener(gFishingRodHookedListener);
        gFishingRodHookedListener.reset();
    }
}

} // namespace advancements
