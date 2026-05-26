#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileLevitationCondition(nlohmann::json const& conditions);
bool matchesLevitationCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
