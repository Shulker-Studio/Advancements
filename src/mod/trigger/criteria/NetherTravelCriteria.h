#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileNetherTravelCondition(nlohmann::json const& conditions);
bool matchesNetherTravelCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
