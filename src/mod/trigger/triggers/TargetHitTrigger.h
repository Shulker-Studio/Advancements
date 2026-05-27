#pragma once

namespace advancements {

class Entry;

bool targetHitTriggerRegistered();
void registerTargetHitTrigger(Entry& mod);
void unregisterTargetHitTrigger();

} // namespace advancements
