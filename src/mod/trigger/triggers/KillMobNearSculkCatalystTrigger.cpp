#include "mod/trigger/triggers/KillMobNearSculkCatalystTrigger.h"

#include "mod/Entry.h"
#include "mod/event/block/SculkCatalystMobKilledEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gKillMobNearSculkCatalystListener;

} // namespace

bool killMobNearSculkCatalystTriggerRegistered() { return gKillMobNearSculkCatalystListener != nullptr; }

void registerKillMobNearSculkCatalystTrigger(Entry& mod) {
    if (killMobNearSculkCatalystTriggerRegistered()) {
        return;
    }

    gKillMobNearSculkCatalystListener = ll::event::EventBus::getInstance().emplaceListener<event::block::SculkCatalystMobKilledEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:kill_mob_near_sculk_catalyst",
                    NoTriggerPayload{},
                }
            );
        }
    );
}

void unregisterKillMobNearSculkCatalystTrigger() {
    if (gKillMobNearSculkCatalystListener) {
        ll::event::EventBus::getInstance().removeListener(gKillMobNearSculkCatalystListener);
        gKillMobNearSculkCatalystListener.reset();
    }
}

} // namespace advancements
