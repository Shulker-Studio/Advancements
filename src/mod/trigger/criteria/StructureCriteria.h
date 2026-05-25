#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileLocationStructureCondition(nlohmann::json const& conditions);
bool matchesLocationStructureCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
