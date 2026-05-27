#include "mod/trigger/triggers/ItemUsedOnBlockTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerChargedRespawnAnchorEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gItemUsedOnBlockListener;

} // namespace

bool itemUsedOnBlockTriggerRegistered() { return gItemUsedOnBlockListener != nullptr; }

void registerItemUsedOnBlockTrigger(Entry& mod) {
    if (itemUsedOnBlockTriggerRegistered()) {
        return;
    }

    gItemUsedOnBlockListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerChargedRespawnAnchorEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:item_used_on_block",
                    ItemUsedOnBlockPayload{"minecraft:glowstone", "minecraft:respawn_anchor"},
                }
            );
        }
    );
}

void unregisterItemUsedOnBlockTrigger() {
    if (gItemUsedOnBlockListener) {
        ll::event::EventBus::getInstance().removeListener(gItemUsedOnBlockListener);
        gItemUsedOnBlockListener.reset();
    }
}

} // namespace advancements
