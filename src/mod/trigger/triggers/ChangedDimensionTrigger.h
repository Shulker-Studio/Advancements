#pragma once

namespace advancements {

class Entry;

bool changedDimensionTriggerRegistered();
void registerChangedDimensionTrigger(Entry& mod);
void unregisterChangedDimensionTrigger();

} // namespace advancements
