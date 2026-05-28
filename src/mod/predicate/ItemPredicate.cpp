#include "mod/predicate/ItemPredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

std::optional<ItemPredicate> parseItemPredicate(nlohmann::json const& conditions, bool allowCount) {
    if (!conditions.is_object()) {
        return std::nullopt;
    }
    if (!hasOnlyKeys(conditions, allowCount ? std::initializer_list<char const*>{"item", "count"}
                                            : std::initializer_list<char const*>{"item"})) {
        return std::nullopt;
    }
    if (!conditions.contains("item") || !conditions.at("item").is_string()) {
        return std::nullopt;
    }

    std::optional<int> count;
    if (conditions.contains("count")) {
        if (!allowCount || !conditions.at("count").is_number_integer()) {
            return std::nullopt;
        }
        count = conditions.at("count").get<int>();
    }

    return ItemPredicate{conditions.at("item").get<std::string>(), count};
}

std::optional<ItemPredicate> parseItemObjectPredicate(nlohmann::json const& predicate) {
    if (!predicate.is_object() || !hasOnlyKeys(predicate, {"items"})) {
        return std::nullopt;
    }
    if (!predicate.contains("items") || !predicate.at("items").is_string()) {
        return std::nullopt;
    }

    return ItemPredicate{predicate.at("items").get<std::string>(), std::nullopt};
}

bool matchesItemPredicate(
    ItemPredicate const&     predicate,
    std::string const&       itemId,
    std::optional<int> const& itemCount
) {
    if (itemId != predicate.itemId) {
        return false;
    }
    if (!predicate.count) {
        return true;
    }
    if (!itemCount) {
        return false;
    }
    return *itemCount >= *predicate.count;
}

} // namespace advancements::predicate
