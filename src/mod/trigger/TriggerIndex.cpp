#include "mod/trigger/TriggerIndex.h"

#include "mod/trigger/criteria/TriggerCriteriaRegistry.h"

namespace advancements {
namespace {

TriggerCondition compileDescriptorCondition(
    TriggerDescriptor const&             descriptor,
    std::optional<nlohmann::json> const& rawConditions
) {
    if (!rawConditions) {
        return NoTriggerCondition{};
    }

    auto const& conditions = *rawConditions;
    if (!conditions.is_object()) {
        return InvalidTriggerCondition{};
    }
    if (conditions.empty()) {
        return NoTriggerCondition{};
    }
    return descriptor.compile(conditions);
}

} // namespace

void TriggerIndex::rebuild(LoadResult const& result) {
    mBindings.clear();
    mBindingCount = 0;

    for (auto const& [advancementId, advancement] : result.advancements) {
        for (auto const& [criterionName, criterion] : advancement.criteria) {
            auto const* descriptor = criteria::findTriggerDescriptor(criterion.trigger);
            mBindings[criterion.trigger].push_back(CriterionBinding{
                &advancement,
                advancementId,
                criterionName,
                criterion.trigger,
                descriptor,
                descriptor == nullptr ? InvalidTriggerCondition{}
                                      : compileDescriptorCondition(*descriptor, criterion.conditions)
            });
            ++mBindingCount;
        }
    }
}

size_t TriggerIndex::triggerCount() const { return mBindings.size(); }

size_t TriggerIndex::bindingCount() const { return mBindingCount; }

std::span<CriterionBinding const> TriggerIndex::find(std::string_view triggerId) const {
    auto const found = mBindings.find(triggerId);
    if (found == mBindings.end()) {
        return {};
    }
    return found->second;
}

} // namespace advancements
