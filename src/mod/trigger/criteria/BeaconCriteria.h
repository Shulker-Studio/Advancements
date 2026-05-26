#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileConstructBeaconCondition(nlohmann::json const& conditions);
bool matchesConstructBeaconCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
