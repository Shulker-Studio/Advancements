#include "mod/trigger/criteria/BlockCriteria.h"

#include "mod/predicate/BlockPredicate.h"
#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compileBlockCondition(nlohmann::json const& conditions) {
    auto const blockPredicate = predicate::parseBlockPredicate(conditions);
    if (!blockPredicate) {
        return InvalidTriggerCondition{};
    }
    return BlockTriggerCondition{blockPredicate->blockId};
}

bool matchesBlockCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<BlockTriggerCondition>(&condition);
    auto const* payload  = payloadAs<BlockTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return predicate::matchesBlockPredicate(predicate::BlockPredicate{compiled->blockId}, payload->blockId);
}

TriggerCondition compileEnterBlockCondition(nlohmann::json const& conditions) {
    auto const blockPredicate = predicate::parseBlockPredicate(conditions);
    if (!blockPredicate) {
        return InvalidTriggerCondition{};
    }
    return EnterBlockCondition{blockPredicate->blockId};
}

bool matchesEnterBlockCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<EnterBlockCondition>(&condition);
    auto const* payload  = payloadAs<EnterBlockPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return predicate::matchesBlockPredicate(predicate::BlockPredicate{compiled->blockId}, payload->blockId);
}

} // namespace advancements::criteria
