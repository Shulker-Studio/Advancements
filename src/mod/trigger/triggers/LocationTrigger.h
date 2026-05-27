#pragma once

namespace advancements {

class Entry;

bool locationTriggerRegistered();
void registerLocationTrigger(Entry& mod);
void unregisterLocationTrigger();

} // namespace advancements
