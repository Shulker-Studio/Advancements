#pragma once

namespace advancements {

class Entry;

bool consumeItemTriggerRegistered();
void registerConsumeItemTrigger(Entry& mod);
void unregisterConsumeItemTrigger();

} // namespace advancements
