#include "mod/trigger/criteria/DamageCriteria.h"

#include "mod/predicate/DamagePredicate.h"
#include "mod/trigger/criteria/Common.h"

namespace advancements::criteria {

TriggerCondition compilePlayerHurtEntityCondition(nlohmann::json const& conditions) {
    auto const predicate = predicate::parsePlayerHurtEntityDamagePredicate(conditions);
    if (!predicate) {
        return InvalidTriggerCondition{};
    }

    bool requireArrowDirectEntity   = predicate->directEntityTypeId == "#minecraft:arrows";
    bool requireTridentDirectEntity = predicate->directEntityTypeId == "minecraft:thrown_trident";
    bool requireMainhandMace        = predicate->directEntityMainhandItemId == "minecraft:mace" || predicate->requireMaceSmashDamageTag;

    if (predicate->directEntityTypeId && !requireArrowDirectEntity && !requireTridentDirectEntity) {
        return InvalidTriggerCondition{};
    }
    if (predicate->directEntityMainhandItemId && predicate->directEntityMainhandItemId != "minecraft:mace") {
        return InvalidTriggerCondition{};
    }

    if (requireArrowDirectEntity || requireTridentDirectEntity || predicate->requireProjectileDamageTag) {
        if ((!requireArrowDirectEntity && !requireTridentDirectEntity) || !predicate->requireProjectileDamageTag || requireMainhandMace
            || predicate->dealtMin.has_value()) {
            return InvalidTriggerCondition{};
        }
        return PlayerHurtEntityCondition{requireArrowDirectEntity, requireTridentDirectEntity, true, false, std::nullopt};
    }

    if (!requireMainhandMace || !predicate->dealtMin.has_value()) {
        return InvalidTriggerCondition{};
    }

    return PlayerHurtEntityCondition{false, false, false, true, predicate->dealtMin};
}

TriggerCondition compileEntityHurtPlayerCondition(nlohmann::json const& conditions) {
    auto const predicate = predicate::parseBlockedProjectileDamagePredicate(conditions);
    if (!predicate) {
        return InvalidTriggerCondition{};
    }

    return EntityHurtPlayerCondition{predicate->requireBlockedDamage, predicate->requireProjectileDamageTag};
}

bool matchesPlayerHurtEntityCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<PlayerHurtEntityCondition>(&condition);
    auto const* payload  = payloadAs<PlayerHurtEntityPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->requireArrowDirectEntity && !payload->directEntityIsArrow) {
        return false;
    }
    if (compiled->requireTridentDirectEntity && payload->directEntityTypeId != "minecraft:thrown_trident") {
        return false;
    }
    if (compiled->requireProjectileDamageTag && !payload->isProjectileDamage) {
        return false;
    }
    if (compiled->requireMainhandMace && !payload->mainhandItemIsMace) {
        return false;
    }
    if (compiled->damageDealtMin.has_value() && payload->damageDealt < *compiled->damageDealtMin) {
        return false;
    }
    return true;
}

bool matchesEntityHurtPlayerCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<EntityHurtPlayerCondition>(&condition);
    auto const* payload  = payloadAs<EntityHurtPlayerPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }
    if (compiled->requireBlockedDamage && !payload->blockedDamage) {
        return false;
    }
    if (compiled->requireProjectileDamageTag && !payload->isProjectileDamage) {
        return false;
    }
    return true;
}

} // namespace advancements::criteria
