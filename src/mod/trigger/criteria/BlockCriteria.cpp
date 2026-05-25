#include "mod/trigger/criteria/BlockCriteria.h"

#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileBlockCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"block"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()) {
        return InvalidTriggerCondition{};
    }
    return BlockTriggerCondition{conditions.at("block").get<std::string>()};
}

bool matchesBlockCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<BlockTriggerCondition>(&condition);
    auto const* payload  = payloadAs<BlockTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->blockId == compiled->blockId;
}

} // namespace advancements::criteria
