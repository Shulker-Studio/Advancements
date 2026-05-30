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

constexpr std::array<std::string_view, 6> SupportedArchaeologyLootTables{
    "minecraft:entities/desert_well_brushable_block",
    "minecraft:entities/desert_pyramid_brushable_block",
    "minecraft:entities/cold_ocean_ruins_brushable_block",
    "minecraft:entities/warm_ocean_ruins_brushable_block",
    "minecraft:entities/trail_ruins_brushable_block_common",
    "minecraft:entities/trail_ruins_brushable_block_rare",
};

std::string bedrockArchaeologyLootTableId(std::string_view javaLootTableId) {
    if (javaLootTableId == "minecraft:archaeology/desert_well") {
        return "minecraft:entities/desert_well_brushable_block";
    }
    if (javaLootTableId == "minecraft:archaeology/desert_pyramid") {
        return "minecraft:entities/desert_pyramid_brushable_block";
    }
    if (javaLootTableId == "minecraft:archaeology/ocean_ruin_cold") {
        return "minecraft:entities/cold_ocean_ruins_brushable_block";
    }
    if (javaLootTableId == "minecraft:archaeology/ocean_ruin_warm") {
        return "minecraft:entities/warm_ocean_ruins_brushable_block";
    }
    if (javaLootTableId == "minecraft:archaeology/trail_ruins_common") {
        return "minecraft:entities/trail_ruins_brushable_block_common";
    }
    if (javaLootTableId == "minecraft:archaeology/trail_ruins_rare") {
        return "minecraft:entities/trail_ruins_brushable_block_rare";
    }
    return {};
}

bool isSupportedBastionLootTable(std::string_view lootTableId) {
    return std::ranges::find(SupportedBastionLootTables, lootTableId) != SupportedBastionLootTables.end();
}

bool isSupportedArchaeologyLootTable(std::string_view lootTableId) {
    return std::ranges::find(SupportedArchaeologyLootTables, lootTableId) != SupportedArchaeologyLootTables.end();
}

bool isPotterySherd(std::string_view itemId) {
    return itemId.starts_with("minecraft:") && itemId.ends_with("_pottery_sherd");
}

} // namespace

TriggerCondition compileLootTableCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"loot_table"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("loot_table") || !conditions.at("loot_table").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto lootTableId                   = conditions.at("loot_table").get<std::string>();
    auto requirePotterySherdGenerated = false;

    auto const bedrockArchaeologyLootTable = bedrockArchaeologyLootTableId(lootTableId);
    if (!bedrockArchaeologyLootTable.empty()) {
        lootTableId                   = bedrockArchaeologyLootTable;
        requirePotterySherdGenerated = true;
    }

    if (!isSupportedBastionLootTable(lootTableId) && !isSupportedArchaeologyLootTable(lootTableId)) {
        return InvalidTriggerCondition{};
    }
    return LootTableCondition{lootTableId, requirePotterySherdGenerated};
}

bool matchesLootTableCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<LootTableCondition>(&condition);
    auto const* payload  = payloadAs<LootTablePayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (payload->lootTableId != compiled->lootTableId) {
        return false;
    }
    if (!compiled->requirePotterySherdGenerated) {
        return true;
    }
    return std::ranges::any_of(payload->generatedItemIds, isPotterySherd);
}

} // namespace advancements::criteria
