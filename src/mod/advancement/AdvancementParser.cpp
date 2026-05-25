#include "mod/advancement/AdvancementParser.h"

#include <format>

namespace advancements {
namespace {

void addTypeError(std::vector<std::string>& errors, std::string_view field, std::string_view expected) {
    errors.emplace_back(std::format("{} must be {}", field, expected));
}

bool requireObject(nlohmann::json const& json, std::vector<std::string>& errors, std::string_view field) {
    if (!json.contains(field)) {
        errors.emplace_back(std::format("missing required field {}", field));
        return false;
    }
    if (!json.at(field).is_object()) {
        addTypeError(errors, field, "an object");
        return false;
    }
    return true;
}

std::optional<std::string> parseStringField(
    nlohmann::json const&      json,
    std::vector<std::string>&  errors,
    std::string_view           field,
    bool                       required
) {
    if (!json.contains(field)) {
        if (required) {
            errors.emplace_back(std::format("missing required field {}", field));
        }
        return std::nullopt;
    }
    if (!json.at(field).is_string()) {
        addTypeError(errors, field, "a string");
        return std::nullopt;
    }
    return json.at(field).get<std::string>();
}

std::optional<bool> parseBoolField(
    nlohmann::json const&      json,
    std::vector<std::string>&  errors,
    std::string_view           field
) {
    if (!json.contains(field)) {
        return std::nullopt;
    }
    if (!json.at(field).is_boolean()) {
        addTypeError(errors, field, "a boolean");
        return std::nullopt;
    }
    return json.at(field).get<bool>();
}

std::optional<TextComponent> parseTextComponent(
    nlohmann::json const&      json,
    std::vector<std::string>&  errors,
    std::string_view           field
) {
    if (!json.contains(field)) {
        errors.emplace_back(std::format("missing required field {}", field));
        return std::nullopt;
    }
    auto const& value = json.at(field);
    if (!value.is_object()) {
        addTypeError(errors, field, "a text component object");
        return std::nullopt;
    }
    bool const hasText      = value.contains("text");
    bool const hasTranslate = value.contains("translate");
    if (hasText == hasTranslate) {
        errors.emplace_back(std::format("{} must contain exactly one of text or translate", field));
        return std::nullopt;
    }
    auto const key = hasText ? "text" : "translate";
    if (!value.at(key).is_string()) {
        errors.emplace_back(std::format("{}.{} must be a string", field, key));
        return std::nullopt;
    }
    return TextComponent{hasText ? TextComponentKind::Text : TextComponentKind::Translate, value.at(key).get<std::string>()};
}

std::optional<Icon> parseIcon(nlohmann::json const& json, std::vector<std::string>& errors) {
    if (!json.contains("icon")) {
        return std::nullopt;
    }
    auto const& iconJson = json.at("icon");
    if (!iconJson.is_object()) {
        addTypeError(errors, "icon", "an object");
        return std::nullopt;
    }
    auto item = parseStringField(iconJson, errors, "item", true);
    if (!item) {
        return std::nullopt;
    }
    return Icon{std::move(*item)};
}

std::optional<Display> parseDisplay(nlohmann::json const& json, std::vector<std::string>& errors) {
    if (!json.contains("display")) {
        return std::nullopt;
    }
    auto const& displayJson = json.at("display");
    if (!displayJson.is_object()) {
        addTypeError(errors, "display", "an object");
        return std::nullopt;
    }

    auto parsedTitle       = parseTextComponent(displayJson, errors, "title");
    auto parsedDescription = parseTextComponent(displayJson, errors, "description");
    if (!parsedTitle || !parsedDescription) {
        return std::nullopt;
    }

    Display display{*parsedTitle, *parsedDescription};
    display.icon           = parseIcon(displayJson, errors);
    display.frame          = parseStringField(displayJson, errors, "frame", false);
    display.showToast      = parseBoolField(displayJson, errors, "show_toast");
    display.announceToChat = parseBoolField(displayJson, errors, "announce_to_chat");
    display.hidden         = parseBoolField(displayJson, errors, "hidden");
    return display;
}

std::map<std::string, Criterion> parseCriteria(nlohmann::json const& json, std::vector<std::string>& errors) {
    std::map<std::string, Criterion> criteria;
    if (!requireObject(json, errors, "criteria")) {
        return criteria;
    }

    for (auto const& [name, value] : json.at("criteria").items()) {
        if (!value.is_object()) {
            errors.emplace_back(std::format("criteria.{} must be an object", name));
            continue;
        }
        if (!value.contains("trigger")) {
            errors.emplace_back(std::format("missing required field criteria.{}.trigger", name));
            continue;
        }
        if (!value.at("trigger").is_string()) {
            errors.emplace_back(std::format("criteria.{}.trigger must be a string", name));
            continue;
        }
        std::optional<nlohmann::json> conditions;
        if (value.contains("conditions")) {
            if (!value.at("conditions").is_object()) {
                errors.emplace_back(std::format("criteria.{}.conditions must be an object", name));
                continue;
            }
            conditions = value.at("conditions");
        }
        criteria.emplace(name, Criterion{value.at("trigger").get<std::string>(), std::move(conditions)});
    }
    if (criteria.empty()) {
        errors.emplace_back("criteria must contain at least one entry");
    }
    return criteria;
}

std::vector<std::vector<std::string>> parseRequirements(
    nlohmann::json const&                   json,
    std::map<std::string, Criterion> const& criteria,
    std::vector<std::string>&               errors
) {
    std::vector<std::vector<std::string>> requirements;
    if (!json.contains("requirements")) {
        return requirements;
    }
    if (!json.at("requirements").is_array()) {
        addTypeError(errors, "requirements", "an array");
        return requirements;
    }

    size_t groupIndex = 0;
    for (auto const& group : json.at("requirements")) {
        if (!group.is_array()) {
            errors.emplace_back(std::format("requirements[{}] must be an array", groupIndex));
            ++groupIndex;
            continue;
        }

        std::vector<std::string> parsedGroup;
        size_t                   entryIndex = 0;
        for (auto const& entry : group) {
            if (!entry.is_string()) {
                errors.emplace_back(std::format("requirements[{}][{}] must be a string", groupIndex, entryIndex));
                ++entryIndex;
                continue;
            }
            auto criterionName = entry.get<std::string>();
            if (!criteria.contains(criterionName)) {
                errors.emplace_back(std::format(
                    "requirements[{}][{}] references missing criterion {}",
                    groupIndex,
                    entryIndex,
                    criterionName
                ));
            }
            parsedGroup.emplace_back(std::move(criterionName));
            ++entryIndex;
        }
        requirements.emplace_back(std::move(parsedGroup));
        ++groupIndex;
    }
    return requirements;
}

std::vector<std::string> parseStringArray(
    nlohmann::json const&      json,
    std::vector<std::string>&  errors,
    std::string_view           field
) {
    std::vector<std::string> values;
    if (!json.contains(field)) {
        return values;
    }
    if (!json.at(field).is_array()) {
        addTypeError(errors, field, "an array");
        return values;
    }

    size_t index = 0;
    for (auto const& item : json.at(field)) {
        if (!item.is_string()) {
            errors.emplace_back(std::format("{}[{}] must be a string", field, index));
        } else {
            values.emplace_back(item.get<std::string>());
        }
        ++index;
    }
    return values;
}

std::optional<int> parseExperience(nlohmann::json const& json, std::vector<std::string>& errors) {
    if (!json.contains("experience")) {
        return std::nullopt;
    }
    if (!json.at("experience").is_number_integer()) {
        addTypeError(errors, "experience", "an integer");
        return std::nullopt;
    }
    auto const experience = json.at("experience").get<int>();
    if (experience < 0) {
        errors.emplace_back("experience must not be negative");
        return std::nullopt;
    }
    return experience;
}

std::optional<Rewards> parseRewards(nlohmann::json const& json, std::vector<std::string>& errors) {
    if (!json.contains("rewards")) {
        return std::nullopt;
    }
    auto const& rewardsJson = json.at("rewards");
    if (!rewardsJson.is_object()) {
        addTypeError(errors, "rewards", "an object");
        return std::nullopt;
    }
    return Rewards{
        parseStringArray(rewardsJson, errors, "recipes"),
        parseStringArray(rewardsJson, errors, "loot"),
        parseExperience(rewardsJson, errors)
    };
}

} // namespace

ParseResult parseAdvancement(nlohmann::json const& json, std::string id, std::filesystem::path sourcePath) {
    ParseResult result;
    if (!json.is_object()) {
        result.errors.emplace_back("root must be an object");
        return result;
    }

    auto formatVersion = parseStringField(json, result.errors, "format_version", true);
    auto parent        = parseStringField(json, result.errors, "parent", false);
    auto display       = parseDisplay(json, result.errors);
    auto criteria      = parseCriteria(json, result.errors);
    auto requirements  = parseRequirements(json, criteria, result.errors);
    auto rewards       = parseRewards(json, result.errors);

    if (!result.errors.empty() || !formatVersion) {
        return result;
    }

    result.advancement = AdvancementDefinition{
        std::move(id),
        std::move(sourcePath),
        std::move(*formatVersion),
        std::move(parent),
        std::move(display),
        std::move(criteria),
        std::move(requirements),
        std::move(rewards)
    };
    return result;
}

} // namespace advancements
