#include "mod/trigger/criteria/NoCriteria.h"

namespace advancements::criteria {

TriggerCondition compileNoCondition(nlohmann::json const&) { return InvalidTriggerCondition{}; }

bool matchesNoCondition(TriggerCondition const& condition, TriggerContext const&) {
    return std::holds_alternative<NoTriggerCondition>(condition);
}

} // namespace advancements::criteria
