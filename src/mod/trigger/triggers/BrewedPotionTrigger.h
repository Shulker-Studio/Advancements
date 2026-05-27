#pragma once

namespace advancements {

class Entry;

bool brewedPotionTriggerRegistered();
void registerBrewedPotionTrigger(Entry& mod);
void unregisterBrewedPotionTrigger();

} // namespace advancements
