#include "mod/trigger/criteria/ItemCriteria.h"

#include "mod/predicate/ItemPredicate.h"
#include "mod/predicate/PlayerPredicate.h"
#include "mod/trigger/criteria/Common.h"

#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include <algorithm>

namespace advancements::criteria {
namespace {

constexpr auto CakeId = "minecraft:cake";
constexpr auto NoteBlockId = "minecraft:note_block";

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
    if (!hasOnlyKeys(conditions, {"item", "items", "block", "blocks"})) {
        return InvalidTriggerCondition{};
    }

    std::vector<std::string> itemIds;
    if (conditions.contains("item")) {
        if (conditions.contains("items") || !conditions.at("item").is_string()) {
            return InvalidTriggerCondition{};
        }
        itemIds.push_back(conditions.at("item").get<std::string>());
    } else if (conditions.contains("items")) {
        if (!conditions.at("items").is_array()) {
            return InvalidTriggerCondition{};
        }
        for (auto const& item : conditions.at("items")) {
            if (!item.is_string()) {
                return InvalidTriggerCondition{};
            }
            itemIds.push_back(item.get<std::string>());
        }
    } else {
        return InvalidTriggerCondition{};
    }

    std::vector<std::string> blockIds;
    if (conditions.contains("block")) {
        if (conditions.contains("blocks") || !conditions.at("block").is_string()) {
            return InvalidTriggerCondition{};
        }
        blockIds.push_back(conditions.at("block").get<std::string>());
    } else if (conditions.contains("blocks")) {
        if (!conditions.at("blocks").is_array()) {
            return InvalidTriggerCondition{};
        }
        for (auto const& block : conditions.at("blocks")) {
            if (!block.is_string()) {
                return InvalidTriggerCondition{};
            }
            blockIds.push_back(block.get<std::string>());
        }
    } else {
        return InvalidTriggerCondition{};
    }

    if (itemIds.empty() || blockIds.empty()) {
        return InvalidTriggerCondition{};
    }
    return ItemUsedOnBlockCondition{std::move(itemIds), std::move(blockIds)};
}

TriggerCondition compileAllayDropItemOnBlockCondition(nlohmann::json const& conditions) {
    if (conditions.empty()) {
        return ItemUsedOnBlockCondition{{CakeId}, {NoteBlockId}};
    }
    if (!hasOnlyKeys(conditions, {"item", "block"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("item") || !conditions.at("item").is_string() || conditions.at("item").get<std::string>() != CakeId) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()
        || conditions.at("block").get<std::string>() != NoteBlockId) {
        return InvalidTriggerCondition{};
    }

    return ItemUsedOnBlockCondition{{CakeId}, {NoteBlockId}};
}

TriggerCondition compilePiglinAdmireItemCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"item", "items", "was_targeting_bartering_player"})) {
        return InvalidTriggerCondition{};
    }

    std::vector<std::string> itemIds;
    if (conditions.contains("item")) {
        if (conditions.contains("items") || !conditions.at("item").is_string()) {
            return InvalidTriggerCondition{};
        }
        itemIds.push_back(conditions.at("item").get<std::string>());
    } else if (conditions.contains("items")) {
        if (!conditions.at("items").is_array()) {
            return InvalidTriggerCondition{};
        }
        for (auto const& item : conditions.at("items")) {
            if (!item.is_string()) {
                return InvalidTriggerCondition{};
            }
            itemIds.push_back(item.get<std::string>());
        }
    } else {
        return InvalidTriggerCondition{};
    }

    std::optional<bool> wasTargetingBarteringPlayer;
    if (conditions.contains("was_targeting_bartering_player")) {
        if (!conditions.at("was_targeting_bartering_player").is_boolean()) {
            return InvalidTriggerCondition{};
        }
        wasTargetingBarteringPlayer = conditions.at("was_targeting_bartering_player").get<bool>();
    }

    if (itemIds.empty()) {
        return InvalidTriggerCondition{};
    }
    return PiglinAdmireItemCondition{std::move(itemIds), wasTargetingBarteringPlayer};
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
    return std::ranges::find(compiled->itemIds, payload->itemId) != compiled->itemIds.end()
        && std::ranges::find(compiled->blockIds, payload->blockId) != compiled->blockIds.end();
}

bool matchesPiglinAdmireItemCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<PiglinAdmireItemCondition>(&condition);
    auto const* payload  = payloadAs<PiglinAdmireItemPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (std::ranges::find(compiled->itemIds, payload->itemId) == compiled->itemIds.end()) {
        return false;
    }
    if (!compiled->wasTargetingBarteringPlayer) {
        return true;
    }
    return payload->wasTargetingBarteringPlayer == *compiled->wasTargetingBarteringPlayer;
}

bool matchesVillagerTradeCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<VillagerTradeCondition>(&condition);
    if (compiled == nullptr) {
        return false;
    }
    return static_cast<float>(context.player.getPosition().y) >= compiled->playerYMin;
}

} // namespace advancements::criteria
