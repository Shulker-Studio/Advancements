#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace advancements::predicate {

struct BlockPredicate {
    std::string blockId;
};

std::optional<BlockPredicate> parseBlockPredicate(nlohmann::json const& conditions);
bool                          matchesBlockPredicate(BlockPredicate const& predicate, std::string const& blockId);

} // namespace advancements::predicate
