#include "mod/trigger/triggers/ChangedDimensionTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerDimensionChangedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include "mc/world/level/dimension/VanillaDimensions.h"

#include <string>

namespace advancements {
namespace {

ll::event::ListenerPtr gChangedDimensionListener;

std::string dimensionId(DimensionType dimension) {
    if (dimension == VanillaDimensions::Overworld()) {
        return "minecraft:overworld";
    }
    if (dimension == VanillaDimensions::Nether()) {
        return "minecraft:the_nether";
    }
    if (dimension == VanillaDimensions::TheEnd()) {
        return "minecraft:the_end";
    }
    return std::to_string(static_cast<int>(dimension));
}

} // namespace

bool changedDimensionTriggerRegistered() { return gChangedDimensionListener != nullptr; }

void registerChangedDimensionTrigger(Entry& mod) {
    if (changedDimensionTriggerRegistered()) {
        return;
    }

    gChangedDimensionListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerDimensionChangedEvent>(
        [&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.self(),
                    "minecraft:changed_dimension",
                    DimensionTriggerPayload{dimensionId(event.fromDimension()), dimensionId(event.toDimension())},
                }
            );
        }
    );
}

void unregisterChangedDimensionTrigger() {
    if (gChangedDimensionListener) {
        ll::event::EventBus::getInstance().removeListener(gChangedDimensionListener);
        gChangedDimensionListener.reset();
    }
}

} // namespace advancements
