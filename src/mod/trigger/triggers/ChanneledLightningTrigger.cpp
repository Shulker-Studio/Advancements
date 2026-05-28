#include "mod/trigger/triggers/ChanneledLightningTrigger.h"

#include "mod/Entry.h"
#include "mod/event/entity/PlayerProjectileLightningHitEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include <string>


namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerProjectileLightningHitListener;

bool isVillagerTypeId(std::string const& entityTypeId) {
    return entityTypeId == "minecraft:villager" || entityTypeId == "minecraft:villager_v2";
}

} // namespace

bool channeledLightningTriggerRegistered() { return gPlayerProjectileLightningHitListener != nullptr; }

void registerChanneledLightningTrigger(Entry& mod) {
    if (channeledLightningTriggerRegistered()) {
        return;
    }

    gPlayerProjectileLightningHitListener = ll::event::EventBus::getInstance().emplaceListener<event::entity::PlayerProjectileLightningHitEvent>(
        [&mod](auto& event) {
            if (event.projectileTypeId() != "minecraft:thrown_trident" || !event.projectileIsChanneling()
                || !isVillagerTypeId(event.struckEntityTypeId()) || !event.dimensionIsLightning()
                || !event.hitPositionCanSeeSky()) {
                return;
            }

            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:channeled_lightning",
                    EntityTriggerPayload{"minecraft:villager"},
                }
            );
        }
    );
}

void unregisterChanneledLightningTrigger() {
    if (gPlayerProjectileLightningHitListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerProjectileLightningHitListener);
        gPlayerProjectileLightningHitListener.reset();
    }
}

} // namespace advancements