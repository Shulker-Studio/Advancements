#include "mod/predicate/LocationPredicate.h"

#include "mod/predicate/Common.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/levelgen/structure/VanillaStructureFeatureType.h"

namespace advancements::predicate {
namespace {

std::optional<PositionRangePredicate> parsePositionRangePredicate(nlohmann::json const& predicate) {
    if (!predicate.is_object() || !hasOnlyKeys(predicate, {"y"})) {
        return std::nullopt;
    }
    if (!predicate.contains("y") || !predicate.at("y").is_object()) {
        return std::nullopt;
    }

    auto const& y = predicate.at("y");
    if (!hasOnlyKeys(y, {"min"}) || !y.contains("min") || !y.at("min").is_number()) {
        return std::nullopt;
    }

    return PositionRangePredicate{y.at("min").get<float>()};
}

} // namespace

std::optional<LocationPredicate> parseLocationPredicate(nlohmann::json const& predicate) {
    if (!predicate.is_object() || !hasOnlyKeys(predicate, {"structures", "position"})) {
        return std::nullopt;
    }

    std::optional<std::string> structureId;
    if (predicate.contains("structures")) {
        if (!predicate.at("structures").is_string()) {
            return std::nullopt;
        }
        structureId = predicate.at("structures").get<std::string>();
    }

    std::optional<PositionRangePredicate> position;
    if (predicate.contains("position")) {
        if (!predicate.at("position").is_object()) {
            return std::nullopt;
        }
        position = parsePositionRangePredicate(predicate.at("position"));
        if (!position) {
            return std::nullopt;
        }
    }

    if (!structureId && !position) {
        return std::nullopt;
    }

    return LocationPredicate{std::move(structureId), position};
}

std::optional<std::string> currentSupportedLocationStructure(Player& player) {
    auto const& currentStructure = player.getCurrentStructureFeature();
    if (currentStructure == VanillaStructureFeatureType::Bastion()) {
        return "minecraft:bastion_remnant";
    }
    if (currentStructure == VanillaStructureFeatureType::Fortress()) {
        return "minecraft:fortress";
    }
    if (currentStructure == VanillaStructureFeatureType::EndCity()) {
        return "minecraft:end_city";
    }
    if (currentStructure == VanillaStructureFeatureType::Stronghold()) {
        return "minecraft:stronghold";
    }
    if (currentStructure == VanillaStructureFeatureType::TrialChambers()) {
        return "minecraft:trial_chambers";
    }
    return std::nullopt;
}

bool matchesLocationPredicate(LocationPredicate const& predicate, std::string const& structureId) {
    return predicate.structureId && *predicate.structureId == structureId;
}

} // namespace advancements::predicate
