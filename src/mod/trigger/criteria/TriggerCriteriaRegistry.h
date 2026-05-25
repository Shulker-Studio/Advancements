#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerDescriptor const* findTriggerDescriptor(std::string_view triggerId);

} // namespace advancements::criteria
