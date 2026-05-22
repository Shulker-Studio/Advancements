#include "mod/advancement/TriggerIndex.h"

namespace my_mod::advancement {

void TriggerIndex::rebuild(LoadResult const& result) {
    mBindings.clear();
    mBindingCount = 0;

    for (auto const& [advancementId, advancement] : result.advancements) {
        for (auto const& [criterionName, criterion] : advancement.criteria) {
            mBindings[criterion.trigger].push_back(CriterionBinding{
                advancementId,
                criterionName,
                criterion.trigger,
                criterion.conditions
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

} // namespace my_mod::advancement
