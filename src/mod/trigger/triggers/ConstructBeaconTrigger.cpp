#include "mod/trigger/triggers/ConstructBeaconTrigger.h"

#include "mod/Entry.h"
#include "mod/event/block/BeaconLevelChangedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"

#include <algorithm>

namespace advancements {
namespace {

ll::event::ListenerPtr gConstructBeaconListener;

constexpr float BeaconHorizontalRange    = 10.0F;
constexpr float BeaconVerticalRangeBelow = 9.0F;
constexpr float BeaconVerticalRangeAbove = 5.0F;

bool isWithinConstructBeaconRange(Player const& player, BlockSource const& region, BlockPos const& pos) {
    if (player.getDimensionId() != region.getDimensionId()) {
        return false;
    }

    auto const playerPos = player.getPosition();
    auto const beaconCenter = pos.center();
    auto const horizontalOffset = std::max(std::abs(playerPos.x - beaconCenter.x), std::abs(playerPos.z - beaconCenter.z));
    if (horizontalOffset > BeaconHorizontalRange) {
        return false;
    }

    return playerPos.y >= beaconCenter.y - BeaconVerticalRangeBelow && playerPos.y <= beaconCenter.y + BeaconVerticalRangeAbove;
}

} // namespace

bool constructBeaconTriggerRegistered() { return gConstructBeaconListener != nullptr; }

void registerConstructBeaconTrigger(Entry& mod) {
    if (constructBeaconTriggerRegistered()) {
        return;
    }

    gConstructBeaconListener = ll::event::EventBus::getInstance().emplaceListener<event::block::BeaconLevelChangedEvent>([&mod](auto& event) {
        auto* level = ll::service::getLevel().as_ptr();
        if (level == nullptr) {
            return;
        }

        level->forEachPlayer([&](Player& player) {
            if (isWithinConstructBeaconRange(player, event.region(), event.pos())) {
                dispatchTrigger(
                    mod,
                    TriggerContext{
                        player,
                        "minecraft:construct_beacon",
                        ConstructBeaconTriggerPayload{event.level()},
                    }
                );
            }
            return true;
        });
    });
}

void unregisterConstructBeaconTrigger() {
    if (gConstructBeaconListener) {
        ll::event::EventBus::getInstance().removeListener(gConstructBeaconListener);
        gConstructBeaconListener.reset();
    }
}

} // namespace advancements
