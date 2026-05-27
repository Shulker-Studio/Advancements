#pragma once

namespace advancements {

class Entry;

bool usedTotemTriggerRegistered();
void registerUsedTotemTrigger(Entry& mod);
void unregisterUsedTotemTrigger();

} // namespace advancements
