#include "mod/advancement/TriggerIndex.h"

#include <algorithm>
#include <array>
#include <utility>

namespace advancements {
namespace {

constexpr std::array<std::string_view, 4> SupportedBastionLootTables{
    "minecraft:chests/bastion_bridge",
    "minecraft:chests/bastion_hoglin_stable",
    "minecraft:chests/bastion_other",
    "minecraft:chests/bastion_treasure",
};

bool hasOnlyKeys(nlohmann::json const& conditions, std::initializer_list<char const*> keys) {
    for (auto const& [key, _] : conditions.items()) {
        if (std::ranges::find(keys, key) == keys.end()) {
            return false;
        }
    }
    return true;
}

TriggerCondition compileBlockCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"block"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("block") || !conditions.at("block").is_string()) {
        return InvalidTriggerCondition{};
    }
    return BlockTriggerCondition{conditions.at("block").get<std::string>()};
}

TriggerCondition compileItemCondition(nlohmann::json const& conditions, bool allowCount) {
    if (!hasOnlyKeys(conditions, allowCount ? std::initializer_list<char const*>{"item", "count"}
                                             : std::initializer_list<char const*>{"item"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("item") || !conditions.at("item").is_string()) {
        return InvalidTriggerCondition{};
    }
    std::optional<int> count;
    if (conditions.contains("count")) {
        if (!allowCount || !conditions.at("count").is_number_integer()) {
            return InvalidTriggerCondition{};
        }
        count = conditions.at("count").get<int>();
    }
    return ItemTriggerCondition{conditions.at("item").get<std::string>(), count};
}

TriggerCondition compileEntityCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"entity"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_string()) {
        return InvalidTriggerCondition{};
    }
    return EntityTriggerCondition{conditions.at("entity").get<std::string>()};
}

TriggerCondition compileChangedDimensionCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"from", "to"})) {
        return InvalidTriggerCondition{};
    }
    std::optional<std::string> fromDimension;
    std::optional<std::string> toDimension;
    if (conditions.contains("from")) {
        if (!conditions.at("from").is_string()) {
            return InvalidTriggerCondition{};
        }
        fromDimension = conditions.at("from").get<std::string>();
    }
    if (conditions.contains("to")) {
        if (!conditions.at("to").is_string()) {
            return InvalidTriggerCondition{};
        }
        toDimension = conditions.at("to").get<std::string>();
    }
    return DimensionTriggerCondition{fromDimension, toDimension};
}

TriggerCondition compileNoCondition(nlohmann::json const&) { return InvalidTriggerCondition{}; }

TriggerCondition compileShotCrossbowCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"item"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("item") || !conditions.at("item").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& item = conditions.at("item");
    if (!hasOnlyKeys(item, {"items"})) {
        return InvalidTriggerCondition{};
    }
    if (!item.contains("items") || !item.at("items").is_string()) {
        return InvalidTriggerCondition{};
    }

    auto const itemId = item.at("items").get<std::string>();
    if (itemId != "minecraft:crossbow") {
        return InvalidTriggerCondition{};
    }
    return ItemTriggerCondition{itemId, std::nullopt};
}

