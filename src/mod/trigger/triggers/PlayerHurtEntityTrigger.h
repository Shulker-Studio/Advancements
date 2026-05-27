#pragma once

namespace advancements {

class Entry;

bool playerHurtEntityTriggerRegistered();
void registerPlayerHurtEntityTrigger(Entry& mod);
void unregisterPlayerHurtEntityTrigger();

} // namespace advancements
