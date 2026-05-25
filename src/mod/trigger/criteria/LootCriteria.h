#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileLootTableCondition(nlohmann::json const& conditions);
bool matchesLootTableCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
