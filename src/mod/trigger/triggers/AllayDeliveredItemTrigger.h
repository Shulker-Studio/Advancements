#pragma once

namespace advancements {
class Entry;

bool allayDeliveredItemTriggerRegistered();
void registerAllayDeliveredItemTrigger(Entry& mod);
void unregisterAllayDeliveredItemTrigger();

} // namespace advancements
