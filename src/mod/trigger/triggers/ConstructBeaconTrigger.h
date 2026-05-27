#pragma once

namespace advancements {

class Entry;

bool constructBeaconTriggerRegistered();
void registerConstructBeaconTrigger(Entry& mod);
void unregisterConstructBeaconTrigger();

} // namespace advancements
