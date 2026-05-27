#include "mod/trigger/criteria/StructureCriteria.h"

#include "mod/trigger/criteria/Common.h"
#include "mod/predicate/PlayerPredicate.h"

namespace advancements::criteria {

TriggerCondition compileLocationStructureCondition(nlohmann::json const& conditions) {
    auto const predicate = predicate::parseSinglePlayerLocationPredicate(conditions);
    if (!predicate || !predicate->location.structureId) {
        return InvalidTriggerCondition{};
    }
    return LocationStructureCondition{*predicate->location.structureId};
}

bool matchesLocationStructureCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<LocationStructureCondition>(&condition);
    auto const* payload  = payloadAs<LocationStructurePayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->structureId == compiled->structureId;
}

} // namespace advancements::criteria
