#pragma once

namespace advancements {

class Entry;

bool itemUsedOnBlockTriggerRegistered();
void registerItemUsedOnBlockTrigger(Entry& mod);
void unregisterItemUsedOnBlockTrigger();

} // namespace advancements
