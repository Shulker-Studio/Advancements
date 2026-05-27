#include "mod/trigger/triggers/CuredZombieVillagerTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerCuredZombieVillagerEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gCuredZombieVillagerListener;

} // namespace

bool curedZombieVillagerTriggerRegistered() { return gCuredZombieVillagerListener != nullptr; }

void registerCuredZombieVillagerTrigger(Entry& mod) {
    if (curedZombieVillagerTriggerRegistered()) {
        return;
    }

    gCuredZombieVillagerListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerCuredZombieVillagerEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:cured_zombie_villager",
                    NoTriggerPayload{},
                }
            );
        }
    );
}

void unregisterCuredZombieVillagerTrigger() {
    if (gCuredZombieVillagerListener) {
        ll::event::EventBus::getInstance().removeListener(gCuredZombieVillagerListener);
        gCuredZombieVillagerListener.reset();
    }
}

} // namespace advancements
