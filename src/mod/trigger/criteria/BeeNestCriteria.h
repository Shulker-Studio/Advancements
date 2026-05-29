#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileBeeNestDestroyedCondition(nlohmann::json const& conditions);
bool matchesBeeNestDestroyedCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
