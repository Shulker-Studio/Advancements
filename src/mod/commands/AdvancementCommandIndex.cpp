#include "mod/commands/AdvancementCommandIndex.h"

#include <algorithm>

namespace advancements::commands {

AdvancementCommandIndex buildAdvancementCommandIndex(LoadResult const& result) {
    AdvancementCommandIndex index;

    index.advancementIds.reserve(result.advancements.size());
    for (auto const& [id, _] : result.advancements) {
        index.advancementIds.emplace_back(id);
    }
    std::ranges::sort(index.advancementIds);

    for (auto const& advancementEntry : result.advancements) {
        for (auto const& criterionEntry : advancementEntry.second.criteria) {
            index.criterionNames.emplace_back(criterionEntry.first);
        }
    }
    std::ranges::sort(index.criterionNames);
    auto const last = std::ranges::unique(index.criterionNames).begin();
    index.criterionNames.erase(last, index.criterionNames.end());

    return index;
}

} // namespace advancements::commands
