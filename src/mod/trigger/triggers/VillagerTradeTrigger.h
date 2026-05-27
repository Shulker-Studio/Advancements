#pragma once

namespace advancements {

class Entry;

bool villagerTradeTriggerRegistered();
void registerVillagerTradeTrigger(Entry& mod);
void unregisterVillagerTradeTrigger();

} // namespace advancements
