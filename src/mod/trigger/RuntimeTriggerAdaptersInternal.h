#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements {

class Entry;

Entry* currentRuntimeTriggerMod();
void   dispatchTrigger(Entry& mod, TriggerContext const& context);

bool inventoryRuntimeRegistered();
void registerInventoryRuntime();
void unregisterInventoryRuntime();

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
