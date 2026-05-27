#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "mod/Entry.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/actor/BeaconBlockActor.h"
#include "mc/world/level/block/actor/EndGatewayBlockActor.h"
#include "mc/world/level/block/SkullBlock.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/end/EndDragonFight.h"
#include "mc/world/level/dimension/end/RespawnAnimation.h"
#include "mc/world/level/dimension/VanillaDimensions.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>

namespace advancements {
namespace {

ll::event::ListenerPtr gDestroyBlockListener;

constexpr float WitherSummonChebyshevRange       = 50.0F;
constexpr float RespawnDragonHorizontalRange     = 192.0F;
constexpr float BeaconHorizontalRange            = 10.0F;
constexpr float BeaconVerticalRangeBelow         = 9.0F;
constexpr float BeaconVerticalRangeAbove         = 5.0F;
std::unordered_map<mce::UUID, Vec3> gNetherTravelStartPositions;

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

void dispatchChangedDimension(Entry& mod, Player& player, DimensionType fromDimension, DimensionType toDimension) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:changed_dimension",
            DimensionTriggerPayload{dimensionId(fromDimension), dimensionId(toDimension)},
        }
    );
}

float horizontalDistance(Vec3 const& lhs, Vec3 const& rhs) {
    auto const dx = lhs.x - rhs.x;
    auto const dz = lhs.z - rhs.z;
    return std::sqrt(dx * dx + dz * dz);
}

void dispatchNetherTravel(Entry& mod, Player& player, float horizontalTravelDistance) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:nether_travel",
            NetherTravelTriggerPayload{horizontalTravelDistance},
        }
    );
}

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

void dispatchSummonedWither(Entry& mod, Level& level, BlockSource const& region, Vec3 const& pos) {
    level.forEachPlayer([&](Player& player) {
        if (isWithinWitherSummonRange(player, region, pos)) {
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
}

void dispatchRespawnDragon(Entry& mod, Level& level) {
    level.forEachPlayer([&](Player& player) {
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
}

LL_TYPE_INSTANCE_HOOK(
    PlayerFireDimensionChangedEventHook,
    HookPriority::Normal,
    Player,
    &Player::fireDimensionChangedEvent,
    void,
    DimensionType fromDimension,
    DimensionType toDimension
) {
    auto const positionBeforeChange = this->getPosition();
    origin(fromDimension, toDimension);

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }

    if (fromDimension != toDimension) {
        dispatchChangedDimension(*mod, *this, fromDimension, toDimension);

        auto const playerId = this->getUuid();
        if (fromDimension == VanillaDimensions::Overworld() && toDimension == VanillaDimensions::Nether()) {
            gNetherTravelStartPositions[playerId] = positionBeforeChange;
            return;
        }

        if (fromDimension == VanillaDimensions::Nether()) {
            auto const found = gNetherTravelStartPositions.find(playerId);
            if (found != gNetherTravelStartPositions.end()) {
                if (toDimension == VanillaDimensions::Overworld()) {
                    auto const travelled = horizontalDistance(found->second, this->getPosition());
                    dispatchNetherTravel(*mod, *this, travelled);
                }
                gNetherTravelStartPositions.erase(found);
            }
        }
    }
}

LL_TYPE_INSTANCE_HOOK(
    SkullBlockCheckMobSpawnHook,
    HookPriority::Normal,
    SkullBlock,
    &SkullBlock::checkMobSpawn,
    bool,
    ::Level&       level,
    ::BlockSource& region,
    ::BlockPos const& pos
) {
    auto const spawned = origin(level, region, pos);
    auto*      mod     = currentRuntimeTriggerMod();
    if (spawned && mod != nullptr) {
        dispatchSummonedWither(*mod, level, region, pos.center());
    }
    return spawned;
}

LL_TYPE_INSTANCE_HOOK(EndDragonFightTryRespawnHook, HookPriority::Normal, EndDragonFight, &EndDragonFight::tryRespawn, void) {
    auto const hadRespawnStage = this->mRespawnStage != RespawnAnimation::None;
    auto const hadCrystals     = !this->mRespawnCrystals->empty();
    origin();

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }

    auto const enteredRespawnStage = !hadRespawnStage && this->mRespawnStage != RespawnAnimation::None;
    auto const recordedCrystals    = !hadCrystals && !this->mRespawnCrystals->empty();
    if (!enteredRespawnStage && !recordedCrystals) {
        return;
    }

    auto* level = ll::service::getLevel().as_ptr();
    if (level == nullptr) {
        return;
    }
    dispatchRespawnDragon(*mod, *level);
}

struct WorldRuntimeHookState {
    ll::memory::HookRegistrar<PlayerFireDimensionChangedEventHook> dimensionChangedEventHook;
    ll::memory::HookRegistrar<SkullBlockCheckMobSpawnHook>         skullBlockCheckMobSpawnHook;
    ll::memory::HookRegistrar<EndDragonFightTryRespawnHook>        endDragonFightTryRespawnHook;
};

std::unique_ptr<WorldRuntimeHookState> gWorldRuntimeHookState;

} // namespace

bool worldRuntimeRegistered() { return gDestroyBlockListener || gWorldRuntimeHookState != nullptr; }

void registerWorldRuntime(Entry& mod) {
    if (worldRuntimeRegistered()) {
        return;
    }

    (void)PlayerFireDimensionChangedEventHook::_AutoHookCount;
    (void)SkullBlockCheckMobSpawnHook::_AutoHookCount;
    (void)EndDragonFightTryRespawnHook::_AutoHookCount;
    gWorldRuntimeHookState = std::make_unique<WorldRuntimeHookState>();

    auto& eventBus = ll::event::EventBus::getInstance();
    gDestroyBlockListener = eventBus.emplaceListener<ll::event::PlayerDestroyBlockEvent>([&mod](auto& event) {
        auto const& block = event.self().getDimensionBlockSource().getBlock(event.pos());
        dispatchTrigger(
            mod,
            TriggerContext{
                event.self(),
                "bedrock:player_destroy_block",
                BlockTriggerPayload{block.getTypeName()},
            }
        );
        return true;
    });
}

void unregisterWorldRuntime() {
    gNetherTravelStartPositions.clear();
    gWorldRuntimeHookState.reset();

    auto& eventBus = ll::event::EventBus::getInstance();
    if (gDestroyBlockListener) {
        eventBus.removeListener(gDestroyBlockListener);
        gDestroyBlockListener.reset();
    }
}

} // namespace advancements
