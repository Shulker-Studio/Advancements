#include "mod/trigger/criteria/DimensionCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileChangedDimensionCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"from", "to"})) {
        return InvalidTriggerCondition{};
    }
    std::optional<std::string> fromDimension;
    std::optional<std::string> toDimension;
    if (conditions.contains("from")) {
        if (!conditions.at("from").is_string()) {
            return InvalidTriggerCondition{};
        }
        fromDimension = conditions.at("from").get<std::string>();
    }
    if (conditions.contains("to")) {
        if (!conditions.at("to").is_string()) {
            return InvalidTriggerCondition{};
        }
        toDimension = conditions.at("to").get<std::string>();
    }
    return DimensionTriggerCondition{fromDimension, toDimension};
}

bool matchesChangedDimensionCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<DimensionTriggerCondition>(&condition);
    auto const* payload  = payloadAs<DimensionTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->fromDimension && payload->fromDimension != *compiled->fromDimension) {
        return false;
    }
    if (compiled->toDimension && payload->toDimension != *compiled->toDimension) {
        return false;
    }
    return true;
}

} // namespace advancements::criteria
