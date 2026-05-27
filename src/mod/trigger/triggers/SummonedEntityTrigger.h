#pragma once

namespace advancements {

class Entry;

bool summonedEntityTriggerRegistered();
void registerSummonedEntityTrigger(Entry& mod);
void unregisterSummonedEntityTrigger();

} // namespace advancements
