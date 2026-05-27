#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "mod/event/player/PlayerTickEvent.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/entity/components_json_legacy/TransformationComponent.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/monster/ZombieVillager.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/gamemode/InteractionResult.h"
#include "mc/world/level/Level.h"

#include <mc/legacy/ActorUniqueID.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace advancements {
namespace {

constexpr int  TemperateFrogVariant                 = 0;
constexpr int  ColdFrogVariant                      = 1;
constexpr int  WarmFrogVariant                      = 2;

std::optional<std::string> frogVariantIdForVariant(int variant) {
    switch (variant) {
    case TemperateFrogVariant:
        return std::string{"minecraft:temperate"};
    case ColdFrogVariant:
        return std::string{"minecraft:cold"};
    case WarmFrogVariant:
        return std::string{"minecraft:warm"};
    default:
        return std::nullopt;
    }
}

LL_TYPE_INSTANCE_HOOK(
    PlayerInteractEntityHook,
    HookPriority::Normal,
    Player,
    &Player::interact,
    InteractionResult,
    Actor&      actor,
    Vec3 const& location
) {
    return origin(actor, location);
}

LL_TYPE_INSTANCE_HOOK(
    ZombieVillagerMaintainOldDataHook,
    HookPriority::Normal,
    TransformationComponent,
    &TransformationComponent::maintainOldData,
    void,
    Actor&                           originalActor,
    Actor&                           transformed,
    TransformationDescription const& transformation,
    ActorUniqueID const&             ownerID,
    Level const&                     level
) {
    origin(originalActor, transformed, transformation, ownerID, level);
}

struct InventoryRuntimeHookState {
    ll::memory::HookRegistrar<ZombieVillagerMaintainOldDataHook> zombieVillagerMaintainOldDataHook;
};

std::unique_ptr<InventoryRuntimeHookState> gInventoryRuntimeHookState;

} // namespace

bool inventoryRuntimeRegistered() { return gInventoryRuntimeHookState != nullptr; }

void registerInventoryRuntime() {
    if (gInventoryRuntimeHookState) {
        return;
    }

    (void)ZombieVillagerMaintainOldDataHook::_AutoHookCount;
    gInventoryRuntimeHookState = std::make_unique<InventoryRuntimeHookState>();
}

void unregisterInventoryRuntime() {
    gInventoryRuntimeHookState.reset();
}

} // namespace advancements
