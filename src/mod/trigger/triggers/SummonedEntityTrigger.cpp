#include "mod/trigger/triggers/SummonedEntityTrigger.h"

#include "mod/Entry.h"
#include "mod/event/block/WitherSummonedEvent.h"
#include "mod/event/player/DragonRespawnedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

#include <algorithm>

namespace advancements {
namespace {

ll::event::ListenerPtr gWitherSummonedListener;
ll::event::ListenerPtr gDragonRespawnedListener;

constexpr float WitherSummonChebyshevRange   = 50.0F;
constexpr float RespawnDragonHorizontalRange = 192.0F;

bool isWithinWitherSummonRange(Player const& player, BlockSource const& region, Vec3 const& pos) {
    if (player.getDimensionId() != region.getDimensionId()) {
        return false;
    }

    auto const playerPos     = player.getPosition();
    auto const maxAxisOffset = std::max(
        {std::abs(playerPos.x - pos.x), std::abs(playerPos.y - pos.y), std::abs(playerPos.z - pos.z)}
    );
    return maxAxisOffset <= WitherSummonChebyshevRange;
}

bool isWithinRespawnDragonRange(Player const& player) {
    if (player.getDimensionId() != VanillaDimensions::TheEnd()) {
        return false;
    }

    auto const playerPos = player.getPosition();
    auto const horizontal = std::max(std::abs(playerPos.x), std::abs(playerPos.z));
    return horizontal <= RespawnDragonHorizontalRange;
}

} // namespace

bool summonedEntityTriggerRegistered() { return gWitherSummonedListener != nullptr || gDragonRespawnedListener != nullptr; }

void registerSummonedEntityTrigger(Entry& mod) {
    if (summonedEntityTriggerRegistered()) {
        return;
    }

    gWitherSummonedListener = ll::event::EventBus::getInstance().emplaceListener<event::block::WitherSummonedEvent>([&mod](auto& event) {
        auto* level = ll::service::getLevel().as_ptr();
        if (level == nullptr) {
            return;
        }

        level->forEachPlayer([&](Player& player) {
            if (isWithinWitherSummonRange(player, event.region(), event.pos())) {
                dispatchTrigger(
                    mod,
                    TriggerContext{
                        player,
                        "minecraft:summoned_entity",
                        EntityTriggerPayload{"minecraft:wither"},
                    }
                );
            }
            return true;
        });
    });

    gDragonRespawnedListener = ll::event::EventBus::getInstance().emplaceListener<event::player::DragonRespawnedEvent>([&mod](auto&) {
        auto* level = ll::service::getLevel().as_ptr();
        if (level == nullptr) {
            return;
        }

        level->forEachPlayer([&](Player& player) {
            if (isWithinRespawnDragonRange(player)) {
                dispatchTrigger(
                    mod,
                    TriggerContext{
                        player,
                        "minecraft:summoned_entity",
                        EntityTriggerPayload{"minecraft:ender_dragon"},
                    }
                );
            }
            return true;
        });
    });
}

void unregisterSummonedEntityTrigger() {
    if (gWitherSummonedListener) {
        ll::event::EventBus::getInstance().removeListener(gWitherSummonedListener);
        gWitherSummonedListener.reset();
    }
    if (gDragonRespawnedListener) {
        ll::event::EventBus::getInstance().removeListener(gDragonRespawnedListener);
        gDragonRespawnedListener.reset();
    }
}

} // namespace advancements
