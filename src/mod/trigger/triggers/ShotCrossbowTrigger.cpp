#include "mod/trigger/triggers/ShotCrossbowTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/PlayerShotCrossbowEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include <optional>

namespace advancements {
namespace {

ll::event::ListenerPtr gShotCrossbowListener;

} // namespace

bool shotCrossbowTriggerRegistered() { return gShotCrossbowListener != nullptr; }

void registerShotCrossbowTrigger(Entry& mod) {
    if (shotCrossbowTriggerRegistered()) {
        return;
    }

    gShotCrossbowListener = ll::event::EventBus::getInstance().emplaceListener<event::item::PlayerShotCrossbowEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:shot_crossbow",
                    ItemTriggerPayload{"minecraft:crossbow", std::nullopt},
                }
            );
        }
    );
}

void unregisterShotCrossbowTrigger() {
    if (gShotCrossbowListener) {
        ll::event::EventBus::getInstance().removeListener(gShotCrossbowListener);
        gShotCrossbowListener.reset();
    }
}

} // namespace advancements
