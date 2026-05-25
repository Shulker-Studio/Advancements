#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileBlockCondition(nlohmann::json const& conditions);
bool matchesBlockCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
