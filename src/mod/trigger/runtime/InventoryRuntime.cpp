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

constexpr int  CureZombieVillagerMaxTrackedTicks    = 20 * 60 * 6;
constexpr int  TemperateFrogVariant                 = 0;
constexpr int  ColdFrogVariant                      = 1;
constexpr int  WarmFrogVariant                      = 2;

struct PendingZombieVillagerCure {
    ActorUniqueID zombieVillagerId;
    mce::UUID     playerId;
    int           ticksRemaining{CureZombieVillagerMaxTrackedTicks};
};

std::unordered_map<int64, PendingZombieVillagerCure> gPendingZombieVillagerCures;
ll::event::ListenerPtr                               gPlayerTickListener;

void dispatchCuredZombieVillager(Entry& mod, Player& player) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:cured_zombie_villager",
            NoTriggerPayload{},
        }
    );
}

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

bool isZombieVillagerActorType(ActorType actorType) {
    return actorType == ActorType::ZombieVillager || actorType == ActorType::ZombieVillagerV2;
}

bool isZombieVillagerCureInteraction(Actor& actor) {
    if (!isZombieVillagerActorType(actor.getEntityTypeId())) {
        return false;
    }
    return static_cast<ZombieVillager&>(actor).villagerConversionTime > 0;
}

void trackZombieVillagerCure(Player& player, Actor& actor) {
    auto const zombieVillagerId = actor.getOrCreateUniqueID();
    gPendingZombieVillagerCures[zombieVillagerId.rawID] = PendingZombieVillagerCure{
        zombieVillagerId,
        player.getUuid(),
    };
}

void checkPendingZombieVillagerCures(Player& player) {
    for (auto it = gPendingZombieVillagerCures.begin(); it != gPendingZombieVillagerCures.end();) {
        auto& pending = it->second;
        if (pending.playerId != player.getUuid()) {
            ++it;
            continue;
        }

        --pending.ticksRemaining;
        if (pending.ticksRemaining <= 0) {
            it = gPendingZombieVillagerCures.erase(it);
        }
        else {
            ++it;
        }
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
    auto const mayStartZombieVillagerCure = isZombieVillagerActorType(actor.getEntityTypeId())
                                         && !isZombieVillagerCureInteraction(actor)
                                         && !getSelectedItem().isNull()
                                         && getSelectedItem().getTypeName() == "minecraft:golden_apple";
    auto result = origin(actor, location);

    if (!result.mSuccess && !result.mSwing) {
        return result;
    }

    if (mayStartZombieVillagerCure) {
        trackZombieVillagerCure(*this, actor);
    }

    return result;
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
    auto const originalType = originalActor.getTypeName();
    auto const transformedType = transformed.getTypeName();
    auto const originalRuntimeType = originalActor.getEntityTypeId();
    auto const originalUniqueId = originalActor.getOrCreateUniqueID().rawID;
    auto const maybeTrackedCure = isZombieVillagerActorType(originalRuntimeType)
                               ? gPendingZombieVillagerCures.find(originalUniqueId)
                               : gPendingZombieVillagerCures.end();

    auto* mod = currentRuntimeTriggerMod();
    origin(originalActor, transformed, transformation, ownerID, level);

    if (mod == nullptr || maybeTrackedCure == gPendingZombieVillagerCures.end()) {
        return;
    }

    auto const transformedRuntimeType = transformed.getEntityTypeId();
    if (transformedRuntimeType != ActorType::Villager && transformedRuntimeType != ActorType::VillagerV2) {
        return;
    }

    auto* player = level.getPlayer(maybeTrackedCure->second.playerId);
    if (player == nullptr) {
        return;
    }

    dispatchCuredZombieVillager(*mod, *player);
    gPendingZombieVillagerCures.erase(maybeTrackedCure);
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
    gPlayerTickListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerTickEvent>([](auto& event) {
        auto* mod = currentRuntimeTriggerMod();
        if (mod != nullptr && !gPendingZombieVillagerCures.empty()) {
            checkPendingZombieVillagerCures(event.self());
        }
    });
}

void unregisterInventoryRuntime() {
    gPendingZombieVillagerCures.clear();
    gInventoryRuntimeHookState.reset();
    if (gPlayerTickListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerTickListener);
        gPlayerTickListener.reset();
    }
}

} // namespace advancements
