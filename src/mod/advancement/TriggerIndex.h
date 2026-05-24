#pragma once

#include "mod/advancement/AdvancementLoader.h"

#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace my_mod::advancement {

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

using TriggerCondition = std::variant<
    NoTriggerCondition,
    InvalidTriggerCondition,
    ItemTriggerCondition,
    EntityTriggerCondition,
    BlockTriggerCondition,
    DimensionTriggerCondition,
    LocationStructureCondition,
    LootTableCondition,
    PlayerHurtEntityCondition,
    TargetHitCondition,
    EntityHurtPlayerCondition>;

struct CriterionBinding {
    std::string      advancementId;
    std::string      criterionName;
    std::string      triggerId;
    TriggerCondition condition;
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

} // namespace my_mod::advancement
