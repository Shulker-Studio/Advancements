#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDestroyBlockEvent.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/dimension/VanillaDimensions.h"
#include "mc/world/level/levelgen/structure/VanillaStructureFeatureType.h"

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace advancements {
namespace {

ll::event::ListenerPtr gDestroyBlockListener;

constexpr int LocationStructureCheckIntervalTicks = 20;

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

struct WorldRuntimeHookState {
    ll::memory::HookRegistrar<PlayerTickWorldHook>                 tickWorldHook;
    ll::memory::HookRegistrar<PlayerFireDimensionChangedEventHook> dimensionChangedEventHook;
    ll::memory::HookRegistrar<PlayerStartSleepInBedHook>           startSleepInBedHook;
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
