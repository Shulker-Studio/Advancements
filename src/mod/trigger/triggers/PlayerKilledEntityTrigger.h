#pragma once

namespace advancements {

class Entry;

bool playerKilledEntityTriggerRegistered();
void registerPlayerKilledEntityTrigger(Entry& mod);
void unregisterPlayerKilledEntityTrigger();

} // namespace advancements
