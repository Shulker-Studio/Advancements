#include "mod/trigger/criteria/LevitationCriteria.h"

#include "mod/trigger/criteria/Common.h"

#include <cmath>

namespace advancements::criteria {

TriggerCondition compileLevitationCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"distance"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("distance") || !conditions.at("distance").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& distance = conditions.at("distance");
    if (!hasOnlyKeys(distance, {"y"}) || !distance.contains("y") || !distance.at("y").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& y = distance.at("y");
    if (!hasOnlyKeys(y, {"min"}) || !y.contains("min") || !y.at("min").is_number()) {
        return InvalidTriggerCondition{};
    }

    return LevitationTriggerCondition{y.at("min").get<float>()};
}

bool matchesLevitationCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<LevitationTriggerCondition>(&condition);
    auto const* payload  = payloadAs<LevitationTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return std::abs(payload->verticalDistance) >= compiled->verticalDistanceMin;
}

} // namespace advancements::criteria
