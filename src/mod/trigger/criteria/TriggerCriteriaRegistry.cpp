#include "mod/trigger/criteria/TriggerCriteriaRegistry.h"

#include "mod/trigger/TriggerRegistry.h"
#include "mod/trigger/criteria/BeaconCriteria.h"
#include "mod/trigger/criteria/BlockCriteria.h"
#include "mod/trigger/criteria/DamageCriteria.h"
#include "mod/trigger/criteria/DimensionCriteria.h"
#include "mod/trigger/criteria/EffectsCriteria.h"
#include "mod/trigger/criteria/EntityCriteria.h"
#include "mod/trigger/criteria/ItemCriteria.h"
#include "mod/trigger/criteria/LevitationCriteria.h"
#include "mod/trigger/criteria/LootCriteria.h"
#include "mod/trigger/criteria/NetherTravelCriteria.h"
#include "mod/trigger/criteria/NoCriteria.h"
#include "mod/trigger/criteria/ProjectileCriteria.h"
#include "mod/trigger/criteria/StructureCriteria.h"

namespace advancements::criteria {
namespace {

constexpr TriggerRegistration MigratedDescriptors[]{
    {"minecraft:changed_dimension", compileChangedDimensionCondition, matchesChangedDimensionCondition},
    {"minecraft:channeled_lightning", compileChanneledLightningCondition, matchesChanneledLightningCondition},
    {"minecraft:brewed_potion", compileNoCondition, matchesNoCondition},
    {"minecraft:construct_beacon", compileConstructBeaconCondition, matchesConstructBeaconCondition},
    {"minecraft:consume_item", compileSimpleItemCondition, matchesSimpleItemCondition},
    {"minecraft:cured_zombie_villager", compileNoCondition, matchesNoCondition},
    {"minecraft:enter_block", compileEnterBlockCondition, matchesEnterBlockCondition},
    {"minecraft:entity_killed_player", compileEntityCondition, matchesEntityCondition},
    {"minecraft:entity_hurt_player", compileEntityHurtPlayerCondition, matchesEntityHurtPlayerCondition},
    {"minecraft:effects_changed", compileEffectsChangedCondition, matchesEffectsChangedCondition},
    {"minecraft:enchanted_item", compileNoCondition, matchesNoCondition},
    {"minecraft:filled_bucket", compileSimpleItemCondition, matchesSimpleItemCondition},
    {"minecraft:fishing_rod_hooked", compileSimpleItemCondition, matchesSimpleItemCondition},
    {"minecraft:inventory_changed", compileInventoryItemCondition, matchesInventoryItemCondition},
    {"minecraft:item_used_on_block", compileItemUsedOnBlockCondition, matchesItemUsedOnBlockCondition},
    {"minecraft:levitation", compileLevitationCondition, matchesLevitationCondition},
    {"minecraft:location", compileLocationStructureCondition, matchesLocationStructureCondition},
    {"minecraft:nether_travel", compileNetherTravelCondition, matchesNetherTravelCondition},
    {"minecraft:player_generates_container_loot", compileLootTableCondition, matchesLootTableCondition},
    {"minecraft:player_hurt_entity", compilePlayerHurtEntityCondition, matchesPlayerHurtEntityCondition},
    {"minecraft:player_interacted_with_entity", compilePlayerInteractedWithEntityCondition, matchesPlayerInteractedWithEntityCondition},
    {"minecraft:player_killed_entity", compilePlayerKilledEntityCondition, matchesPlayerKilledEntityCondition},
    {"minecraft:shot_crossbow", compileShotCrossbowCondition, matchesSimpleItemCondition},
    {"minecraft:slept_in_bed", compileNoCondition, matchesNoCondition},
    {"minecraft:summoned_entity", compileSummonedEntityCondition, matchesEntityCondition},
    {"minecraft:target_hit", compileTargetHitCondition, matchesTargetHitCondition},
    {"minecraft:used_totem", compileSimpleItemCondition, matchesSimpleItemCondition},
    {"minecraft:villager_trade", compileVillagerTradeCondition, matchesVillagerTradeCondition},
};

TriggerRegistry const& currentTriggerRegistry() {
    static TriggerRegistry registry = [] {
        TriggerRegistry seededRegistry;
        for (auto const& descriptor : MigratedDescriptors) {
            seededRegistry.registerTrigger(descriptor);
        }
        return seededRegistry;
    }();
    return registry;
}

} // namespace

TriggerDescriptor const* findTriggerDescriptor(std::string_view triggerId) {
    return currentTriggerRegistry().find(triggerId);
}

} // namespace advancements::criteria
