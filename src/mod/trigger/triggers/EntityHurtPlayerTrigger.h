#pragma once

namespace advancements {

class Entry;

bool entityHurtPlayerTriggerRegistered();
void registerEntityHurtPlayerTrigger(Entry& mod);
void unregisterEntityHurtPlayerTrigger();

} // namespace advancements
