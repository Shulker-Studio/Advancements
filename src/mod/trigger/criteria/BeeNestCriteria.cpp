#include "mod/trigger/criteria/BeeNestCriteria.h"

#include "mod/predicate/ItemPredicate.h"
#include "mod/trigger/criteria/Common.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/enchanting/Enchant.h"
#include "mc/world/item/enchanting/EnchantUtils.h"

#include <algorithm>

namespace advancements::criteria {
namespace {

bool isSupportedBeeNestBlock(std::string const& blockId) {
    return blockId == "minecraft:bee_nest" || blockId == "minecraft:beehive";
}

std::optional<std::string> parseBlockId(nlohmann::json const& conditions) {
    if (!conditions.contains("block")) {
        return std::nullopt;
    }
    if (!conditions.at("block").is_string()) {
        return std::string{};
    }

    auto blockId = conditions.at("block").get<std::string>();
    if (!isSupportedBeeNestBlock(blockId)) {
        return std::string{};
    }
    return blockId;
}

std::optional<int> parseNumBeesInsideMin(nlohmann::json const& conditions) {
    if (!conditions.contains("num_bees_inside")) {
        return std::nullopt;
    }
    if (!conditions.at("num_bees_inside").is_object()) {
        return -1;
    }

    auto const& numBeesInside = conditions.at("num_bees_inside");
    if (!hasOnlyKeys(numBeesInside, {"min"}) || !numBeesInside.contains("min") || !numBeesInside.at("min").is_number_integer()) {
        return -1;
    }
    return numBeesInside.at("min").get<int>();
}

std::optional<std::string> parseItemId(nlohmann::json const& conditions) {
    if (!conditions.contains("item")) {
        return std::nullopt;
    }
    if (!conditions.at("item").is_object()) {
        return std::string{};
    }

    auto const& item = conditions.at("item");
    if (!hasOnlyKeys(item, {"items", "enchantments"})) {
        return std::string{};
    }
    if (!item.contains("items")) {
        return std::nullopt;
    }
    if (!item.at("items").is_string()) {
        return std::string{};
    }
    return item.at("items").get<std::string>();
}

bool parseRequiresSilkTouch(nlohmann::json const& conditions) {
    if (!conditions.contains("item") || !conditions.at("item").is_object()) {
        return false;
    }

    auto const& item = conditions.at("item");
    if (!item.contains("enchantments") || !item.at("enchantments").is_array()) {
        return false;
    }

    return std::ranges::any_of(item.at("enchantments"), [](auto const& enchantment) {
        return enchantment.is_object() && enchantment.contains("enchantment") && enchantment.at("enchantment").is_string()
            && enchantment.at("enchantment").template get<std::string>() == "minecraft:silk_touch";
    });
}

std::string selectedItemId(Player const& player) {
    auto const& selectedItem = player.getSelectedItem();
    return selectedItem.isNull() ? std::string{} : selectedItem.getTypeName();
}

bool selectedItemHasSilkTouch(Player const& player) {
    return EnchantUtils::hasEnchant(Enchant::Type::SilkTouch, player.getSelectedItem());
}

} // namespace

TriggerCondition compileBeeNestDestroyedCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"block", "item", "num_bees_inside"})) {
        return InvalidTriggerCondition{};
    }

    auto const blockId = parseBlockId(conditions);
    if (blockId && blockId->empty()) {
        return InvalidTriggerCondition{};
    }

    auto const itemId = parseItemId(conditions);
    if (itemId && itemId->empty()) {
        return InvalidTriggerCondition{};
    }

    auto const numBeesInsideMin = parseNumBeesInsideMin(conditions);
    if (numBeesInsideMin && *numBeesInsideMin < 0) {
        return InvalidTriggerCondition{};
    }

    return BeeNestDestroyedCondition{blockId, itemId, parseRequiresSilkTouch(conditions), numBeesInsideMin};
}

bool matchesBeeNestDestroyedCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* payload = payloadAs<BeeNestDestroyedPayload>(context);
    if (payload == nullptr) {
        return false;
    }
    if (std::holds_alternative<NoTriggerCondition>(condition)) {
        return true;
    }

    auto const* compiled = std::get_if<BeeNestDestroyedCondition>(&condition);
    if (compiled == nullptr) {
        return false;
    }
    if (compiled->blockId && payload->blockId != *compiled->blockId) {
        return false;
    }
    if (compiled->itemId && !predicate::matchesItemPredicate(predicate::ItemPredicate{*compiled->itemId, std::nullopt}, selectedItemId(context.player), std::nullopt)) {
        return false;
    }
    if (compiled->requireSilkTouch && !selectedItemHasSilkTouch(context.player)) {
        return false;
    }
    if (compiled->numBeesInsideMin && payload->numBeesInside < *compiled->numBeesInsideMin) {
        return false;
    }
    return true;
}

} // namespace advancements::criteria
