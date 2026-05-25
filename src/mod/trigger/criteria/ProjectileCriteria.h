#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileTargetHitCondition(nlohmann::json const& conditions);
bool matchesTargetHitCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
