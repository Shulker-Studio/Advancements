#include "mod/predicate/LocationPredicate.h"

#include "mod/predicate/Common.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/levelgen/structure/VanillaStructureFeatureType.h"

namespace advancements::predicate {

std::optional<LocationPredicate> parseLocationPredicate(nlohmann::json const& predicate) {
    if (!predicate.is_object() || !hasOnlyKeys(predicate, {"structures"})) {
        return std::nullopt;
    }
    if (!predicate.contains("structures") || !predicate.at("structures").is_string()) {
        return std::nullopt;
    }

    return LocationPredicate{predicate.at("structures").get<std::string>()};
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
    return predicate.structureId == structureId;
}

} // namespace advancements::predicate
