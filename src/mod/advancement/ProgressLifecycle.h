#pragma once

namespace advancements {

class Entry;

bool progressLifecycleRegistered();
void registerProgressLifecycle(Entry& mod);
void unregisterProgressLifecycle();

} // namespace advancements
