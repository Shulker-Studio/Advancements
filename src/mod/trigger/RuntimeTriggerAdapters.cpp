#include "mod/trigger/RuntimeTriggerAdapters.h"

#include "mod/Entry.h"
#include "mod/event/block/BeeNestDestroyedEvent.h"
#include "mod/event/block/TargetBlockHitEvent.h"
#include "mod/event/block/WitherSummonedEvent.h"
#include "mod/event/block/BeaconLevelChangedEvent.h"
#include "mod/event/block/SculkCatalystMobKilledEvent.h"
#include "mod/event/entity/PlayerBredAnimalsEvent.h"
#include "mod/event/entity/PlayerProjectileLightningHitEvent.h"
#include "mod/event/entity/PlayerTamedAnimalEvent.h"
#include "mod/event/item/ContainerOutputTakenEvent.h"
#include "mod/event/item/EnchantedItemEvent.h"
#include "mod/event/item/FishingRodHookedItemEvent.h"
#include "mod/event/item/PlayerGeneratedContainerLootEvent.h"
#include "mod/event/item/PlayerConsumedItemEvent.h"
#include "mod/event/item/PlayerFilledBucketEvent.h"
#include "mod/event/item/PlayerInventoryChangedEvent.h"
#include "mod/event/item/PlayerShotCrossbowEvent.h"
#include "mod/event/player/PlayerBlockUsingShieldEvent.h"
#include "mod/event/player/PlayerCuredZombieVillagerEvent.h"
#include "mod/event/player/DragonRespawnedEvent.h"
#include "mod/event/player/PlayerDimensionChangedEvent.h"
#include "mod/event/player/PlayerEnteredEndGatewayEvent.h"
#include "mod/event/player/PlayerEffectsChangedEvent.h"
#include "mod/event/player/PlayerInteractedWithEntityEvent.h"
#include "mod/event/player/PlayerSleptInBedEvent.h"
#include "mod/event/player/PlayerTickEvent.h"
#include "mod/event/player/PlayerUsedTotemEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"
#include "mod/trigger/TriggerDispatcher.h"
#include "mod/trigger/triggers/BredAnimalsTrigger.h"
#include "mod/trigger/triggers/BrewedPotionTrigger.h"
#include "mod/trigger/triggers/BeeNestDestroyedTrigger.h"
#include "mod/trigger/triggers/ChangedDimensionTrigger.h"
#include "mod/trigger/triggers/ChanneledLightningTrigger.h"
#include "mod/trigger/triggers/ConstructBeaconTrigger.h"
#include "mod/trigger/triggers/ConsumeItemTrigger.h"
#include "mod/trigger/triggers/CuredZombieVillagerTrigger.h"
#include "mod/trigger/triggers/EnterBlockTrigger.h"
#include "mod/trigger/triggers/EffectsChangedTrigger.h"
#include "mod/trigger/triggers/EnchantedItemTrigger.h"
#include "mod/trigger/triggers/EntityHurtPlayerTrigger.h"
#include "mod/trigger/triggers/EntityKilledPlayerTrigger.h"
#include "mod/trigger/triggers/FilledBucketTrigger.h"
#include "mod/trigger/triggers/FishingRodHookedTrigger.h"
#include "mod/trigger/triggers/InventoryChangedTrigger.h"
#include "mod/trigger/triggers/ItemUsedOnBlockTrigger.h"
#include "mod/trigger/triggers/KillMobNearSculkCatalystTrigger.h"
#include "mod/trigger/triggers/LevitationTrigger.h"
#include "mod/trigger/triggers/LocationTrigger.h"
#include "mod/trigger/triggers/NetherTravelTrigger.h"
#include "mod/trigger/triggers/PlayerGeneratedContainerLootTrigger.h"
#include "mod/trigger/triggers/PlayerInteractedWithEntityTrigger.h"
#include "mod/trigger/triggers/PlayerKilledEntityTrigger.h"
#include "mod/trigger/triggers/PlayerHurtEntityTrigger.h"
#include "mod/trigger/triggers/ShotCrossbowTrigger.h"
#include "mod/trigger/triggers/SleptInBedTrigger.h"
#include "mod/trigger/triggers/SummonedEntityTrigger.h"
#include "mod/trigger/triggers/TameAnimalTrigger.h"
#include "mod/trigger/triggers/TargetHitTrigger.h"
#include "mod/trigger/triggers/UsedTotemTrigger.h"
#include "mod/trigger/triggers/VillagerTradeTrigger.h"

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
                    "Advancements debug: trigger={} player={} loot_table={} generated_items={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.lootTableId,
                    payload.generatedItemIds.size()
                );
            } else if constexpr (std::is_same_v<Payload, PlayerHurtEntityPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} direct_arrow={} projectile={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.directEntityIsArrow,
                    payload.isProjectileDamage
                );
            } else if constexpr (std::is_same_v<Payload, EntityHurtPlayerPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} blocked={} projectile={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.blockedDamage,
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
            } else if constexpr (std::is_same_v<Payload, BeeNestDestroyedPayload>) {
                logger.debug(
                    "Advancements debug: trigger={} player={} block={} bees={}",
                    context.triggerId,
                    context.player.getRealName(),
                    payload.blockId,
                    payload.numBeesInside
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
    return event::player::playerTickEventSourceRegistered() || locationTriggerRegistered()
        || levitationTriggerRegistered()
        || channeledLightningTriggerRegistered()
        || changedDimensionTriggerRegistered()
        || netherTravelTriggerRegistered()
        || summonedEntityTriggerRegistered()
        || playerInteractedWithEntityTriggerRegistered()
        || curedZombieVillagerTriggerRegistered()
        || bredAnimalsTriggerRegistered() || tameAnimalTriggerRegistered()
        || entityHurtPlayerTriggerRegistered() || entityKilledPlayerTriggerRegistered() || playerHurtEntityTriggerRegistered()
        || playerKilledEntityTriggerRegistered() || targetHitTriggerRegistered() || brewedPotionTriggerRegistered()
        || enchantedItemTriggerRegistered() || villagerTradeTriggerRegistered() || usedTotemTriggerRegistered()
        || consumeItemTriggerRegistered() || inventoryChangedTriggerRegistered() || filledBucketTriggerRegistered()
        || fishingRodHookedTriggerRegistered() || shotCrossbowTriggerRegistered() || sleptInBedTriggerRegistered()
        || effectsChangedTriggerRegistered()
        || playerGeneratedContainerLootTriggerRegistered()
        || enterBlockTriggerRegistered()
        || itemUsedOnBlockTriggerRegistered()
        || constructBeaconTriggerRegistered()
        || beeNestDestroyedTriggerRegistered()
        || killMobNearSculkCatalystTriggerRegistered()
        || event::item::playerConsumedItemEventSourceRegistered()
        || event::item::playerInventoryChangedEventSourceRegistered()
        || event::item::playerFilledBucketEventSourceRegistered()
        || event::item::fishingRodHookedItemEventSourceRegistered()
        || event::item::playerGeneratedContainerLootEventSourceRegistered()
        || event::item::playerShotCrossbowEventSourceRegistered()
        || event::item::containerOutputTakenEventSourceRegistered() || event::item::enchantedItemEventSourceRegistered()
        || event::player::playerBlockUsingShieldEventSourceRegistered()
        || event::player::playerCuredZombieVillagerEventSourceRegistered()
        || event::player::playerDimensionChangedEventSourceRegistered()
        || event::player::playerEnteredEndGatewayEventSourceRegistered()
        || event::player::playerEffectsChangedEventSourceRegistered()
        || event::player::playerInteractedWithEntityEventSourceRegistered()
        || event::player::playerSleptInBedEventSourceRegistered()
        || event::player::playerUsedTotemEventSourceRegistered()
        || event::block::targetBlockHitEventSourceRegistered()
        || event::block::beeNestDestroyedEventSourceRegistered()
        || event::block::beaconLevelChangedEventSourceRegistered()
        || event::block::sculkCatalystMobKilledEventSourceRegistered()
        || event::block::witherSummonedEventSourceRegistered()
        || event::entity::playerBredAnimalsEventSourceRegistered()
        || event::entity::playerProjectileLightningHitEventSourceRegistered()
        || event::entity::playerTamedAnimalEventSourceRegistered()
        || event::player::dragonRespawnedEventSourceRegistered();
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
    event::block::registerBeaconLevelChangedEventSource();
    event::block::registerBeeNestDestroyedEventSource();
    event::block::registerSculkCatalystMobKilledEventSource();
    event::block::registerTargetBlockHitEventSource();
    event::block::registerWitherSummonedEventSource();
    event::item::registerContainerOutputTakenEventSource();
    event::item::registerEnchantedItemEventSource();
    event::item::registerPlayerFilledBucketEventSource();
    event::item::registerFishingRodHookedItemEventSource();
    event::item::registerPlayerGeneratedContainerLootEventSource();
    event::item::registerPlayerConsumedItemEventSource();
    event::item::registerPlayerInventoryChangedEventSource();
    event::item::registerPlayerShotCrossbowEventSource();
    event::entity::registerPlayerBredAnimalsEventSource();
    event::entity::registerPlayerProjectileLightningHitEventSource();
    event::entity::registerPlayerTamedAnimalEventSource();
    event::player::registerPlayerBlockUsingShieldEventSource();
    event::player::registerPlayerCuredZombieVillagerEventSource();
    event::player::registerDragonRespawnedEventSource();
    event::player::registerPlayerDimensionChangedEventSource();
    event::player::registerPlayerEnteredEndGatewayEventSource();
    event::player::registerPlayerEffectsChangedEventSource();
    event::player::registerPlayerInteractedWithEntityEventSource();
    event::player::registerPlayerSleptInBedEventSource();
    event::player::registerPlayerTickEventSource();
    event::player::registerPlayerUsedTotemEventSource();
    registerEntityHurtPlayerTrigger(mod);
    registerEntityKilledPlayerTrigger(mod);
    registerConstructBeaconTrigger(mod);
    registerBeeNestDestroyedTrigger(mod);
    registerKillMobNearSculkCatalystTrigger(mod);
    registerChanneledLightningTrigger(mod);
    registerChangedDimensionTrigger(mod);
    registerLocationTrigger(mod);
    registerLevitationTrigger(mod);
    registerNetherTravelTrigger(mod);
    registerSummonedEntityTrigger(mod);
    registerPlayerInteractedWithEntityTrigger(mod);
    registerCuredZombieVillagerTrigger(mod);
    registerBredAnimalsTrigger(mod);
    registerTameAnimalTrigger(mod);
    registerPlayerKilledEntityTrigger(mod);
    registerPlayerHurtEntityTrigger(mod);
    registerTargetHitTrigger(mod);
    registerBrewedPotionTrigger(mod);
    registerEnchantedItemTrigger(mod);
    registerVillagerTradeTrigger(mod);
    registerUsedTotemTrigger(mod);
    registerConsumeItemTrigger(mod);
    registerEnterBlockTrigger(mod);
    registerEffectsChangedTrigger(mod);
    registerItemUsedOnBlockTrigger(mod);
    registerInventoryChangedTrigger(mod);
    registerFilledBucketTrigger(mod);
    registerFishingRodHookedTrigger(mod);
    registerPlayerGeneratedContainerLootTrigger(mod);
    registerShotCrossbowTrigger(mod);
    registerSleptInBedTrigger(mod);
}

void unregisterRuntimeTriggerAdapters() {
    gRuntimeTriggerMod = nullptr;
    unregisterEntityHurtPlayerTrigger();
    unregisterEntityKilledPlayerTrigger();
    unregisterConstructBeaconTrigger();
    unregisterBeeNestDestroyedTrigger();
    unregisterKillMobNearSculkCatalystTrigger();
    unregisterChanneledLightningTrigger();
    unregisterChangedDimensionTrigger();
    unregisterLevitationTrigger();
    unregisterNetherTravelTrigger();
    unregisterSummonedEntityTrigger();
    unregisterPlayerInteractedWithEntityTrigger();
    unregisterCuredZombieVillagerTrigger();
    unregisterBredAnimalsTrigger();
    unregisterTameAnimalTrigger();
    unregisterPlayerKilledEntityTrigger();
    unregisterPlayerHurtEntityTrigger();
    unregisterTargetHitTrigger();
    unregisterBrewedPotionTrigger();
    unregisterEnchantedItemTrigger();
    unregisterVillagerTradeTrigger();
    unregisterUsedTotemTrigger();
    unregisterConsumeItemTrigger();
    unregisterEnterBlockTrigger();
    unregisterEffectsChangedTrigger();
    unregisterItemUsedOnBlockTrigger();
    unregisterInventoryChangedTrigger();
    unregisterFilledBucketTrigger();
    unregisterFishingRodHookedTrigger();
    unregisterPlayerGeneratedContainerLootTrigger();
    unregisterShotCrossbowTrigger();
    unregisterSleptInBedTrigger();
    unregisterLocationTrigger();
    event::block::unregisterBeaconLevelChangedEventSource();
    event::block::unregisterBeeNestDestroyedEventSource();
    event::block::unregisterSculkCatalystMobKilledEventSource();
    event::block::unregisterTargetBlockHitEventSource();
    event::block::unregisterWitherSummonedEventSource();
    event::item::unregisterContainerOutputTakenEventSource();
    event::item::unregisterEnchantedItemEventSource();
    event::item::unregisterPlayerFilledBucketEventSource();
    event::item::unregisterFishingRodHookedItemEventSource();
    event::item::unregisterPlayerGeneratedContainerLootEventSource();
    event::item::unregisterPlayerConsumedItemEventSource();
    event::item::unregisterPlayerInventoryChangedEventSource();
    event::item::unregisterPlayerShotCrossbowEventSource();
    event::entity::unregisterPlayerBredAnimalsEventSource();
    event::entity::unregisterPlayerProjectileLightningHitEventSource();
    event::entity::unregisterPlayerTamedAnimalEventSource();
    event::player::unregisterPlayerBlockUsingShieldEventSource();
    event::player::unregisterPlayerCuredZombieVillagerEventSource();
    event::player::unregisterDragonRespawnedEventSource();
    event::player::unregisterPlayerDimensionChangedEventSource();
    event::player::unregisterPlayerEnteredEndGatewayEventSource();
    event::player::unregisterPlayerEffectsChangedEventSource();
    event::player::unregisterPlayerInteractedWithEntityEventSource();
    event::player::unregisterPlayerSleptInBedEventSource();
    event::player::unregisterPlayerTickEventSource();
    event::player::unregisterPlayerUsedTotemEventSource();
}

} // namespace advancements
