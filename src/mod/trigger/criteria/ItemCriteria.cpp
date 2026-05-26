#include "mod/trigger/criteria/ItemCriteria.h"

#include "mod/trigger/criteria/Common.h"

#include "mc/world/actor/player/Player.h"

namespace advancements::criteria {
namespace {

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

std::optional<float> parseVillagerTradePlayerYMin(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"player"})) {
        return std::nullopt;
    }
    if (!conditions.contains("player") || !conditions.at("player").is_array() || conditions.at("player").size() != 1) {
        return std::nullopt;
    }

    auto const& playerCondition = conditions.at("player").front();
    if (!playerCondition.is_object() || !hasOnlyKeys(playerCondition, {"condition", "entity", "predicate"})) {
        return std::nullopt;
    }
    if (!playerCondition.contains("condition") || !playerCondition.at("condition").is_string()
        || playerCondition.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return std::nullopt;
    }
    if (!playerCondition.contains("entity") || !playerCondition.at("entity").is_string()
        || playerCondition.at("entity").get<std::string>() != "this") {
        return std::nullopt;
    }
    if (!playerCondition.contains("predicate") || !playerCondition.at("predicate").is_object()) {
        return std::nullopt;
    }

    auto const& predicate = playerCondition.at("predicate");
    if (!hasOnlyKeys(predicate, {"location"}) || !predicate.contains("location")
        || !predicate.at("location").is_object()) {
        return std::nullopt;
    }

    auto const& location = predicate.at("location");
    if (!hasOnlyKeys(location, {"position"}) || !location.contains("position") || !location.at("position").is_object()) {
        return std::nullopt;
    }

    auto const& position = location.at("position");
    if (!hasOnlyKeys(position, {"y"}) || !position.contains("y") || !position.at("y").is_object()) {
        return std::nullopt;
    }

    auto const& y = position.at("y");
    if (!hasOnlyKeys(y, {"min"}) || !y.contains("min") || !y.at("min").is_number()) {
        return std::nullopt;
    }

    return y.at("min").get<float>();
}

} // namespace

TriggerCondition compileInventoryItemCondition(nlohmann::json const& conditions) {
    return compileItemCondition(conditions, true);
}

TriggerCondition compileSimpleItemCondition(nlohmann::json const& conditions) {
    return compileItemCondition(conditions, false);
}

TriggerCondition compileItemUsedOnBlockCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"item", "block"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("item") || !conditions.at("item").is_string()) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()) {
        return InvalidTriggerCondition{};
    }
    return ItemUsedOnBlockCondition{
        conditions.at("item").get<std::string>(),
        conditions.at("block").get<std::string>(),
    };
}

TriggerCondition compileShotCrossbowCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"item"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("item") || !conditions.at("item").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& item = conditions.at("item");
    if (!hasOnlyKeys(item, {"items"})) {
        return InvalidTriggerCondition{};
    }
    if (!item.contains("items") || !item.at("items").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto const itemId = item.at("items").get<std::string>();
    if (itemId != "minecraft:crossbow") {
        return InvalidTriggerCondition{};
    }
    return ItemTriggerCondition{itemId, std::nullopt};
}

TriggerCondition compileVillagerTradeCondition(nlohmann::json const& conditions) {
    auto const playerYMin = parseVillagerTradePlayerYMin(conditions);
    if (!playerYMin) {
        return InvalidTriggerCondition{};
    }
    return VillagerTradeCondition{*playerYMin};
}

bool matchesInventoryItemCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<ItemTriggerCondition>(&condition);
    auto const* payload  = payloadAs<ItemTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (payload->itemId != compiled->itemId) {
        return false;
    }
    if (!compiled->count) {
        return true;
    }
    if (!payload->itemCount) {
        return false;
    }
    return *payload->itemCount >= *compiled->count;
}

bool matchesSimpleItemCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<ItemTriggerCondition>(&condition);
    auto const* payload  = payloadAs<ItemTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->itemId == compiled->itemId;
}

bool matchesItemUsedOnBlockCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<ItemUsedOnBlockCondition>(&condition);
    auto const* payload  = payloadAs<ItemUsedOnBlockPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->itemId == compiled->itemId && payload->blockId == compiled->blockId;
}

bool matchesVillagerTradeCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<VillagerTradeCondition>(&condition);
    if (compiled == nullptr) {
        return false;
    }
    return static_cast<float>(context.player.getPosition().y) >= compiled->playerYMin;
}

} // namespace advancements::criteria
