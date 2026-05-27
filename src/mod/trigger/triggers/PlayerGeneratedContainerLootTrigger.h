#pragma once

namespace advancements {

class Entry;

bool playerGeneratedContainerLootTriggerRegistered();
void registerPlayerGeneratedContainerLootTrigger(Entry& mod);
void unregisterPlayerGeneratedContainerLootTrigger();

} // namespace advancements
