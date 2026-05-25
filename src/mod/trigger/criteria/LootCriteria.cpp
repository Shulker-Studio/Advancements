#include "mod/trigger/criteria/LootCriteria.h"

#include "mod/trigger/criteria/Common.h"

#include <array>

namespace advancements::criteria {
namespace {

constexpr std::array<std::string_view, 4> SupportedBastionLootTables{
    "minecraft:chests/bastion_bridge",
    "minecraft:chests/bastion_hoglin_stable",
    "minecraft:chests/bastion_other",
    "minecraft:chests/bastion_treasure",
};

} // namespace

TriggerCondition compileLootTableCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"loot_table"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("loot_table") || !conditions.at("loot_table").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto const lootTableId = conditions.at("loot_table").get<std::string>();
    if (std::ranges::find(SupportedBastionLootTables, lootTableId) == SupportedBastionLootTables.end()) {
        return InvalidTriggerCondition{};
    }
    return LootTableCondition{lootTableId};
}

bool matchesLootTableCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<LootTableCondition>(&condition);
    auto const* payload  = payloadAs<LootTablePayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->lootTableId == compiled->lootTableId;
}

} // namespace advancements::criteria
