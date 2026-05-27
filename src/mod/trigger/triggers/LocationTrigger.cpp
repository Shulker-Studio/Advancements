#include "mod/trigger/triggers/LocationTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerTickEvent.h"
#include "mod/predicate/LocationPredicate.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include "mc/world/actor/player/Player.h"

#include <optional>
#include <string>
#include <unordered_map>

namespace advancements {
namespace {

constexpr int LocationStructureCheckIntervalTicks = 20;

struct LocationStructurePlayerState {
    int                        ticksUntilCheck{LocationStructureCheckIntervalTicks};
    std::optional<std::string> lastStructureId;
};

ll::event::ListenerPtr gPlayerTickListener;
std::unordered_map<mce::UUID, LocationStructurePlayerState> gLocationStructurePlayerStates;

void dispatchLocationStructure(Entry& mod, Player& player, std::string const& structureId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:location",
            LocationStructurePayload{structureId},
        }
    );
}

void checkLocationStructure(Entry& mod, Player& player) {
    auto& state = gLocationStructurePlayerStates[player.getUuid()];
    --state.ticksUntilCheck;
    if (state.ticksUntilCheck > 0) {
        return;
    }
    state.ticksUntilCheck = LocationStructureCheckIntervalTicks;

    auto const structureId = predicate::currentSupportedLocationStructure(player);
    if (!structureId) {
        state.lastStructureId.reset();
        return;
    }
    if (state.lastStructureId && *state.lastStructureId == *structureId) {
        return;
    }

    state.lastStructureId = structureId;
    dispatchLocationStructure(mod, player, *structureId);
}

} // namespace

bool locationTriggerRegistered() { return gPlayerTickListener != nullptr; }

void registerLocationTrigger(Entry& mod) {
    if (locationTriggerRegistered()) {
        return;
    }

    gPlayerTickListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerTickEvent>([&mod](auto& event) {
        checkLocationStructure(mod, event.self());
    });
}

void unregisterLocationTrigger() {
    gLocationStructurePlayerStates.clear();
    if (gPlayerTickListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerTickListener);
        gPlayerTickListener.reset();
    }
}

} // namespace advancements
