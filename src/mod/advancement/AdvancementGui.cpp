#include "mod/advancement/AdvancementGui.h"

#include "ll/api/form/SimpleForm.h"
#include "mod/MyMod.h"
#include "mod/advancement/Localization.h"

#include "mc/world/actor/player/Player.h"

#include <algorithm>
#include <array>
#include <format>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace my_mod::advancement {
namespace {

struct CategoryDefinition {
    std::string_view key;
    std::string_view titleKey;
    std::string_view prefix;
};

constexpr std::array<CategoryDefinition, 5> RootCategories{
    CategoryDefinition{"story", "advancements.story.root.title", "minecraft:story/"},
    CategoryDefinition{"adventure", "advancements.adventure.root.title", "minecraft:adventure/"},
    CategoryDefinition{"nether", "advancements.nether.root.title", "minecraft:nether/"},
    CategoryDefinition{"end", "advancements.end.root.title", "minecraft:end/"},
    CategoryDefinition{"husbandry", "advancements.husbandry.root.title", "minecraft:husbandry/"}
};

std::string titleFor(AdvancementDefinition const& advancement, Player const& player) {
    if (advancement.display) {
        return localizeText(advancement.display->title, player);
    }
    return advancement.id;
}

std::string descriptionFor(AdvancementDefinition const& advancement, Player const& player) {
    if (advancement.display) {
        return localizeText(advancement.display->description, player);
    }
    return localizeKey("advancements.gui.no_description", player);
}

bool hasPrefix(std::string_view value, std::string_view prefix) {
    return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
}

CategoryDefinition const* findCategory(std::string_view key) {
    auto const found = std::ranges::find_if(RootCategories, [&](auto const& category) { return category.key == key; });
    return found == RootCategories.end() ? nullptr : &*found;
}

std::vector<std::string> categoryAdvancementIds(LoadResult const& definitions, CategoryDefinition const& category) {
    std::vector<std::string> ids;
    for (auto const& [id, _] : definitions.advancements) {
        if (hasPrefix(id, category.prefix)) {
            ids.emplace_back(id);
        }
    }
    std::ranges::sort(ids);
    return ids;
}

bool isDone(PlayerProgress const& progress, std::string const& advancementId) {
    auto const found = progress.advancements.find(advancementId);
    return found != progress.advancements.end() && found->second.done;
}

int completedCount(PlayerProgress const& progress, std::vector<std::string> const& ids) {
    auto count = 0;
    for (auto const& id : ids) {
        if (isDone(progress, id)) {
            ++count;
        }
    }
    return count;
}

std::vector<std::vector<std::string>> effectiveRequirementGroups(AdvancementDefinition const& advancement) {
    if (!advancement.requirements.empty()) {
        return advancement.requirements;
    }

    std::vector<std::vector<std::string>> groups;
    groups.reserve(advancement.criteria.size());
    for (auto const& [criterion, _] : advancement.criteria) {
        groups.push_back({criterion});
    }
    return groups;
}

AdvancementProgress const* progressFor(PlayerProgress const& progress, std::string const& advancementId) {
    auto const found = progress.advancements.find(advancementId);
    return found == progress.advancements.end() ? nullptr : &found->second;
}

std::optional<std::string> satisfiedGroupTime(
    AdvancementProgress const*      advancementProgress,
    std::vector<std::string> const& group
) {
    if (advancementProgress == nullptr) {
        return std::nullopt;
    }

    std::optional<std::string> earliest;
    for (auto const& criterion : group) {
        auto const found = advancementProgress->criteria.find(criterion);
        if (found == advancementProgress->criteria.end()) {
            continue;
        }
        if (!earliest || found->second < *earliest) {
            earliest = found->second;
        }
    }
    return earliest;
}

int satisfiedGroupCount(AdvancementDefinition const& advancement, PlayerProgress const& progress) {
    auto const* advancementProgress = progressFor(progress, advancement.id);
    auto const  groups              = effectiveRequirementGroups(advancement);
    return static_cast<int>(std::ranges::count_if(groups, [&](auto const& group) {
        return satisfiedGroupTime(advancementProgress, group).has_value();
    }));
}

std::optional<std::string> completionTime(AdvancementDefinition const& advancement, PlayerProgress const& progress) {
    auto const* advancementProgress = progressFor(progress, advancement.id);
    if (advancementProgress == nullptr || !advancementProgress->done) {
        return std::nullopt;
    }

    auto const groups = effectiveRequirementGroups(advancement);
    if (groups.empty()) {
        return std::nullopt;
    }

    std::optional<std::string> latest;
    for (auto const& group : groups) {
        auto const groupTime = satisfiedGroupTime(advancementProgress, group);
        if (!groupTime) {
            return std::nullopt;
        }
        if (!latest || *groupTime > *latest) {
            latest = *groupTime;
        }
    }
    return latest;
}

std::vector<std::string> orderedCategoryAdvancementIds(
    LoadResult const&               definitions,
    CategoryDefinition const&       category
) {
    auto ids = categoryAdvancementIds(definitions, category);

    std::map<std::string, std::vector<std::string>> childrenByParent;
    for (auto const& id : ids) {
        auto const& advancement = definitions.advancements.at(id);
        if (advancement.parent && hasPrefix(*advancement.parent, category.prefix) && definitions.advancements.contains(*advancement.parent)) {
            childrenByParent[*advancement.parent].emplace_back(id);
        }
    }

    for (auto& [_, children] : childrenByParent) {
        std::ranges::sort(children);
    }

    std::vector<std::string> ordered;
    ordered.reserve(ids.size());

    auto appendWithChildren = [&](auto const& self, std::string const& id) -> void {
        ordered.emplace_back(id);
        auto const children = childrenByParent.find(id);
        if (children == childrenByParent.end()) {
            return;
        }
        for (auto const& child : children->second) {
            self(self, child);
        }
    };

    for (auto const& id : ids) {
        auto const& advancement = definitions.advancements.at(id);
        if (!advancement.parent || !hasPrefix(*advancement.parent, category.prefix) || !definitions.advancements.contains(*advancement.parent)) {
            appendWithChildren(appendWithChildren, id);
        }
    }

    return ordered.size() == ids.size() ? ordered : ids;
}

void showCategory(MyMod& mod, Player& player, std::string const& categoryKey);

void showDetail(MyMod& mod, Player& player, std::string const& categoryKey, std::string const& advancementId) {
    auto worldDataDir = mod.getSelf().getWorldDataDir();
    if (!worldDataDir) {
        player.sendMessage(localizeKey("advancements.gui.error.world_data_unavailable", player));
        return;
    }

    auto const& definitions = mod.getAdvancementLoadResult();
    auto const  found       = definitions.advancements.find(advancementId);
    if (found == definitions.advancements.end()) {
        player.sendMessage(localizeKey("advancements.gui.error.advancement_unloaded", player));
        showAdvancementsGui(mod, player);
        return;
    }

    auto progressResult = mod.getProgressService().getProgress(*worldDataDir, player.getUuid());
    if (!progressResult.ok()) {
        player.sendMessage(localizeKey("advancements.gui.error.progress_load_failed", player));
        return;
    }

    auto const& advancement = found->second;
    auto const  done        = isDone(progressResult.progress, advancement.id);
    auto        content     = std::format(
        "{}\n\n{}: {}",
        descriptionFor(advancement, player),
        localizeKey("advancements.gui.state", player),
        done ? localizeKey("advancements.gui.status.done", player) : localizeKey("advancements.gui.status.in_progress", player)
    );

    if (auto const completedAt = completionTime(advancement, progressResult.progress); done && completedAt) {
        content += std::format(
            "\n{}: {}",
            localizeKey("advancements.gui.completed_at", player),
            *completedAt
        );
    }

    ll::form::SimpleForm form(titleFor(advancement, player), content);
    form.appendButton(localizeKey("advancements.gui.back", player), [&mod, categoryKey](Player& callbackPlayer) {
        showCategory(mod, callbackPlayer, categoryKey);
    });
    form.sendTo(player);
}

void showCategory(MyMod& mod, Player& player, std::string const& categoryKey) {
    auto const* category = findCategory(categoryKey);
    if (category == nullptr) {
        showAdvancementsGui(mod, player);
        return;
    }

    auto worldDataDir = mod.getSelf().getWorldDataDir();
    if (!worldDataDir) {
        player.sendMessage(localizeKey("advancements.gui.error.world_data_unavailable", player));
        return;
    }

    auto progressResult = mod.getProgressService().getProgress(*worldDataDir, player.getUuid());
    if (!progressResult.ok()) {
        player.sendMessage(localizeKey("advancements.gui.error.progress_load_failed", player));
        return;
    }

    auto const& definitions = mod.getAdvancementLoadResult();
    auto const  ids         = orderedCategoryAdvancementIds(definitions, *category);
    auto const  title       = localizeKey(category->titleKey, player);

    ll::form::SimpleForm form(
        title,
        ids.empty() ? localizeKey("advancements.gui.empty_category", player) : ""
    );

    for (auto const& id : ids) {
        auto const& advancement = definitions.advancements.at(id);
        auto        buttonText  = titleFor(advancement, player);
        auto const  groups      = effectiveRequirementGroups(advancement);
        if (groups.size() > 1) {
            buttonText += std::format(" ({}/{})", satisfiedGroupCount(advancement, progressResult.progress), groups.size());
        }
        form.appendButton(buttonText, [&mod, categoryKey, id](Player& callbackPlayer) {
            showDetail(mod, callbackPlayer, categoryKey, id);
        });
    }

    form.appendButton(localizeKey("advancements.gui.back", player), [&mod](Player& callbackPlayer) { showAdvancementsGui(mod, callbackPlayer); });
    form.sendTo(player);
}

} // namespace

void showAdvancementsGui(MyMod& mod, Player& player) {
    auto worldDataDir = mod.getSelf().getWorldDataDir();
    if (!worldDataDir) {
        player.sendMessage(localizeKey("advancements.gui.error.world_data_unavailable", player));
        return;
    }

    auto progressResult = mod.getProgressService().getProgress(*worldDataDir, player.getUuid());
    if (!progressResult.ok()) {
        player.sendMessage(localizeKey("advancements.gui.error.progress_load_failed", player));
        return;
    }

    auto const& definitions = mod.getAdvancementLoadResult();
    ll::form::SimpleForm form(localizeKey("advancements.gui.title", player), "");

    for (auto const& category : RootCategories) {
        auto const ids       = categoryAdvancementIds(definitions, category);
        auto const completed = completedCount(progressResult.progress, ids);
        auto const total     = ids.size();
        auto const key       = std::string{category.key};
        form.appendButton(
            std::format("{} ({}/{})", localizeKey(category.titleKey, player), completed, total),
            [&mod, key](Player& callbackPlayer) { showCategory(mod, callbackPlayer, key); }
        );
    }

    form.sendTo(player);
}

} // namespace my_mod::advancement
