#include "mod/trigger/criteria/ItemCriteria.h"

#include "mod/trigger/criteria/Common.h"

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

} // namespace advancements::criteria
