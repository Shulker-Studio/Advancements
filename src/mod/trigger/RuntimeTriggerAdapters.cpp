#include "mod/trigger/RuntimeTriggerAdapters.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerTickEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"
#include "mod/trigger/TriggerDispatcher.h"
#include "mod/trigger/triggers/LocationTrigger.h"

#include "mc/world/actor/player/Player.h"

#include <string>
#include <type_traits>
#include <variant>

namespace advancements {
namespace {

Entry* gRuntimeTriggerMod = nullptr;

void logTriggerDispatch(Entry& mod, TriggerContext const& context) {
    auto& logger = mod.getSelf().getLogger();
    std::visit(
        [&](auto const& payload) {
            using Payload = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<Payload, BlockTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} block={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.blockId
                );
            } else if constexpr (std::is_same_v<Payload, EnterBlockPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} entered_block={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.blockId
                );
            } else if constexpr (std::is_same_v<Payload, ItemUsedOnBlockPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} item={} block={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.itemId,
                    payload.blockId
                );
            } else if constexpr (std::is_same_v<Payload, ItemTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} item={} count={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.itemId,
                    payload.itemCount ? std::to_string(*payload.itemCount) : std::string{"-"}
                );
            } else if constexpr (std::is_same_v<Payload, EntityTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} entity={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.entityTypeId
                );
            } else if constexpr (std::is_same_v<Payload, PlayerInteractedWithEntityPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} item={} entity={} variant={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.itemId,
                    payload.entityTypeId,
                    payload.entityVariantId
                );
            } else if constexpr (std::is_same_v<Payload, DimensionTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} from={} to={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.fromDimension,
                    payload.toDimension
                );
            } else if constexpr (std::is_same_v<Payload, LocationStructurePayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} structure={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.structureId
                );
            } else if constexpr (std::is_same_v<Payload, LootTablePayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} loot_table={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.lootTableId
                );
            } else if constexpr (std::is_same_v<Payload, PlayerHurtEntityPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} direct_arrow={} projectile={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.directEntityIsArrow,
                    payload.isProjectileDamage
                );
            } else if constexpr (std::is_same_v<Payload, NetherTravelTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} horizontal_distance={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.horizontalDistance
                );
            } else if constexpr (std::is_same_v<Payload, LevitationTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} vertical_distance={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.verticalDistance
                );
            } else if constexpr (std::is_same_v<Payload, ConstructBeaconTriggerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} beacon_level={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.level
                );
            } else if constexpr (std::is_same_v<Payload, PlayerKilledEntitySniperDuelPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} killed_entity={} horizontal_distance={} killing_blow_projectile={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.killedEntityTypeId,
                    payload.horizontalDistance,
                    payload.killingBlowIsProjectile
                );
            }
        },
        context.payload
    );
}

bool anyRuntimeRegistered() {
    return inventoryRuntimeRegistered() || event::player::playerTickEventSourceRegistered() || locationTriggerRegistered()
        || combatRuntimeRegistered() || worldRuntimeRegistered() || lootRuntimeRegistered() || projectileRuntimeRegistered()
        || effectRuntimeRegistered();
}

} // namespace

Entry* currentRuntimeTriggerMod() { return gRuntimeTriggerMod; }

void dispatchTrigger(Entry& mod, TriggerContext const& context) {
    auto worldDataDir = mod.getSelf().getWorldDataDir();
    if (!worldDataDir) {
        return;
    }

    logTriggerDispatch(mod, context);

    auto dispatcher = TriggerDispatcher{mod.getTriggerIndex(), mod.getProgressService()};
    dispatcher.dispatch(*worldDataDir, mod.getAdvancementLoadResult(), context);
}

void registerRuntimeTriggerAdapters(Entry& mod) {
    if (gRuntimeTriggerMod != nullptr || anyRuntimeRegistered()) {
        return;
    }

    gRuntimeTriggerMod = &mod;
    event::player::registerPlayerTickEventSource();
    registerInventoryRuntime();
    registerLocationTrigger(mod);
    registerProjectileRuntime();
    registerWorldRuntime(mod);
    registerLootRuntime();
    registerCombatRuntime(mod);
    registerEffectRuntime();
}

void unregisterRuntimeTriggerAdapters() {
    gRuntimeTriggerMod = nullptr;
    unregisterLocationTrigger();
    unregisterInventoryRuntime();
    event::player::unregisterPlayerTickEventSource();
    unregisterProjectileRuntime();
    unregisterWorldRuntime();
    unregisterLootRuntime();
    unregisterCombatRuntime();
    unregisterEffectRuntime();
}

} // namespace advancements
