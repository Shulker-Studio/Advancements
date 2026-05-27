#pragma once

namespace advancements {

class Entry;

bool inventoryChangedTriggerRegistered();
void registerInventoryChangedTrigger(Entry& mod);
void unregisterInventoryChangedTrigger();

} // namespace advancements
