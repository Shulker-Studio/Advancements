#include "mod/trigger/triggers/EnterBlockTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerEnteredEndGatewayEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gEnterBlockListener;

} // namespace

bool enterBlockTriggerRegistered() { return gEnterBlockListener != nullptr; }

void registerEnterBlockTrigger(Entry& mod) {
    if (enterBlockTriggerRegistered()) {
        return;
    }

    gEnterBlockListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerEnteredEndGatewayEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:enter_block",
                    EnterBlockPayload{"minecraft:end_gateway"},
                }
            );
        }
    );
}

void unregisterEnterBlockTrigger() {
    if (gEnterBlockListener) {
        ll::event::EventBus::getInstance().removeListener(gEnterBlockListener);
        gEnterBlockListener.reset();
    }
}

} // namespace advancements
