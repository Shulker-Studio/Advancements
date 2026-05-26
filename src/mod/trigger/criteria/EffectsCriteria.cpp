#include "mod/trigger/criteria/EffectsCriteria.h"

#include "mod/trigger/criteria/Common.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/effect/MobEffect.h"

#include <algorithm>
#include <array>
#include <optional>

namespace advancements::criteria {
namespace {

struct EffectBinding {
    std::string_view id;
    MobEffect*& (*effect)();
};

std::array const SupportedEffects{
    EffectBinding{"minecraft:absorption", &MobEffect::ABSORPTION},
    EffectBinding{"minecraft:bad_omen", &MobEffect::BAD_OMEN},
    EffectBinding{"minecraft:blindness", &MobEffect::BLINDNESS},
    EffectBinding{"minecraft:conduit_power", &MobEffect::CONDUIT_POWER},
    EffectBinding{"minecraft:darkness", &MobEffect::DARKNESS},
    EffectBinding{"minecraft:fire_resistance", &MobEffect::FIRE_RESISTANCE},
    EffectBinding{"minecraft:haste", &MobEffect::DIG_SPEED},
    EffectBinding{"minecraft:health_boost", &MobEffect::HEALTH_BOOST},
    EffectBinding{"minecraft:hero_of_the_village", &MobEffect::HERO_OF_THE_VILLAGE},
    EffectBinding{"minecraft:hunger", &MobEffect::HUNGER},
    EffectBinding{"minecraft:invisibility", &MobEffect::INVISIBILITY},
    EffectBinding{"minecraft:jump_boost", &MobEffect::JUMP},
    EffectBinding{"minecraft:levitation", &MobEffect::LEVITATION},
    EffectBinding{"minecraft:mining_fatigue", &MobEffect::DIG_SLOWDOWN},
    EffectBinding{"minecraft:nausea", &MobEffect::CONFUSION},
    EffectBinding{"minecraft:night_vision", &MobEffect::NIGHT_VISION},
    EffectBinding{"minecraft:poison", &MobEffect::POISON},
    EffectBinding{"minecraft:raid_omen", &MobEffect::RAID_OMEN},
    EffectBinding{"minecraft:regeneration", &MobEffect::REGENERATION},
    EffectBinding{"minecraft:resistance", &MobEffect::DAMAGE_RESISTANCE},
    EffectBinding{"minecraft:saturation", &MobEffect::SATURATION},
    EffectBinding{"minecraft:slow_falling", &MobEffect::SLOW_FALLING},
    EffectBinding{"minecraft:slowness", &MobEffect::MOVEMENT_SLOWDOWN},
    EffectBinding{"minecraft:speed", &MobEffect::MOVEMENT_SPEED},
    EffectBinding{"minecraft:strength", &MobEffect::DAMAGE_BOOST},
    EffectBinding{"minecraft:trial_omen", &MobEffect::TRIAL_OMEN},
    EffectBinding{"minecraft:village_hero", &MobEffect::HERO_OF_THE_VILLAGE},
    EffectBinding{"minecraft:water_breathing", &MobEffect::WATER_BREATHING},
    EffectBinding{"minecraft:weakness", &MobEffect::WEAKNESS},
    EffectBinding{"minecraft:infested", &MobEffect::INFESTED},
    EffectBinding{"minecraft:oozing", &MobEffect::OOZING},
    EffectBinding{"minecraft:weaving", &MobEffect::WEAVING},
    EffectBinding{"minecraft:wind_charged", &MobEffect::WIND_CHARGED},
    EffectBinding{"minecraft:wither", &MobEffect::WITHER},
};

std::string normalizeEffectId(std::string_view effectId) {
    if (effectId == "minecraft:hero_of_the_village") {
        return "minecraft:village_hero";
    }
    return std::string{effectId};
}

std::optional<EffectBinding> supportedEffect(std::string const& effectId) {
    auto const normalized = normalizeEffectId(effectId);
    auto const found = std::ranges::find_if(SupportedEffects, [&normalized](EffectBinding const& binding) {
        return binding.id == normalized;
    });
    if (found == SupportedEffects.end()) {
        return std::nullopt;
    }
    return *found;
}

bool playerHasEffect(Player const& player, std::string const& effectId) {
    auto const binding = supportedEffect(effectId);
    if (!binding) {
        return false;
    }
    auto* effect = binding->effect();
    return effect != nullptr && player.hasEffect(*effect);
}

std::optional<EffectBinding> supportedEffect(std::string_view effectId) {
    auto const normalized = normalizeEffectId(effectId);
    auto const found = std::ranges::find_if(SupportedEffects, [&normalized](EffectBinding const& binding) {
        return binding.id == normalized;
    });
    if (found == SupportedEffects.end()) {
        return std::nullopt;
    }
    return *found;
}

} // namespace

TriggerCondition compileEffectsChangedCondition(nlohmann::json const& conditions) {
    if (!hasOnlyKeys(conditions, {"effects"})) {
        return InvalidTriggerCondition{};
    }
    if (!conditions.contains("effects") || !conditions.at("effects").is_object()) {
        return InvalidTriggerCondition{};
    }

    std::vector<std::string> requiredEffects;
    for (auto const& [effectId, predicate] : conditions.at("effects").items()) {
        if (!predicate.is_object() || !predicate.empty()) {
            return InvalidTriggerCondition{};
        }
        if (!supportedEffect(std::string_view{effectId})) {
            return InvalidTriggerCondition{};
        }
        requiredEffects.push_back(normalizeEffectId(effectId));
    }

    if (requiredEffects.empty()) {
        return InvalidTriggerCondition{};
    }
    return EffectsChangedCondition{std::move(requiredEffects)};
}

bool matchesEffectsChangedCondition(TriggerCondition const& condition, TriggerContext const& context) {
    auto const* compiled = std::get_if<EffectsChangedCondition>(&condition);
    auto const* payload  = payloadAs<EffectsChangedPayload>(context);
    if (compiled == nullptr || payload == nullptr) {
        return false;
    }

    return std::ranges::all_of(compiled->requiredEffects, [&context](std::string const& effectId) {
        return playerHasEffect(context.player, effectId);
    });
}

} // namespace advancements::criteria
