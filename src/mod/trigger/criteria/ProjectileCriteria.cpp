#include "mod/trigger/criteria/ProjectileCriteria.h"

#include "mod/predicate/DistancePredicate.h"
#include "mod/predicate/EntityPredicate.h"
#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileTargetHitCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"signal_strength", "projectile"})) {
        return InvalidTriggerCondition{};
    }

    if (!conditions.contains("signal_strength") || !conditions.at("signal_strength").is_number_integer()) {
        return InvalidTriggerCondition{};
    }
    auto const signalStrength = conditions.at("signal_strength").get<int>();
    if (signalStrength != 15) {
        return InvalidTriggerCondition{};
    }

    auto const projectilePredicate = predicate::parseSingleThisEntityDistancePredicate(conditions, "projectile");
    if (!projectilePredicate || !projectilePredicate->horizontalDistance) {
        return InvalidTriggerCondition{};
    }
    auto const horizontalMin = projectilePredicate->horizontalDistance->min;
    if (horizontalMin != 30.0F) {
        return InvalidTriggerCondition{};
    }

    return TargetHitCondition{signalStrength, horizontalMin};
}

bool matchesTargetHitCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<TargetHitCondition>(&condition);
    auto const* payload  = payloadAs<TargetHitPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (payload->signalStrength != compiled->requiredSignalStrength) {
        return false;
    }
    return payload->projectileHorizontalDistance >= compiled->projectileHorizontalDistanceMin;
}

} // namespace advancements::criteria
