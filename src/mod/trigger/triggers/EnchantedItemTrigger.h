#pragma once

namespace advancements {

class Entry;

bool enchantedItemTriggerRegistered();
void registerEnchantedItemTrigger(Entry& mod);
void unregisterEnchantedItemTrigger();

} // namespace advancements
