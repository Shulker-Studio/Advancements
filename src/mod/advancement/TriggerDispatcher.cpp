#include "mod/advancement/TriggerDispatcher.h"

#include "mod/advancement/AdvancementNotifier.h"
#include "mod/Entry.h"
#include "mc/world/actor/player/Player.h"

namespace advancements {
namespace {

void ignoreResult(ProgressMutationResult const&) {}

template <typename T>
T const* payloadAs(TriggerContext const& context) {
    return std::get_if<T>(&context.payload);
}

bool matchesBlockCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<BlockTriggerCondition>(&condition);
    auto const* payload  = payloadAs<BlockTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->blockId == compiled->blockId;
}

bool matchesItemCondition(TriggerCondition const& condition, TriggerContext const& context, bool allowCount) {
    auto const* compiled = std::get_if<ItemTriggerCondition>(&condition);
    auto const* payload  = payloadAs<ItemTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (payload->itemId != compiled->itemId) {
        return false;
    }
    if (!allowCount || !compiled->count) {
        return true;
    }
    if (!payload->itemCount) {
        return false;
    }
    return *payload->itemCount >= *compiled->count;
}

bool matchesEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<EntityTriggerCondition>(&condition);
    if (compiled == nullptr) {
        return false;
    }

    if (auto const* payload = payloadAs<EntityTriggerPayload>(context); payload != nullptr) {
        return payload->entityTypeId == compiled->entityTypeId;
    }

    if (auto const* payload = payloadAs<PlayerKilledEntitySniperDuelPayload>(context); payload != nullptr) {
        return payload->killedEntityTypeId == compiled->entityTypeId;
    }

    return false;
}

bool matchesChangedDimensionCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<DimensionTriggerCondition>(&condition);
    auto const* payload  = payloadAs<DimensionTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->fromDimension && payload->fromDimension != *compiled->fromDimension) {
        return false;
    }
    if (compiled->toDimension && payload->toDimension != *compiled->toDimension) {
        return false;
    }
    return true;
}

} // namespace

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
        auto const matched = binding.descriptor == nullptr ? legacyMatches(binding, context)
                                                           : binding.descriptor->match(binding.condition, context);
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

bool TriggerDispatcher::legacyMatches(CriterionBinding const& binding, TriggerContext const& context) const {
    if (std::holds_alternative<NoTriggerCondition>(binding.condition)) {
        return true;
    }
    if (std::holds_alternative<InvalidTriggerCondition>(binding.condition)) {
        return false;
    }

    if (binding.triggerId == "bedrock:player_destroy_block") {
        return matchesBlockCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:inventory_changed") {
        return matchesItemCondition(binding.condition, context, true);
    }
    if (binding.triggerId == "minecraft:consume_item" || binding.triggerId == "minecraft:used_totem"
        || binding.triggerId == "minecraft:fishing_rod_hooked" || binding.triggerId == "minecraft:filled_bucket") {
        return matchesItemCondition(binding.condition, context, false);
    }
    if (binding.triggerId == "minecraft:entity_killed_player") {
        return matchesEntityCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:changed_dimension") {
        return matchesChangedDimensionCondition(binding.condition, context);
    }
    return false;
}

} // namespace advancements
