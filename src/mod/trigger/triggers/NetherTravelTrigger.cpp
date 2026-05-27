#include "mod/trigger/triggers/NetherTravelTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerDimensionChangedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

#include <cmath>
#include <unordered_map>

namespace advancements {
namespace {

ll::event::ListenerPtr gNetherTravelListener;
std::unordered_map<mce::UUID, Vec3> gNetherTravelStartPositions;

float horizontalDistance(Vec3 const& lhs, Vec3 const& rhs) {
    auto const dx = lhs.x - rhs.x;
    auto const dz = lhs.z - rhs.z;
    return std::sqrt(dx * dx + dz * dz);
}

} // namespace

bool netherTravelTriggerRegistered() { return gNetherTravelListener != nullptr; }

void registerNetherTravelTrigger(Entry& mod) {
    if (netherTravelTriggerRegistered()) {
        return;
    }

    gNetherTravelListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerDimensionChangedEvent>([&mod](auto& event) {
        auto const fromDimension = event.fromDimension();
        auto const toDimension = event.toDimension();
        auto const playerId = event.self().getUuid();
        if (fromDimension == VanillaDimensions::Overworld() && toDimension == VanillaDimensions::Nether()) {
            gNetherTravelStartPositions[playerId] = event.positionBeforeChange();
            return;
        }

        if (fromDimension != VanillaDimensions::Nether()) {
            return;
        }

        auto const found = gNetherTravelStartPositions.find(playerId);
        if (found == gNetherTravelStartPositions.end()) {
            return;
        }

        if (toDimension == VanillaDimensions::Overworld()) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.self(),
                    "minecraft:nether_travel",
                    NetherTravelTriggerPayload{horizontalDistance(found->second, event.self().getPosition())},
                }
            );
        }
        gNetherTravelStartPositions.erase(found);
    });
}

void unregisterNetherTravelTrigger() {
    gNetherTravelStartPositions.clear();
    if (gNetherTravelListener) {
        ll::event::EventBus::getInstance().removeListener(gNetherTravelListener);
        gNetherTravelListener.reset();
    }
}

} // namespace advancements
