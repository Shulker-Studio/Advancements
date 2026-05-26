#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "mod/Entry.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BeaconBlockActor.h"
#include "mc/world/level/block/SkullBlock.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/end/EndDragonFight.h"
#include "mc/world/level/dimension/end/RespawnAnimation.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "mc/world/level/levelgen/structure/VanillaStructureFeatureType.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace advancements {
namespace {

ll::event::ListenerPtr gDestroyBlockListener;

constexpr int LocationStructureCheckIntervalTicks = 20;
constexpr float WitherSummonChebyshevRange       = 50.0F;
constexpr float RespawnDragonHorizontalRange     = 192.0F;
constexpr float BeaconHorizontalRange            = 10.0F;
constexpr float BeaconVerticalRangeBelow         = 9.0F;
constexpr float BeaconVerticalRangeAbove         = 5.0F;

struct LocationStructurePlayerState {
    int                        ticksUntilCheck{LocationStructureCheckIntervalTicks};
    std::optional<std::string> lastStructureId;
};

std::unordered_map<mce::UUID, LocationStructurePlayerState> gLocationStructurePlayerStates;
std::unordered_map<mce::UUID, Vec3>                         gNetherTravelStartPositions;

void dispatchSleptInBed(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:slept_in_bed",
            NoTriggerPayload{},
        }
    );
}

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

std::optional<std::string> currentSupportedLocationStructure(Player& player) {
    auto const& currentStructure = player.getCurrentStructureFeature();
    if (currentStructure == VanillaStructureFeatureType::Bastion()) {
        return "minecraft:bastion_remnant";
    }
    if (currentStructure == VanillaStructureFeatureType::Fortress()) {
        return "minecraft:fortress";
    }
    if (currentStructure == VanillaStructureFeatureType::EndCity()) {
        return "minecraft:end_city";
    }
    if (currentStructure == VanillaStructureFeatureType::Stronghold()) {
        return "minecraft:stronghold";
    }
    if (currentStructure == VanillaStructureFeatureType::TrialChambers()) {
        return "minecraft:trial_chambers";
    }
    return std::nullopt;
}

void checkLocationStructure(Entry& mod, Player& player) {
    auto& state = gLocationStructurePlayerStates[player.getUuid()];
    --state.ticksUntilCheck;
    if (state.ticksUntilCheck > 0) {
        return;
    }
    state.ticksUntilCheck = LocationStructureCheckIntervalTicks;

    auto const structureId = currentSupportedLocationStructure(player);
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

bool isWithinConstructBeaconRange(Player const& player, BlockSource const& region, BlockPos const& pos) {
    if (player.getDimensionId() != region.getDimensionId()) {
        return false;
    }

    auto const playerPos    = player.getPosition();
    auto const beaconCenter = pos.center();
    auto const horizontalOffset = std::max(std::abs(playerPos.x - beaconCenter.x), std::abs(playerPos.z - beaconCenter.z));
    if (horizontalOffset > BeaconHorizontalRange) {
        return false;
    }

    return playerPos.y >= beaconCenter.y - BeaconVerticalRangeBelow && playerPos.y <= beaconCenter.y + BeaconVerticalRangeAbove;
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

void dispatchConstructBeacon(Entry& mod, Level& level, BlockSource const& region, BlockPos const& pos, int beaconLevel) {
    level.forEachPlayer([&](Player& player) {
        if (isWithinConstructBeaconRange(player, region, pos)) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    player,
                    "minecraft:construct_beacon",
                    ConstructBeaconTriggerPayload{beaconLevel},
                }
            );
        }
        return true;
    });
}

LL_TYPE_INSTANCE_HOOK(PlayerTickWorldHook, HookPriority::Normal, Player, &Player::$tickWorld, void, Tick const& currentTick) {
    origin(currentTick);

    auto* mod = currentRuntimeTriggerMod();
    if (mod != nullptr) {
        checkLocationStructure(*mod, *this);
    }
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
    PlayerStartSleepInBedHook,
    HookPriority::Normal,
    Player,
    &Player::$startSleepInBed,
    BedSleepingResult,
    BlockPos const& pos
) {
    auto const result = origin(pos);
    if (result != BedSleepingResult::Ok) {
        return result;
    }

    auto* mod = currentRuntimeTriggerMod();
    if (mod != nullptr) {
        dispatchSleptInBed(*mod, *this);
    }

    return result;
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

LL_TYPE_INSTANCE_HOOK(BeaconBlockActorCheckShapeHook, HookPriority::Normal, BeaconBlockActor, &BeaconBlockActor::checkShape, void, BlockSource& region) {
    auto const previousLevel = this->mNumLevels;
    origin(region);

    auto const currentLevel = this->mNumLevels;
    if (currentLevel <= 0 || currentLevel <= previousLevel) {
        return;
    }

    auto* mod = currentRuntimeTriggerMod();
    if (mod == nullptr) {
        return;
    }

    auto* level = ll::service::getLevel().as_ptr();
    if (level == nullptr) {
        return;
    }

    dispatchConstructBeacon(*mod, *level, region, this->mPosition, currentLevel);
}

struct WorldRuntimeHookState {
    ll::memory::HookRegistrar<PlayerTickWorldHook>                 tickWorldHook;
    ll::memory::HookRegistrar<PlayerFireDimensionChangedEventHook> dimensionChangedEventHook;
    ll::memory::HookRegistrar<PlayerStartSleepInBedHook>           startSleepInBedHook;
    ll::memory::HookRegistrar<SkullBlockCheckMobSpawnHook>         skullBlockCheckMobSpawnHook;
    ll::memory::HookRegistrar<EndDragonFightTryRespawnHook>        endDragonFightTryRespawnHook;
    ll::memory::HookRegistrar<BeaconBlockActorCheckShapeHook>      beaconBlockActorCheckShapeHook;
};

std::unique_ptr<WorldRuntimeHookState> gWorldRuntimeHookState;

} // namespace

bool worldRuntimeRegistered() { return gDestroyBlockListener || gWorldRuntimeHookState != nullptr; }

void registerWorldRuntime(Entry& mod) {
    if (worldRuntimeRegistered()) {
        return;
    }

    (void)PlayerTickWorldHook::_AutoHookCount;
    (void)PlayerFireDimensionChangedEventHook::_AutoHookCount;
    (void)PlayerStartSleepInBedHook::_AutoHookCount;
    (void)SkullBlockCheckMobSpawnHook::_AutoHookCount;
    (void)EndDragonFightTryRespawnHook::_AutoHookCount;
    (void)BeaconBlockActorCheckShapeHook::_AutoHookCount;
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
    gLocationStructurePlayerStates.clear();
    gNetherTravelStartPositions.clear();
    gWorldRuntimeHookState.reset();

    auto& eventBus = ll::event::EventBus::getInstance();
    if (gDestroyBlockListener) {
        eventBus.removeListener(gDestroyBlockListener);
        gDestroyBlockListener.reset();
    }
}

} // namespace advancements
