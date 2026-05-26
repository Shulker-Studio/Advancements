#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileEntityCondition(nlohmann::json const& conditions);
TriggerCondition compilePlayerKilledEntityCondition(nlohmann::json const& conditions);
TriggerCondition compileSummonedEntityCondition(nlohmann::json const& conditions);
bool matchesEntityCondition(TriggerCondition const& condition, TriggerContext const& context);
bool matchesPlayerKilledEntityCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
