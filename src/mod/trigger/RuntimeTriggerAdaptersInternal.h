#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements {

class Entry;

Entry* currentRuntimeTriggerMod();
void   dispatchTrigger(Entry& mod, TriggerContext const& context);

bool locationTriggerRegistered();
void registerLocationTrigger(Entry& mod);
void unregisterLocationTrigger();

bool levitationTriggerRegistered();
void registerLevitationTrigger(Entry& mod);
void unregisterLevitationTrigger();

bool changedDimensionTriggerRegistered();
void registerChangedDimensionTrigger(Entry& mod);
void unregisterChangedDimensionTrigger();

bool netherTravelTriggerRegistered();
void registerNetherTravelTrigger(Entry& mod);
void unregisterNetherTravelTrigger();

bool summonedEntityTriggerRegistered();
void registerSummonedEntityTrigger(Entry& mod);
void unregisterSummonedEntityTrigger();

bool playerInteractedWithEntityTriggerRegistered();
void registerPlayerInteractedWithEntityTrigger(Entry& mod);
void unregisterPlayerInteractedWithEntityTrigger();

bool curedZombieVillagerTriggerRegistered();
void registerCuredZombieVillagerTrigger(Entry& mod);
void unregisterCuredZombieVillagerTrigger();

bool entityHurtPlayerTriggerRegistered();
void registerEntityHurtPlayerTrigger(Entry& mod);
void unregisterEntityHurtPlayerTrigger();

bool playerHurtEntityTriggerRegistered();
void registerPlayerHurtEntityTrigger(Entry& mod);
void unregisterPlayerHurtEntityTrigger();

bool playerKilledEntityTriggerRegistered();
void registerPlayerKilledEntityTrigger(Entry& mod);
void unregisterPlayerKilledEntityTrigger();

bool entityKilledPlayerTriggerRegistered();
void registerEntityKilledPlayerTrigger(Entry& mod);
void unregisterEntityKilledPlayerTrigger();

bool targetHitTriggerRegistered();
void registerTargetHitTrigger(Entry& mod);
void unregisterTargetHitTrigger();

bool brewedPotionTriggerRegistered();
void registerBrewedPotionTrigger(Entry& mod);
void unregisterBrewedPotionTrigger();

bool enchantedItemTriggerRegistered();
void registerEnchantedItemTrigger(Entry& mod);
void unregisterEnchantedItemTrigger();

bool villagerTradeTriggerRegistered();
void registerVillagerTradeTrigger(Entry& mod);
void unregisterVillagerTradeTrigger();

bool usedTotemTriggerRegistered();
void registerUsedTotemTrigger(Entry& mod);
void unregisterUsedTotemTrigger();

bool consumeItemTriggerRegistered();
void registerConsumeItemTrigger(Entry& mod);
void unregisterConsumeItemTrigger();

bool inventoryChangedTriggerRegistered();
void registerInventoryChangedTrigger(Entry& mod);
void unregisterInventoryChangedTrigger();

bool filledBucketTriggerRegistered();
void registerFilledBucketTrigger(Entry& mod);
void unregisterFilledBucketTrigger();

bool fishingRodHookedTriggerRegistered();
void registerFishingRodHookedTrigger(Entry& mod);
void unregisterFishingRodHookedTrigger();

bool shotCrossbowTriggerRegistered();
void registerShotCrossbowTrigger(Entry& mod);
void unregisterShotCrossbowTrigger();

bool sleptInBedTriggerRegistered();
void registerSleptInBedTrigger(Entry& mod);
void unregisterSleptInBedTrigger();

bool effectsChangedTriggerRegistered();
void registerEffectsChangedTrigger(Entry& mod);
void unregisterEffectsChangedTrigger();

bool playerGeneratedContainerLootTriggerRegistered();
void registerPlayerGeneratedContainerLootTrigger(Entry& mod);
void unregisterPlayerGeneratedContainerLootTrigger();

bool enterBlockTriggerRegistered();
void registerEnterBlockTrigger(Entry& mod);
void unregisterEnterBlockTrigger();

bool itemUsedOnBlockTriggerRegistered();
void registerItemUsedOnBlockTrigger(Entry& mod);
void unregisterItemUsedOnBlockTrigger();

bool constructBeaconTriggerRegistered();
void registerConstructBeaconTrigger(Entry& mod);
void unregisterConstructBeaconTrigger();

bool worldRuntimeRegistered();
void registerWorldRuntime(Entry& mod);
void unregisterWorldRuntime();

} // namespace advancements
