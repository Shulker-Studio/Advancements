#include "mod/advancement/TriggerIndex.h"

#include <algorithm>

namespace my_mod::advancement {
namespace {

bool hasOnlyKeys(nlohmann::json const& conditions, std::initializer_list<char const*> keys) {
    for (auto const& [key, _] : conditions.items()) {
        if (std::ranges::find(keys, key) == keys.end()) {
            return false;
        }
    }
    return true;
}

TriggerCondition compileBlockCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"block"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()) {
        return InvalidTriggerCondition{};
    }
    return BlockTriggerCondition{conditions.at("block").get<std::string>()};
}

TriggerCondition compileItemCondition(nlohmann::json const& conditions, bool allowCount) {
    if (!hasOnlyKeys(conditions, allowCount ? std::initializer_list<char const*>{"item", "count"}
                                             : std::initializer_list<char const*>{"item"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("item") || !conditions.at("item").is_string()) {
        return InvalidTriggerCondition{};
    }
    std::optional<int> count;
    if (conditions.contains("count")) {
        if (!allowCount || !conditions.at("count").is_number_integer()) {
            return InvalidTriggerCondition{};
        }
        count = conditions.at("count").get<int>();
    }
    return ItemTriggerCondition{conditions.at("item").get<std::string>(), count};
}

TriggerCondition compileEntityCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"entity"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_string()) {
        return InvalidTriggerCondition{};
    }
    return EntityTriggerCondition{conditions.at("entity").get<std::string>()};
}

TriggerCondition compileChangedDimensionCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"from", "to"})) {
        return InvalidTriggerCondition{};
    }
    std::optional<std::string> fromDimension;
    std::optional<std::string> toDimension;
    if (conditions.contains("from")) {
        if (!conditions.at("from").is_string()) {
            return InvalidTriggerCondition{};
        }
        fromDimension = conditions.at("from").get<std::string>();
    }
    if (conditions.contains("to")) {
        if (!conditions.at("to").is_string()) {
            return InvalidTriggerCondition{};
        }
        toDimension = conditions.at("to").get<std::string>();
    }
    return DimensionTriggerCondition{fromDimension, toDimension};
}

TriggerCondition compileLocationStructureCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"player"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("player") || !conditions.at("player").is_array() || conditions.at("player").size() != 1) {
        return InvalidTriggerCondition{};
    }

    auto const& playerCondition = conditions.at("player").front();
    if (!playerCondition.is_object() || !hasOnlyKeys(playerCondition, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("condition") || !playerCondition.at("condition").is_string()
        || playerCondition.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("entity") || !playerCondition.at("entity").is_string()
        || playerCondition.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("predicate") || !playerCondition.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = playerCondition.at("predicate");
    if (!hasOnlyKeys(predicate, {"location"}) || !predicate.contains("location") || !predicate.at("location").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& location = predicate.at("location");
    if (!hasOnlyKeys(location, {"structures"}) || !location.contains("structures") || !location.at("structures").is_string()) {
        return InvalidTriggerCondition{};
    }

    return LocationStructureCondition{location.at("structures").get<std::string>()};
}

TriggerCondition compileTriggerCondition(std::string_view triggerId, std::optional<nlohmann::json> const& rawConditions) {
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

    if (triggerId == "bedrock:player_destroy_block") {
        return compileBlockCondition(conditions);
    }
    if (triggerId == "minecraft:inventory_changed") {
        return compileItemCondition(conditions, true);
    }
    if (triggerId == "minecraft:consume_item" || triggerId == "minecraft:used_totem"
        || triggerId == "minecraft:fishing_rod_hooked" || triggerId == "minecraft:filled_bucket") {
        return compileItemCondition(conditions, false);
    }
    if (triggerId == "minecraft:player_killed_entity") {
        return compileEntityCondition(conditions);
    }
    if (triggerId == "minecraft:entity_killed_player") {
        return compileEntityCondition(conditions);
    }
    if (triggerId == "minecraft:changed_dimension") {
        return compileChangedDimensionCondition(conditions);
    }
    if (triggerId == "minecraft:location") {
        return compileLocationStructureCondition(conditions);
    }
    if (triggerId == "minecraft:villager_trade" || triggerId == "minecraft:enchanted_item") {
        return InvalidTriggerCondition{};
    }
    return InvalidTriggerCondition{};
}

} // namespace

void TriggerIndex::rebuild(LoadResult const& result) {
    mBindings.clear();
    mBindingCount = 0;

    for (auto const& [advancementId, advancement] : result.advancements) {
        for (auto const& [criterionName, criterion] : advancement.criteria) {
            mBindings[criterion.trigger].push_back(CriterionBinding{
                advancementId,
                criterionName,
                criterion.trigger,
                compileTriggerCondition(criterion.trigger, criterion.conditions)
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
