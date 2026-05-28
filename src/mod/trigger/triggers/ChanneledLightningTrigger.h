#pragma once

namespace advancements {

class Entry;

bool channeledLightningTriggerRegistered();
void registerChanneledLightningTrigger(Entry& mod);
void unregisterChanneledLightningTrigger();

} // namespace advancements