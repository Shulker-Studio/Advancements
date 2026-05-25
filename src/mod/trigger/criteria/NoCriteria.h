#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileNoCondition(nlohmann::json const& conditions);
bool matchesNoCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
