#pragma once

#include "mod/trigger/TriggerIndex.h"

namespace advancements::criteria {

TriggerCondition compileInventoryItemCondition(nlohmann::json const& conditions);
TriggerCondition compileSimpleItemCondition(nlohmann::json const& conditions);
TriggerCondition compileItemUsedOnBlockCondition(nlohmann::json const& conditions);
TriggerCondition compileShotCrossbowCondition(nlohmann::json const& conditions);
bool matchesInventoryItemCondition(TriggerCondition const& condition, TriggerContext const& context);
bool matchesSimpleItemCondition(TriggerCondition const& condition, TriggerContext const& context);
bool matchesItemUsedOnBlockCondition(TriggerCondition const& condition, TriggerContext const& context);

} // namespace advancements::criteria
