#include "mod/trigger/criteria/ItemCriteria.h"

#include "mod/predicate/ItemPredicate.h"
#include "mod/predicate/PlayerPredicate.h"
#include "mod/trigger/criteria/Common.h"

#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"

namespace advancements::criteria {
namespace {

TriggerCondition compileRequiredInventoryItemsCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"required_items"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("required_items") || !conditions.at("required_items").is_array()) {
        return InvalidTriggerCondition{};
    }

    std::vector<std::string> requiredItemIds;
    for (auto const& entry : conditions.at("required_items")) {
        if (!entry.is_string()) {
            return InvalidTriggerCondition{};
        }
        requiredItemIds.push_back(entry.get<std::string>());
    }

    if (requiredItemIds.empty()) {
        return InvalidTriggerCondition{};
    }
    return InventoryItemsCondition{std::move(requiredItemIds)};
}

TriggerCondition compileItemCondition(nlohmann::json const& conditions, bool allowCount) {
    auto const itemPredicate = predicate::parseItemPredicate(conditions, allowCount);
    if (!itemPredicate) {
        return InvalidTriggerCondition{};
    }
    return ItemTriggerCondition{itemPredicate->itemId, itemPredicate->count};
}

std::optional<float> parseVillagerTradePlayerYMin(nlohmann::json const& conditions) {
    auto const predicate = predicate::parseSinglePlayerLocationPredicate(conditions);
    if (!predicate || !predicate->location.position || !predicate->location.position->yMin) {
        return std::nullopt;
    }

    return *predicate->location.position->yMin;
}

} // namespace

TriggerCondition compileInventoryItemCondition(nlohmann::json const& conditions) {
    auto requiredItems = compileRequiredInventoryItemsCondition(conditions);
    if (!std::holds_alternative<InvalidTriggerCondition>(requiredItems)) {
        return requiredItems;
    }
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

    auto const itemPredicate = predicate::parseItemObjectPredicate(conditions.at("item"));
    if (!itemPredicate) {
        return InvalidTriggerCondition{};
    }

    auto const& itemId = itemPredicate->itemId;
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
    if (auto const* compiledItems = std::get_if<InventoryItemsCondition>(&condition)) {
        auto const* payload = payloadAs<ItemTriggerPayload>(context);
        if (payload == nullptr) {
            return false;
        }

        auto const& inventory = context.player.getInventory();
        for (auto const& requiredItemId : compiledItems->requiredItemIds) {
            bool found = false;
            for (int slot = 0; slot < inventory.getContainerSize(); ++slot) {
                auto const& item = inventory.getItem(slot);
                if (!item.isNull() && item.getTypeName() == requiredItemId && item.mCount > 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    auto const* compiled = std::get_if<ItemTriggerCondition>(&condition);
    auto const* payload  = payloadAs<ItemTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return predicate::matchesItemPredicate(predicate::ItemPredicate{compiled->itemId, compiled->count}, payload->itemId, payload->itemCount);
}

bool matchesSimpleItemCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<ItemTriggerCondition>(&condition);
    auto const* payload  = payloadAs<ItemTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return predicate::matchesItemPredicate(predicate::ItemPredicate{compiled->itemId, compiled->count}, payload->itemId, payload->itemCount);
}

bool matchesItemUsedOnBlockCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<ItemUsedOnBlockCondition>(&condition);
    auto const* payload  = payloadAs<ItemUsedOnBlockPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return predicate::matchesItemPredicate(predicate::ItemPredicate{compiled->itemId, std::nullopt}, payload->itemId, std::nullopt)
        && payload->blockId == compiled->blockId;
}

bool matchesVillagerTradeCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<VillagerTradeCondition>(&condition);
    if (compiled == nullptr) {
        return false;
    }
    return static_cast<float>(context.player.getPosition().y) >= compiled->playerYMin;
}

} // namespace advancements::criteria
