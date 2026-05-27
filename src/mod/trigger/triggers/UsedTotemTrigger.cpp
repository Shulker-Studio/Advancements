#include "mod/trigger/triggers/UsedTotemTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerUsedTotemEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include <optional>

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerUsedTotemListener;

} // namespace

bool usedTotemTriggerRegistered() { return gPlayerUsedTotemListener != nullptr; }

void registerUsedTotemTrigger(Entry& mod) {
    if (usedTotemTriggerRegistered()) {
        return;
    }

    gPlayerUsedTotemListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerUsedTotemEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:used_totem",
                    ItemTriggerPayload{"minecraft:totem_of_undying", std::nullopt},
                }
            );
        }
    );
}

void unregisterUsedTotemTrigger() {
    if (gPlayerUsedTotemListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerUsedTotemListener);
        gPlayerUsedTotemListener.reset();
    }
}

} // namespace advancements
