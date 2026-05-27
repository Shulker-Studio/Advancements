#pragma once

namespace advancements {

class Entry;

bool enterBlockTriggerRegistered();
void registerEnterBlockTrigger(Entry& mod);
void unregisterEnterBlockTrigger();

} // namespace advancements
