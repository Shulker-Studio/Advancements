#include "mod/trigger/criteria/BeaconCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileConstructBeaconCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"level"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("level") || !conditions.at("level").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& level = conditions.at("level");
    if (!hasOnlyKeys(level, {"min"}) || !level.contains("min") || !level.at("min").is_number_integer()) {
        return InvalidTriggerCondition{};
    }

    return ConstructBeaconTriggerCondition{level.at("min").get<int>()};
}

bool matchesConstructBeaconCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* payload = payloadAs<ConstructBeaconTriggerPayload>(context);
    if (payload == nullptr) {
        return false;
    }

    if (std::holds_alternative<NoTriggerCondition>(condition)) {
        return true;
    }

    auto const* compiled = std::get_if<ConstructBeaconTriggerCondition>(&condition);
    return compiled != nullptr && payload->level >= compiled->levelMin;
}

} // namespace advancements::criteria
