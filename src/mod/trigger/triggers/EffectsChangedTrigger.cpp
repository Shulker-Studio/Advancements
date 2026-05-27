#include "mod/trigger/triggers/EffectsChangedTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerEffectsChangedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gEffectsChangedListener;

} // namespace

bool effectsChangedTriggerRegistered() { return gEffectsChangedListener != nullptr; }

void registerEffectsChangedTrigger(Entry& mod) {
    if (effectsChangedTriggerRegistered()) {
        return;
    }

    gEffectsChangedListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerEffectsChangedEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:effects_changed",
                    EffectsChangedPayload{},
                }
            );
        }
    );
}

void unregisterEffectsChangedTrigger() {
    if (gEffectsChangedListener) {
        ll::event::EventBus::getInstance().removeListener(gEffectsChangedListener);
        gEffectsChangedListener.reset();
    }
}

} // namespace advancements
