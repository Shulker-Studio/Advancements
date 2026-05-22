#include "mod/advancement/TriggerDispatcher.h"

#include "mc/world/actor/player/Player.h"

namespace my_mod::advancement {
namespace {

void ignoreResult(ProgressMutationResult const&) {}

} // namespace

TriggerDispatcher::TriggerDispatcher(TriggerIndex const& index, ProgressService const& progressService)
: mIndex(index),
  mProgressService(progressService) {}

void TriggerDispatcher::dispatch(
    std::filesystem::path const& worldDataDir,
    LoadResult const&            definitions,
    TriggerContext const&        context
) const {
    auto const bindings = mIndex.find(context.triggerId);
    for (auto const& binding : bindings) {
        if (!matches(binding, context)) {
            continue;
        }

        auto const advancement = definitions.advancements.find(binding.advancementId);
        if (advancement == definitions.advancements.end()) {
            continue;
        }

        ignoreResult(
            mProgressService.grantCriterion(worldDataDir, context.player.getUuid(), advancement->second, binding.criterionName)
        );
    }
}

bool TriggerDispatcher::matches(CriterionBinding const& binding, TriggerContext const& context) const {
    if (!binding.conditions) {
        return true;
    }

    auto const& conditions = *binding.conditions;
    if (!conditions.is_object() || conditions.empty()) {
        return true;
    }

    if (conditions.size() != 1 || !conditions.contains("block") || !conditions.at("block").is_string()) {
        return false;
    }

    return context.blockId == conditions.at("block").get<std::string>();
}

} // namespace my_mod::advancement