TriggerCondition compileLocationStructureCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"player"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("player") || !conditions.at("player").is_array() || conditions.at("player").size() != 1) {
        return InvalidTriggerCondition{};
    }

    auto const& playerCondition = conditions.at("player").front();
    if (!playerCondition.is_object() || !hasOnlyKeys(playerCondition, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("condition") || !playerCondition.at("condition").is_string()
        || playerCondition.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("entity") || !playerCondition.at("entity").is_string()
        || playerCondition.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!playerCondition.contains("predicate") || !playerCondition.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = playerCondition.at("predicate");
    if (!hasOnlyKeys(predicate, {"location"}) || !predicate.contains("location") || !predicate.at("location").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& location = predicate.at("location");
    if (!hasOnlyKeys(location, {"structures"}) || !location.contains("structures") || !location.at("structures").is_string()) {
        return InvalidTriggerCondition{};
    }

    return LocationStructureCondition{location.at("structures").get<std::string>()};
}

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

TriggerCondition compilePlayerHurtEntityCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"damage"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("damage") || !conditions.at("damage").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& damage = conditions.at("damage");
    if (!hasOnlyKeys(damage, {"type"})) {
        return InvalidTriggerCondition{};
    }
    if (!damage.contains("type") || !damage.at("type").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& type = damage.at("type");
    if (!hasOnlyKeys(type, {"direct_entity", "tags"})) {
        return InvalidTriggerCondition{};
    }

    bool requireArrowDirectEntity   = false;
    bool requireProjectileDamageTag = false;

    if (type.contains("direct_entity")) {
        if (!type.at("direct_entity").is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& directEntity = type.at("direct_entity");
        if (!hasOnlyKeys(directEntity, {"type"})) {
            return InvalidTriggerCondition{};
        }
        if (!directEntity.contains("type") || !directEntity.at("type").is_string()) {
            return InvalidTriggerCondition{};
        }
        if (directEntity.at("type").get<std::string>() != "#minecraft:arrows") {
            return InvalidTriggerCondition{};
        }
        requireArrowDirectEntity = true;
    }

    if (type.contains("tags")) {
        if (!type.at("tags").is_array()) {
            return InvalidTriggerCondition{};
        }
        auto const& tags = type.at("tags");
        if (tags.size() != 1 || !tags.at(0).is_object()) {
            return InvalidTriggerCondition{};
        }
        auto const& tagEntry = tags.at(0);
        if (!hasOnlyKeys(tagEntry, {"id", "expected"})) {
            return InvalidTriggerCondition{};
        }
        if (!tagEntry.contains("id") || !tagEntry.at("id").is_string()) {
            return InvalidTriggerCondition{};
        }
        if (!tagEntry.contains("expected") || !tagEntry.at("expected").is_boolean()) {
            return InvalidTriggerCondition{};
        }
        if (tagEntry.at("id").get<std::string>() != "minecraft:is_projectile"
            || !tagEntry.at("expected").get<bool>()) {
            return InvalidTriggerCondition{};
        }
        requireProjectileDamageTag = true;
    }

    if (!requireArrowDirectEntity || !requireProjectileDamageTag) {
        return InvalidTriggerCondition{};
    }

    return PlayerHurtEntityCondition{requireArrowDirectEntity, requireProjectileDamageTag};
}

TriggerCondition compileTargetHitCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"signal_strength", "projectile"})) {
        return InvalidTriggerCondition{};
    }

    if (!conditions.contains("signal_strength") || !conditions.at("signal_strength").is_number_integer()) {
        return InvalidTriggerCondition{};
    }
    auto const signalStrength = conditions.at("signal_strength").get<int>();
    if (signalStrength != 15) {
        return InvalidTriggerCondition{};
    }

    if (!conditions.contains("projectile") || !conditions.at("projectile").is_array()) {
        return InvalidTriggerCondition{};
    }
    auto const& projectile = conditions.at("projectile");
    if (projectile.size() != 1) {
        return InvalidTriggerCondition{};
    }

    auto const& projectileEntry = projectile.at(0);
    if (!projectileEntry.is_object() || !hasOnlyKeys(projectileEntry, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!projectileEntry.contains("condition") || !projectileEntry.at("condition").is_string()
        || projectileEntry.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!projectileEntry.contains("entity") || !projectileEntry.at("entity").is_string()
        || projectileEntry.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!projectileEntry.contains("predicate") || !projectileEntry.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = projectileEntry.at("predicate");
    if (!hasOnlyKeys(predicate, {"distance"}) || !predicate.contains("distance") || !predicate.at("distance").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& distance = predicate.at("distance");
    if (!hasOnlyKeys(distance, {"horizontal"}) || !distance.contains("horizontal")
        || !distance.at("horizontal").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& horizontal = distance.at("horizontal");
    if (!hasOnlyKeys(horizontal, {"min"}) || !horizontal.contains("min") || !horizontal.at("min").is_number()) {
        return InvalidTriggerCondition{};
    }

    auto const horizontalMin = horizontal.at("min").get<float>();
    if (horizontalMin != 30.0F) {
        return InvalidTriggerCondition{};
    }

    return TargetHitCondition{signalStrength, horizontalMin};
}

TriggerCondition compileEntityHurtPlayerCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"damage"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("damage") || !conditions.at("damage").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& damage = conditions.at("damage");
    if (!hasOnlyKeys(damage, {"blocked", "type"})) {
        return InvalidTriggerCondition{};
    }
    if (!damage.contains("blocked") || !damage.at("blocked").is_boolean() || !damage.at("blocked").get<bool>()) {
        return InvalidTriggerCondition{};
    }
    if (!damage.contains("type") || !damage.at("type").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& type = damage.at("type");
    if (!hasOnlyKeys(type, {"tags"})) {
        return InvalidTriggerCondition{};
    }
    if (!type.contains("tags") || !type.at("tags").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const& tags = type.at("tags");
    if (tags.size() != 1 || !tags.at(0).is_object()) {
        return InvalidTriggerCondition{};
    }
    auto const& tagEntry = tags.at(0);
    if (!hasOnlyKeys(tagEntry, {"id", "expected"})) {
        return InvalidTriggerCondition{};
    }
    if (!tagEntry.contains("id") || !tagEntry.at("id").is_string() || !tagEntry.contains("expected")
        || !tagEntry.at("expected").is_boolean()) {
        return InvalidTriggerCondition{};
    }
    if (tagEntry.at("id").get<std::string>() != "minecraft:is_projectile" || !tagEntry.at("expected").get<bool>()) {
        return InvalidTriggerCondition{};
    }

    return EntityHurtPlayerCondition{true, true};
}

TriggerCondition compilePlayerKilledEntitySniperDuelCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"entity", "killing_blow"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("entity") || !conditions.at("entity").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const& entity = conditions.at("entity");
    if (entity.size() != 1 || !entity.at(0).is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& entityEntry = entity.at(0);
    if (!hasOnlyKeys(entityEntry, {"condition", "entity", "predicate"})) {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("condition") || !entityEntry.at("condition").is_string()
        || entityEntry.at("condition").get<std::string>() != "minecraft:entity_properties") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("entity") || !entityEntry.at("entity").is_string()
        || entityEntry.at("entity").get<std::string>() != "this") {
        return InvalidTriggerCondition{};
    }
    if (!entityEntry.contains("predicate") || !entityEntry.at("predicate").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& predicate = entityEntry.at("predicate");
    if (!hasOnlyKeys(predicate, {"type", "distance"})) {
        return InvalidTriggerCondition{};
    }
    if (!predicate.contains("type") || !predicate.at("type").is_string()
        || predicate.at("type").get<std::string>() != "minecraft:skeleton") {
        return InvalidTriggerCondition{};
    }
    if (!predicate.contains("distance") || !predicate.at("distance").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& distance = predicate.at("distance");
    if (!hasOnlyKeys(distance, {"horizontal"}) || !distance.contains("horizontal")
        || !distance.at("horizontal").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& horizontal = distance.at("horizontal");
    if (!hasOnlyKeys(horizontal, {"min"}) || !horizontal.contains("min") || !horizontal.at("min").is_number()) {
        return InvalidTriggerCondition{};
    }

    auto const horizontalMin = horizontal.at("min").get<float>();
    if (horizontalMin != 50.0F) {
        return InvalidTriggerCondition{};
    }

    if (!conditions.contains("killing_blow") || !conditions.at("killing_blow").is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& killingBlow = conditions.at("killing_blow");
    if (!hasOnlyKeys(killingBlow, {"tags"}) || !killingBlow.contains("tags") || !killingBlow.at("tags").is_array()) {
        return InvalidTriggerCondition{};
    }

    auto const& tags = killingBlow.at("tags");
    if (tags.size() != 1 || !tags.at(0).is_object()) {
        return InvalidTriggerCondition{};
    }

    auto const& tagEntry = tags.at(0);
    if (!hasOnlyKeys(tagEntry, {"id", "expected"})) {
        return InvalidTriggerCondition{};
    }
    if (!tagEntry.contains("id") || !tagEntry.at("id").is_string()) {
        return InvalidTriggerCondition{};
    }
    if (!tagEntry.contains("expected") || !tagEntry.at("expected").is_boolean()) {
        return InvalidTriggerCondition{};
    }
    if (tagEntry.at("id").get<std::string>() != "minecraft:is_projectile" || !tagEntry.at("expected").get<bool>()) {
        return InvalidTriggerCondition{};
    }

    return PlayerKilledEntitySniperDuelCondition{"minecraft:skeleton", horizontalMin, true};
}

TriggerCondition compileTriggerCondition(std::string_view triggerId, std::optional<nlohmann::json> const& rawConditions) {
    if (!rawConditions) {
        return NoTriggerCondition{};
    }

    auto const& conditions = *rawConditions;
    if (!conditions.is_object()) {
        return InvalidTriggerCondition{};
    }
    if (conditions.empty()) {
        return NoTriggerCondition{};
    }

    if (triggerId == "bedrock:player_destroy_block") {
        return compileBlockCondition(conditions);
    }
    if (triggerId == "minecraft:inventory_changed") {
        return compileItemCondition(conditions, true);
    }
    if (triggerId == "minecraft:consume_item" || triggerId == "minecraft:used_totem"
        || triggerId == "minecraft:fishing_rod_hooked" || triggerId == "minecraft:filled_bucket") {
        return compileItemCondition(conditions, false);
    }
    if (triggerId == "minecraft:shot_crossbow") {
        return compileShotCrossbowCondition(conditions);
    }
    if (triggerId == "minecraft:player_killed_entity") {
        auto compiled = compilePlayerKilledEntitySniperDuelCondition(conditions);
        if (!std::holds_alternative<InvalidTriggerCondition>(compiled)) {
            return std::move(compiled);
        }
        return compileEntityCondition(conditions);
    }
    if (triggerId == "minecraft:entity_killed_player") {
        return compileEntityCondition(conditions);
    }
    if (triggerId == "minecraft:changed_dimension") {
        return compileChangedDimensionCondition(conditions);
    }
    if (triggerId == "minecraft:location") {
        return compileLocationStructureCondition(conditions);
    }
    if (triggerId == "minecraft:player_generates_container_loot") {
        return compileLootTableCondition(conditions);
    }
    if (triggerId == "minecraft:player_hurt_entity") {
        return compilePlayerHurtEntityCondition(conditions);
    }
    if (triggerId == "minecraft:target_hit") {
        return compileTargetHitCondition(conditions);
    }
    if (triggerId == "minecraft:entity_hurt_player") {
        return compileEntityHurtPlayerCondition(conditions);
    }
    if (triggerId == "minecraft:villager_trade" || triggerId == "minecraft:enchanted_item") {
        return InvalidTriggerCondition{};
    }
    return InvalidTriggerCondition{};
}

template <typename T>
T const* payloadAs(TriggerContext const& context) {
    return std::get_if<T>(&context.payload);
}

bool matchesNoCondition(TriggerCondition const& condition, TriggerContext const&) {
    return std::holds_alternative<NoTriggerCondition>(condition);
}

bool matchesBlockCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<BlockTriggerCondition>(&condition);
    auto const* payload  = payloadAs<BlockTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->blockId == compiled->blockId;
}

bool matchesInventoryItemCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<ItemTriggerCondition>(&condition);
    auto const* payload  = payloadAs<ItemTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (payload->itemId != compiled->itemId) {
        return false;
    }
    if (!compiled->count) {
        return true;
    }
    if (!payload->itemCount) {
        return false;
    }
    return *payload->itemCount >= *compiled->count;
}

bool matchesSimpleItemCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<ItemTriggerCondition>(&condition);
    auto const* payload  = payloadAs<ItemTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->itemId == compiled->itemId;
}

bool matchesEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<EntityTriggerCondition>(&condition);
    auto const* payload  = payloadAs<EntityTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    return payload->entityTypeId == compiled->entityTypeId;
}

bool matchesChangedDimensionCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<DimensionTriggerCondition>(&condition);
    auto const* payload  = payloadAs<DimensionTriggerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->fromDimension && payload->fromDimension != *compiled->fromDimension) {
        return false;
    }
    if (compiled->toDimension && payload->toDimension != *compiled->toDimension) {
        return false;
    }
    return true;
}

constexpr TriggerDescriptor MigratedDescriptors[]{
    {"bedrock:player_destroy_block", compileBlockCondition, matchesBlockCondition},
    {"minecraft:changed_dimension", compileChangedDimensionCondition, matchesChangedDimensionCondition},
    {"minecraft:consume_item", [](nlohmann::json const& conditions) { return compileItemCondition(conditions, false); }, matchesSimpleItemCondition},
    {"minecraft:entity_killed_player", compileEntityCondition, matchesEntityCondition},
    {"minecraft:filled_bucket", [](nlohmann::json const& conditions) { return compileItemCondition(conditions, false); }, matchesSimpleItemCondition},
    {"minecraft:fishing_rod_hooked", [](nlohmann::json const& conditions) { return compileItemCondition(conditions, false); }, matchesSimpleItemCondition},
    {"minecraft:inventory_changed", [](nlohmann::json const& conditions) { return compileItemCondition(conditions, true); }, matchesInventoryItemCondition},
    {"minecraft:slept_in_bed", compileNoCondition, matchesNoCondition},
    {"minecraft:used_totem", [](nlohmann::json const& conditions) { return compileItemCondition(conditions, false); }, matchesSimpleItemCondition},
};

TriggerDescriptor const* findTriggerDescriptor(std::string_view triggerId) {
    auto const found = std::ranges::find_if(MigratedDescriptors, [triggerId](TriggerDescriptor const& descriptor) {
        return descriptor.id == triggerId;
    });
    if (found == std::end(MigratedDescriptors)) {
        return nullptr;
    }
    return &*found;
}

TriggerCondition compileDescriptorCondition(
    TriggerDescriptor const&             descriptor,
    std::optional<nlohmann::json> const& rawConditions
) {
    if (!rawConditions) {
        return NoTriggerCondition{};
    }

    auto const& conditions = *rawConditions;
    if (!conditions.is_object()) {
        return InvalidTriggerCondition{};
    }
    if (conditions.empty()) {
        return NoTriggerCondition{};
    }
    return descriptor.compile(conditions);
}

bool shouldKeepLegacyBinding(std::string_view triggerId, std::optional<nlohmann::json> const& rawConditions) {
    if (triggerId != "minecraft:player_killed_entity" || !rawConditions || !rawConditions->is_object()
        || rawConditions->empty()) {
        return false;
    }

    return !std::holds_alternative<InvalidTriggerCondition>(compilePlayerKilledEntitySniperDuelCondition(*rawConditions));
}

} // namespace

void TriggerIndex::rebuild(LoadResult const& result) {
    mBindings.clear();
    mBindingCount = 0;

    for (auto const& [advancementId, advancement] : result.advancements) {
        for (auto const& [criterionName, criterion] : advancement.criteria) {
            auto const* descriptor = findTriggerDescriptor(criterion.trigger);
            if (descriptor != nullptr && shouldKeepLegacyBinding(criterion.trigger, criterion.conditions)) {
                descriptor = nullptr;
            }
            mBindings[criterion.trigger].push_back(CriterionBinding{
                &advancement,
                advancementId,
                criterionName,
                criterion.trigger,
                descriptor,
                descriptor == nullptr ? compileTriggerCondition(criterion.trigger, criterion.conditions)
                                      : compileDescriptorCondition(*descriptor, criterion.conditions)
            });
            ++mBindingCount;
        }
    }
}

size_t TriggerIndex::triggerCount() const { return mBindings.size(); }

size_t TriggerIndex::bindingCount() const { return mBindingCount; }

std::span<CriterionBinding const> TriggerIndex::find(std::string_view triggerId) const {
    auto const found = mBindings.find(triggerId);
    if (found == mBindings.end()) {
        return {};
    }
    return found->second;
}

} // namespace advancements
