#pragma once

namespace advancements {

class Entry;

bool effectsChangedTriggerRegistered();
void registerEffectsChangedTrigger(Entry& mod);
void unregisterEffectsChangedTrigger();

} // namespace advancements
