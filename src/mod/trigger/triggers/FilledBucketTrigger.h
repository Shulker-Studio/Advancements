#pragma once

namespace advancements {

class Entry;

bool filledBucketTriggerRegistered();
void registerFilledBucketTrigger(Entry& mod);
void unregisterFilledBucketTrigger();

} // namespace advancements
