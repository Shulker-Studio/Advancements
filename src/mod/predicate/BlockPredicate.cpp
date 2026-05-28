#include "mod/predicate/BlockPredicate.h"

#include "mod/predicate/Common.h"

namespace advancements::predicate {

std::optional<BlockPredicate> parseBlockPredicate(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"block"})) {
        return std::nullopt;
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()) {
        return std::nullopt;
    }
    return BlockPredicate{conditions.at("block").get<std::string>()};
}

bool matchesBlockPredicate(BlockPredicate const& predicate, std::string const& blockId) {
    return blockId == predicate.blockId;
}

} // namespace advancements::predicate
