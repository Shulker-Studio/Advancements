#pragma once

namespace advancements {

class Entry;

bool placedBlockTriggerRegistered();
void registerPlacedBlockTrigger(Entry& mod);
void unregisterPlacedBlockTrigger();

} // namespace advancements
