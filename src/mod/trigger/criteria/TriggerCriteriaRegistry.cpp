#include "mod/trigger/criteria/TriggerCriteriaRegistry.h"

#include "mod/trigger/criteria/BlockCriteria.h"
#include "mod/trigger/criteria/DamageCriteria.h"
#include "mod/trigger/criteria/DimensionCriteria.h"
#include "mod/trigger/criteria/EntityCriteria.h"
#include "mod/trigger/criteria/ItemCriteria.h"
#include "mod/trigger/criteria/LootCriteria.h"
#include "mod/trigger/criteria/NoCriteria.h"
#include "mod/trigger/criteria/ProjectileCriteria.h"
#include "mod/trigger/criteria/StructureCriteria.h"

#include <algorithm>

namespace advancements::criteria {
namespace {

constexpr TriggerDescriptor MigratedDescriptors[]{
    {"bedrock:player_destroy_block", compileBlockCondition, matchesBlockCondition},
    {"minecraft:changed_dimension", compileChangedDimensionCondition, matchesChangedDimensionCondition},
    {"minecraft:consume_item", compileSimpleItemCondition, matchesSimpleItemCondition},
    {"minecraft:cured_zombie_villager", compileNoCondition, matchesNoCondition},
    {"minecraft:entity_killed_player", compileEntityCondition, matchesEntityCondition},
    {"minecraft:entity_hurt_player", compileEntityHurtPlayerCondition, matchesEntityHurtPlayerCondition},
    {"minecraft:filled_bucket", compileSimpleItemCondition, matchesSimpleItemCondition},
    {"minecraft:fishing_rod_hooked", compileSimpleItemCondition, matchesSimpleItemCondition},
    {"minecraft:inventory_changed", compileInventoryItemCondition, matchesInventoryItemCondition},
    {"minecraft:location", compileLocationStructureCondition, matchesLocationStructureCondition},
    {"minecraft:player_generates_container_loot", compileLootTableCondition, matchesLootTableCondition},
    {"minecraft:player_hurt_entity", compilePlayerHurtEntityCondition, matchesPlayerHurtEntityCondition},
    {"minecraft:player_killed_entity", compilePlayerKilledEntityCondition, matchesPlayerKilledEntityCondition},
    {"minecraft:shot_crossbow", compileShotCrossbowCondition, matchesSimpleItemCondition},
    {"minecraft:slept_in_bed", compileNoCondition, matchesNoCondition},
    {"minecraft:target_hit", compileTargetHitCondition, matchesTargetHitCondition},
    {"minecraft:used_totem", compileSimpleItemCondition, matchesSimpleItemCondition},
};

} // namespace

TriggerDescriptor const* findTriggerDescriptor(std::string_view triggerId) {
    auto const found = std::ranges::find_if(MigratedDescriptors, [triggerId](TriggerDescriptor const& descriptor) {
        return descriptor.id == triggerId;
    });
    if (found == std::end(MigratedDescriptors)) {
        return nullptr;
    }
    return &*found;
}

} // namespace advancements::criteria
