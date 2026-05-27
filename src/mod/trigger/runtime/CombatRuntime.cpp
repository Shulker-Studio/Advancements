#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

namespace advancements {
bool combatRuntimeRegistered() { return false; }

void registerCombatRuntime([[maybe_unused]] Entry& mod) {}

void unregisterCombatRuntime() {}

} // namespace advancements
