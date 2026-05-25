#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileChangedDimensionCondition(nlohmann::json const& conditions);
bool matchesChangedDimensionCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
