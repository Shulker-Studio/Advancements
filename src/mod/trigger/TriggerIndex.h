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

struct BlockTriggerPayload {
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

struct LootTablePayload {
    std::string lootTableId;
};

struct PlayerHurtEntityPayload {
    bool directEntityIsArrow;
    bool isProjectileDamage;
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
};

using TriggerPayload = std::variant<
    NoTriggerPayload,
    ItemTriggerPayload,
    EntityTriggerPayload,
    BlockTriggerPayload,
    DimensionTriggerPayload,
    LocationStructurePayload,
    NetherTravelTriggerPayload,
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

struct EntityTriggerCondition {
    std::string entityTypeId;
};

struct BlockTriggerCondition {
    std::string blockId;
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

struct LootTableCondition {
    std::string lootTableId;
};

struct PlayerHurtEntityCondition {
    bool requireArrowDirectEntity;
    bool requireProjectileDamageTag;
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

using TriggerCondition = std::variant<
    NoTriggerCondition,
    InvalidTriggerCondition,
    ItemTriggerCondition,
    EntityTriggerCondition,
    BlockTriggerCondition,
    DimensionTriggerCondition,
    LocationStructureCondition,
    NetherTravelTriggerCondition,
    LootTableCondition,
    PlayerHurtEntityCondition,
    TargetHitCondition,
    EntityHurtPlayerCondition,
    PlayerKilledEntitySniperDuelCondition>;

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
