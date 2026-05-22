#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace my_mod::advancement {

enum class TextComponentKind {
    Text,
    Translate,
};

struct TextComponent {
    TextComponentKind kind;
    std::string       value;
};

struct Icon {
    std::string item;
};

struct Display {
    TextComponent       title;
    TextComponent       description;
    std::optional<Icon> icon;
    std::optional<std::string> frame;
    std::optional<bool>        showToast;
    std::optional<bool>        announceToChat;
    std::optional<bool>        hidden;
};

struct Criterion {
    std::string                   trigger;
    std::optional<nlohmann::json> conditions;
};

struct Rewards {
    std::vector<std::string> recipes;
    std::vector<std::string> loot;
    std::optional<int>       experience;
};

struct AdvancementDefinition {
    std::string                           id;
    std::filesystem::path                 sourcePath;
    std::string                           formatVersion;
    std::optional<std::string>            parent;
    std::optional<Display>                display;
    std::map<std::string, Criterion>      criteria;
    std::vector<std::vector<std::string>> requirements;
    std::optional<Rewards>                rewards;
};

} // namespace my_mod::advancement
