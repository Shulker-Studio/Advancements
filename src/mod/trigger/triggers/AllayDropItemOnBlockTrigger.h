#pragma once

namespace advancements {
class Entry;

bool allayDropItemOnBlockTriggerRegistered();
void registerAllayDropItemOnBlockTrigger(Entry& mod);
void unregisterAllayDropItemOnBlockTrigger();

} // namespace advancements
