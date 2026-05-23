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

bool hasOnlyKeys(nlohmann::json const& conditions, std::initializer_list<char const*> keys) {
    for (auto const& [key, _] : conditions.items()) {
        if (std::ranges::find(keys, key) == keys.end()) {
            return false;
        }
    }
    return true;
}

bool matchesBlockCondition(nlohmann::json const& conditions, TriggerContext const& context) {
    auto const* payload = payloadAs<BlockTriggerPayload>(context);
    if (payload == nullptr) {
        return false;
    }
    if (!hasOnlyKeys(conditions, {"block"})) {
        return false;
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()) {
        return false;
    }
    return payload->blockId == conditions.at("block").get<std::string>();
}

bool matchesItemCondition(nlohmann::json const& conditions, TriggerContext const& context, bool allowCount) {
    auto const* payload = payloadAs<ItemTriggerPayload>(context);
    if (payload == nullptr) {
        return false;
    }
    if (!hasOnlyKeys(conditions, allowCount ? std::initializer_list<char const*>{"item", "count"}
                                             : std::initializer_list<char const*>{"item"})) {
        return false;
    }
    if (!conditions.contains("item") || !conditions.at("item").is_string()) {
        return false;
    }
    if (payload->itemId != conditions.at("item").get<std::string>()) {
        return false;
    }
    if (!allowCount || !conditions.contains("count")) {
        return true;
    }
    if (!conditions.at("count").is_number_integer() || !payload->itemCount) {
        return false;
    }
    return *payload->itemCount >= conditions.at("count").get<int>();
}

bool matchesEntityCondition(nlohmann::json const& conditions, TriggerContext const& context) {
    auto const* payload = payloadAs<EntityTriggerPayload>(context);
    if (payload == nullptr) {
        return false;
    }
    if (!hasOnlyKeys(conditions, {"entity"})) {
        return false;
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_string()) {
        return false;
    }
    return payload->entityTypeId == conditions.at("entity").get<std::string>();
}

bool matchesChangedDimensionCondition(nlohmann::json const& conditions, TriggerContext const& context) {
    auto const* payload = payloadAs<DimensionTriggerPayload>(context);
    if (payload == nullptr) {
        return false;
    }
    if (!hasOnlyKeys(conditions, {"from", "to"})) {
        return false;
    }
    if (conditions.contains("from")) {
        if (!conditions.at("from").is_string() || payload->fromDimension != conditions.at("from").get<std::string>()) {
            return false;
        }
    }
    if (conditions.contains("to")) {
        if (!conditions.at("to").is_string() || payload->toDimension != conditions.at("to").get<std::string>()) {
            return false;
        }
    }
    return true;
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
    if (!binding.conditions) {
        return true;
    }

    auto const& conditions = *binding.conditions;
    if (!conditions.is_object() || conditions.empty()) {
        return true;
    }

    if (binding.triggerId == "bedrock:player_destroy_block") {
        return matchesBlockCondition(conditions, context);
    }
    if (binding.triggerId == "minecraft:inventory_changed") {
        return matchesItemCondition(conditions, context, true);
    }
    if (binding.triggerId == "minecraft:consume_item") {
        return matchesItemCondition(conditions, context, false);
    }
    if (binding.triggerId == "minecraft:player_killed_entity") {
        return matchesEntityCondition(conditions, context);
    }
    if (binding.triggerId == "minecraft:entity_killed_player") {
        return matchesEntityCondition(conditions, context);
    }
    if (binding.triggerId == "minecraft:changed_dimension") {
        return matchesChangedDimensionCondition(conditions, context);
    }
    return false;
}

} // namespace my_mod::advancement
