#include "mod/trigger/TriggerDispatcher.h"

#include "mod/advancement/AdvancementNotifier.h"
#include "mod/Entry.h"
#include "mc/world/actor/player/Player.h"

namespace advancements {
void ignoreResult(ProgressMutationResult const&) {}

bool descriptorMatches(CriterionBinding const& binding, TriggerContext const& context) {
    if (std::holds_alternative<NoTriggerCondition>(binding.condition)) {
        return true;
    }
    if (std::holds_alternative<InvalidTriggerCondition>(binding.condition)) {
        return false;
    }
    if (binding.descriptor == nullptr) {
        return false;
    }
    return binding.descriptor->match(binding.condition, context);
}

TriggerDispatcher::TriggerDispatcher(TriggerIndex const& index, ProgressService& progressService)
: mIndex(index),
  mProgressService(progressService) {}

void TriggerDispatcher::dispatch(
    std::filesystem::path const& worldDataDir,
    [[maybe_unused]] LoadResult const& definitions,
    TriggerContext const&        context
) const {
    auto const bindings = mIndex.find(context.triggerId);
    for (auto const& binding : bindings) {
        auto const matched = descriptorMatches(binding, context);
        if (!matched) {
            continue;
        }

        if (binding.advancement == nullptr) {
            continue;
        }

        auto const& advancement = *binding.advancement;

        auto const result =
            mProgressService.grantCriterion(worldDataDir, context.player.getUuid(), advancement, binding.criterionName);

        auto& mod    = advancements::Entry::getInstance();
        auto& logger = mod.getSelf().getLogger();
        if (!result.ok()) {
            logger.debug(
                "Advancements debug: grant failed advancement={} criterion={} player={} errors={}",
                binding.advancementId,
                binding.criterionName,
                context.player.getRealName(),
                result.errors.size()
            );
            continue;
        }

        if (result.changed) {
            logger.debug(
                "Advancements debug: granted advancement={} criterion={} player={} done={}",
                binding.advancementId,
                binding.criterionName,
                context.player.getRealName(),
                result.done
            );
        }

        if (result.becameDone) {
            notifyAdvancementCompleted(mod, context.player, advancement);
        }

        ignoreResult(result);
    }
}

} // namespace advancements
