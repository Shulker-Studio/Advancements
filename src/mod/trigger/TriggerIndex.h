#pragma once

#include "mod/advancement/AdvancementLoader.h"

#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

class Player;

namespace advancements {

struct NoTriggerPayload {};

struct ItemTriggerPayload {
    std::string        itemId;
    std::optional<int> itemCount;
};

struct EntityTriggerPayload {
    std::string entityTypeId;
};

struct PlayerInteractedWithEntityPayload {
    std::string itemId;
    std::string entityTypeId;
    std::string entityVariantId;
};

struct BredAnimalsPayload {
    std::vector<std::string> childTypeIds;
};

struct TameAnimalPayload {
    std::string                entityTypeId;
    std::string                entityVariantId;
};

struct BlockTriggerPayload {
    std::string blockId;
};

struct EnterBlockPayload {
    std::string blockId;
};

struct ItemUsedOnBlockPayload {
    std::string itemId;
    std::string blockId;
};

struct DimensionTriggerPayload {
    std::string fromDimension;
    std::string toDimension;
};

struct LocationStructurePayload {
    std::string structureId;
};

struct NetherTravelTriggerPayload {
    float horizontalDistance;
};

struct LevitationTriggerPayload {
    float verticalDistance;
};

struct EffectsChangedPayload {};

struct ConstructBeaconTriggerPayload {
    int level;
};

struct BeeNestDestroyedPayload {
    std::string blockId;
    int         numBeesInside;
};

struct LootTablePayload {
    std::string              lootTableId;
    std::vector<std::string> generatedItemIds;
};

struct PlayerHurtEntityPayload {
    bool directEntityIsArrow;
    std::optional<std::string> directEntityTypeId;
    bool isProjectileDamage;
    bool mainhandItemIsMace;
    float damageDealt;
};

struct TargetHitPayload {
    int   signalStrength;
    float projectileHorizontalDistance;
};

struct EntityHurtPlayerPayload {
    bool blockedDamage;
    bool isProjectileDamage;
};

struct PlayerKilledEntitySniperDuelPayload {
    std::string                killedEntityTypeId;
    float                      horizontalDistance;
    bool                       killingBlowIsProjectile;
    std::optional<std::string> directEntityTypeId;
    bool                       killedEntityIsIllagerCaptain;
};

using TriggerPayload = std::variant<
    NoTriggerPayload,
    ItemTriggerPayload,
    EntityTriggerPayload,
    PlayerInteractedWithEntityPayload,
    BredAnimalsPayload,
    TameAnimalPayload,
    BlockTriggerPayload,
    EnterBlockPayload,
    ItemUsedOnBlockPayload,
    DimensionTriggerPayload,
    LocationStructurePayload,
    NetherTravelTriggerPayload,
    LevitationTriggerPayload,
    EffectsChangedPayload,
    ConstructBeaconTriggerPayload,
    BeeNestDestroyedPayload,
    LootTablePayload,
    PlayerHurtEntityPayload,
    TargetHitPayload,
    EntityHurtPlayerPayload,
    PlayerKilledEntitySniperDuelPayload>;

struct TriggerContext {
    Player&        player;
    std::string    triggerId;
    TriggerPayload payload;
};

struct NoTriggerCondition {};

struct InvalidTriggerCondition {};

struct ItemTriggerCondition {
    std::string        itemId;
    std::optional<int> count;
};

struct InventoryItemsCondition {
    std::vector<std::string> requiredItemIds;
};

struct EntityTriggerCondition {
    std::string entityTypeId;
};

struct PlayerInteractedWithEntityCondition {
    std::string itemId;
    std::string entityTypeId;
    std::string entityVariantId;
};

struct BredAnimalsCondition {
    std::string childTypeId;
};

struct TameAnimalCondition {
    std::string                entityTypeId;
    std::string                entityVariantId;
};

struct BlockTriggerCondition {
    std::string blockId;
};

struct EnterBlockCondition {
    std::string blockId;
};

struct ItemUsedOnBlockCondition {
    std::vector<std::string> itemIds;
    std::vector<std::string> blockIds;
};

struct DimensionTriggerCondition {
    std::optional<std::string> fromDimension;
    std::optional<std::string> toDimension;
};

struct LocationStructureCondition {
    std::string structureId;
};

struct NetherTravelTriggerCondition {
    float horizontalDistanceMin;
};

struct LevitationTriggerCondition {
    float verticalDistanceMin;
};

struct EffectsChangedCondition {
    std::vector<std::string> requiredEffects;
};

struct ConstructBeaconTriggerCondition {
    int levelMin;
};

struct LootTableCondition {
    std::string lootTableId;
    bool        requirePotterySherdGenerated;
};

struct VillagerTradeCondition {
    float playerYMin;
};

struct PlayerHurtEntityCondition {
    bool requireArrowDirectEntity;
    bool requireTridentDirectEntity;
    bool requireProjectileDamageTag;
    bool requireMainhandMace;
    std::optional<float> damageDealtMin;
};

struct TargetHitCondition {
    int   requiredSignalStrength;
    float projectileHorizontalDistanceMin;
};

struct EntityHurtPlayerCondition {
    bool requireBlockedDamage;
    bool requireProjectileDamageTag;
};

struct PlayerKilledEntitySniperDuelCondition {
    std::string                targetEntityTypeId;
    std::optional<float>       horizontalDistanceMin;
    bool                       requireProjectileKillingBlow;
    std::optional<std::string> directEntityTypeId;
};

struct PlayerKilledEntityRaidCaptainCondition {};

struct BeeNestDestroyedCondition {
    std::optional<std::string> blockId;
    std::optional<std::string> itemId;
    bool                       requireSilkTouch;
    std::optional<int>         numBeesInsideMin;
};

using TriggerCondition = std::variant<
    NoTriggerCondition,
    InvalidTriggerCondition,
    ItemTriggerCondition,
    InventoryItemsCondition,
    EntityTriggerCondition,
    PlayerInteractedWithEntityCondition,
    BredAnimalsCondition,
    TameAnimalCondition,
    BlockTriggerCondition,
    EnterBlockCondition,
    ItemUsedOnBlockCondition,
    DimensionTriggerCondition,
    LocationStructureCondition,
    NetherTravelTriggerCondition,
    LevitationTriggerCondition,
    EffectsChangedCondition,
    ConstructBeaconTriggerCondition,
    BeeNestDestroyedCondition,
    LootTableCondition,
    VillagerTradeCondition,
    PlayerHurtEntityCondition,
    TargetHitCondition,
    EntityHurtPlayerCondition,
    PlayerKilledEntitySniperDuelCondition,
    PlayerKilledEntityRaidCaptainCondition>;

struct TriggerDescriptor {
    using CompileFn = TriggerCondition (*)(nlohmann::json const& conditions);
    using MatchFn   = bool (*)(TriggerCondition const& condition, TriggerContext const& context);

    std::string_view id;
    CompileFn        compile;
    MatchFn          match;
};

struct CriterionBinding {
    AdvancementDefinition const* advancement;
    std::string                  advancementId;
    std::string                  criterionName;
    std::string                  triggerId;
    TriggerDescriptor const*     descriptor;
    TriggerCondition             condition;
};

class TriggerIndex {
public:
    void rebuild(LoadResult const& result);

    [[nodiscard]] size_t triggerCount() const;
    [[nodiscard]] size_t bindingCount() const;
    [[nodiscard]] std::span<CriterionBinding const> find(std::string_view triggerId) const;

private:
    std::map<std::string, std::vector<CriterionBinding>, std::less<>> mBindings;
    size_t                                                           mBindingCount{0};
};

} // namespace advancements
