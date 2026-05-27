#include "mod/event/player/PlayerCuredZombieVillagerEvent.h"

#include "mod/event/player/PlayerTickEvent.h"

#include "ll/api/event/Emitter.h"
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
#include <unordered_map>

namespace advancements::event::player {
namespace {

constexpr int CureZombieVillagerMaxTrackedTicks = 20 * 60 * 6;

struct PendingZombieVillagerCure {
    ActorUniqueID zombieVillagerId;
    mce::UUID     playerId;
    int           ticksRemaining{CureZombieVillagerMaxTrackedTicks};
};

std::unordered_map<int64, PendingZombieVillagerCure> gPendingZombieVillagerCures;
ll::event::ListenerPtr                               gPlayerTickListener;

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
    PlayerInteractEntityForCureEventHook,
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
    ZombieVillagerMaintainOldDataEventHook,
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
    auto const originalRuntimeType = originalActor.getEntityTypeId();
    auto const originalUniqueId = originalActor.getOrCreateUniqueID().rawID;
    auto const maybeTrackedCure = isZombieVillagerActorType(originalRuntimeType)
                               ? gPendingZombieVillagerCures.find(originalUniqueId)
                               : gPendingZombieVillagerCures.end();

    origin(originalActor, transformed, transformation, ownerID, level);

    if (maybeTrackedCure == gPendingZombieVillagerCures.end()) {
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

    PlayerCuredZombieVillagerEvent event{*player};
    ll::event::EventBus::getInstance().publish(event);
    gPendingZombieVillagerCures.erase(maybeTrackedCure);
}

std::unique_ptr<ll::event::EmitterBase> playerCuredZombieVillagerEventEmitterFactory();

class PlayerCuredZombieVillagerEventEmitter
    : public ll::event::Emitter<playerCuredZombieVillagerEventEmitterFactory, PlayerCuredZombieVillagerEvent> {
public:
    PlayerCuredZombieVillagerEventEmitter() {
        gPlayerTickListener = ll::event::EventBus::getInstance().emplaceListener<PlayerTickEvent>([](auto& event) {
            if (!gPendingZombieVillagerCures.empty()) {
                checkPendingZombieVillagerCures(event.self());
            }
        });
    }

    ~PlayerCuredZombieVillagerEventEmitter() override {
        gPendingZombieVillagerCures.clear();
        if (gPlayerTickListener) {
            ll::event::EventBus::getInstance().removeListener(gPlayerTickListener);
            gPlayerTickListener.reset();
        }
    }

private:
    ll::memory::HookRegistrar<PlayerInteractEntityForCureEventHook> playerInteractEntityForCureHook;
    ll::memory::HookRegistrar<ZombieVillagerMaintainOldDataEventHook> zombieVillagerMaintainOldDataHook;
};

std::unique_ptr<ll::event::EmitterBase> playerCuredZombieVillagerEventEmitterFactory() {
    return std::make_unique<PlayerCuredZombieVillagerEventEmitter>();
}

bool gPlayerCuredZombieVillagerEventSourceRegistered = false;

} // namespace

bool playerCuredZombieVillagerEventSourceRegistered() { return gPlayerCuredZombieVillagerEventSourceRegistered; }

void registerPlayerCuredZombieVillagerEventSource() {
    if (playerCuredZombieVillagerEventSourceRegistered()) {
        return;
    }

    (void)PlayerInteractEntityForCureEventHook::_AutoHookCount;
    (void)ZombieVillagerMaintainOldDataEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerCuredZombieVillagerEvent>(playerCuredZombieVillagerEventEmitterFactory);
    gPlayerCuredZombieVillagerEventSourceRegistered = true;
}

void unregisterPlayerCuredZombieVillagerEventSource() { gPlayerCuredZombieVillagerEventSourceRegistered = false; }

} // namespace advancements::event::player
