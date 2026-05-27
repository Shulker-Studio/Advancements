#pragma once

namespace advancements {

class Entry;

bool entityKilledPlayerTriggerRegistered();
void registerEntityKilledPlayerTrigger(Entry& mod);
void unregisterEntityKilledPlayerTrigger();

} // namespace advancements
