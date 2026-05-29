#pragma once

namespace advancements {

class Entry;

bool beeNestDestroyedTriggerRegistered();
void registerBeeNestDestroyedTrigger(Entry& mod);
void unregisterBeeNestDestroyedTrigger();

} // namespace advancements
