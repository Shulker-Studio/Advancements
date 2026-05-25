#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compilePlayerHurtEntityCondition(nlohmann::json const& conditions);
TriggerCondition compileEntityHurtPlayerCondition(nlohmann::json const& conditions);
bool matchesPlayerHurtEntityCondition(TriggerCondition const& condition, TriggerContext const& context);
bool matchesEntityHurtPlayerCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
