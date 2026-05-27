#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements {

class Entry;

Entry* currentRuntimeTriggerMod();
void   dispatchTrigger(Entry& mod, TriggerContext const& context);

bool inventoryRuntimeRegistered();
void registerInventoryRuntime();
void unregisterInventoryRuntime();

bool locationTriggerRegistered();
void registerLocationTrigger(Entry& mod);
void unregisterLocationTrigger();

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

bool fishingRodHookedTriggerRegistered();
void registerFishingRodHookedTrigger(Entry& mod);
void unregisterFishingRodHookedTrigger();

bool combatRuntimeRegistered();
void registerCombatRuntime(Entry& mod);
void unregisterCombatRuntime();

bool worldRuntimeRegistered();
void registerWorldRuntime(Entry& mod);
void unregisterWorldRuntime();

bool lootRuntimeRegistered();
void registerLootRuntime();
void unregisterLootRuntime();

bool projectileRuntimeRegistered();
void registerProjectileRuntime();
void unregisterProjectileRuntime();

bool effectRuntimeRegistered();
void registerEffectRuntime();
void unregisterEffectRuntime();

} // namespace advancements
