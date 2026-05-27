#pragma once

namespace advancements {

class Entry;

bool playerInteractedWithEntityTriggerRegistered();
void registerPlayerInteractedWithEntityTrigger(Entry& mod);
void unregisterPlayerInteractedWithEntityTrigger();

} // namespace advancements
