#pragma once

namespace advancements {

class Entry;

bool curedZombieVillagerTriggerRegistered();
void registerCuredZombieVillagerTrigger(Entry& mod);
void unregisterCuredZombieVillagerTrigger();

} // namespace advancements
