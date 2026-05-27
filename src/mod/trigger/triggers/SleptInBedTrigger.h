#pragma once

namespace advancements {

class Entry;

bool sleptInBedTriggerRegistered();
void registerSleptInBedTrigger(Entry& mod);
void unregisterSleptInBedTrigger();

} // namespace advancements
