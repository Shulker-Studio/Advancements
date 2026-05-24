#pragma once

#include "mod/advancement/AdvancementLoader.h"
#include "mod/advancement/ProgressService.h"
#include "mod/advancement/TriggerIndex.h"

#include <filesystem>
#include <optional>
#include <string>
#include <variant>

class Player;

namespace my_mod::advancement {

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
    std::string killedEntityTypeId;
    float       horizontalDistance;
    bool        killingBlowIsProjectile;
};

using TriggerPayload = std::variant<
    NoTriggerPayload,
    ItemTriggerPayload,
    EntityTriggerPayload,
    BlockTriggerPayload,
    DimensionTriggerPayload,
    LocationStructurePayload,
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

class TriggerDispatcher {
public:
    TriggerDispatcher(TriggerIndex const& index, ProgressService& progressService);

    void dispatch(
        std::filesystem::path const& worldDataDir,
        LoadResult const&            definitions,
        TriggerContext const&        context
    ) const;

private:
    [[nodiscard]] bool matches(CriterionBinding const& binding, TriggerContext const& context) const;

    TriggerIndex const& mIndex;
    ProgressService&    mProgressService;
};

} // namespace my_mod::advancement
