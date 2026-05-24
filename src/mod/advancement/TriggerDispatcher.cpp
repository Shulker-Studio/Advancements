#include "mod/advancement/TriggerDispatcher.h"

#include "mod/advancement/AdvancementNotifier.h"
#include "mod/MyMod.h"
#include "mc/world/actor/player/Player.h"

namespace my_mod::advancement {
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
    auto const* payload  = payloadAs<EntityTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->entityTypeId == compiled->entityTypeId;
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

bool matchesLocationStructureCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<LocationStructureCondition>(&condition);
    auto const* payload  = payloadAs<LocationStructurePayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->structureId == compiled->structureId;
}

bool matchesLootTableCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<LootTableCondition>(&condition);
    auto const* payload  = payloadAs<LootTablePayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->lootTableId == compiled->lootTableId;
}

bool matchesPlayerHurtEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<PlayerHurtEntityCondition>(&condition);
    auto const* payload  = payloadAs<PlayerHurtEntityPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->requireArrowDirectEntity && !payload->directEntityIsArrow) {
        return false;
    }
    if (compiled->requireProjectileDamageTag && !payload->isProjectileDamage) {
        return false;
    }
    return true;
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

        auto const result =
            mProgressService.grantCriterion(worldDataDir, context.player.getUuid(), advancement->second, binding.criterionName);

        auto& mod    = my_mod::MyMod::getInstance();
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
            notifyAdvancementCompleted(mod, context.player, advancement->second);
        }

        ignoreResult(result);
    }
}

bool TriggerDispatcher::matches(CriterionBinding const& binding, TriggerContext const& context) const {
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
    if (binding.triggerId == "minecraft:shot_crossbow") {
        return matchesItemCondition(binding.condition, context, false);
    }
    if (binding.triggerId == "minecraft:player_killed_entity") {
        return matchesEntityCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:entity_killed_player") {
        return matchesEntityCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:changed_dimension") {
        return matchesChangedDimensionCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:location") {
        return matchesLocationStructureCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:player_generates_container_loot") {
        return matchesLootTableCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:player_hurt_entity") {
        return matchesPlayerHurtEntityCondition(binding.condition, context);
    }
    if (binding.triggerId == "minecraft:target_hit") {
        return matchesTargetHitCondition(binding.condition, context);
    }
    return false;
}

} // namespace my_mod::advancement
