#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace advancements::predicate {

struct ItemPredicate {
    std::string        itemId;
    std::optional<int> count;
};

std::optional<ItemPredicate> parseItemPredicate(nlohmann::json const& conditions, bool allowCount);
std::optional<ItemPredicate> parseItemObjectPredicate(nlohmann::json const& predicate);
bool matchesItemPredicate(
    ItemPredicate const&     predicate,
    std::string const&       itemId,
    std::optional<int> const& itemCount
);

} // namespace advancements::predicate
