#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileEffectsChangedCondition(nlohmann::json const& conditions);
bool matchesEffectsChangedCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
