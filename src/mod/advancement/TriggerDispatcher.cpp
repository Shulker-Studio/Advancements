#include "mod/advancement/TriggerDispatcher.h"

#include "mod/MyMod.h"
#include "mc/world/actor/player/Player.h"

namespace my_mod::advancement {
namespace {

void ignoreResult(ProgressMutationResult const&) {}

bool hasOnlyKeys(nlohmann::json const& conditions, std::initializer_list<char const*> keys) {
    for (auto const& [key, _] : conditions.items()) {
        if (std::ranges::find(keys, key) == keys.end()) {
            return false;
        }
    }
    return true;
}

bool matchesBlockCondition(nlohmann::json const& conditions, TriggerContext const& context) {
    if (!hasOnlyKeys(conditions, {"block"})) {
        return false;
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()) {
        return false;
    }
    return context.blockId == conditions.at("block").get<std::string>();
}

bool matchesItemCondition(nlohmann::json const& conditions, TriggerContext const& context, bool allowCount) {
    if (!hasOnlyKeys(conditions, allowCount ? std::initializer_list<char const*>{"item", "count"}
                                             : std::initializer_list<char const*>{"item"})) {
        return false;
    }
    if (!conditions.contains("item") || !conditions.at("item").is_string()) {
        return false;
    }
    if (context.itemId != conditions.at("item").get<std::string>()) {
        return false;
    }
    if (!allowCount || !conditions.contains("count")) {
        return true;
    }
    if (!conditions.at("count").is_number_integer() || !context.itemCount) {
        return false;
    }
    return *context.itemCount >= conditions.at("count").get<int>();
}

bool matchesEntityCondition(nlohmann::json const& conditions, TriggerContext const& context) {
    if (!hasOnlyKeys(conditions, {"entity"})) {
        return false;
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_string()) {
        return false;
    }
    return context.entityTypeId == conditions.at("entity").get<std::string>();
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

        auto& logger = my_mod::MyMod::getInstance().getSelf().getLogger();
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
    return false;
}

} // namespace my_mod::advancement
