#include "mod/gui/AdvancementGuiIndex.h"

#include <algorithm>
#include <array>
#include <map>
#include <string_view>

namespace advancements {
namespace {

struct CategoryDefinition {
    std::string_view key;
    std::string_view titleKey;
    std::string_view prefix;
};

constexpr std::array<CategoryDefinition, 5> RootCategories{
    CategoryDefinition{"story", "advancements.story.root.title", "minecraft:story/"},
    CategoryDefinition{"nether", "advancements.nether.root.title", "minecraft:nether/"},
    CategoryDefinition{"end", "advancements.end.root.title", "minecraft:end/"},
    CategoryDefinition{"adventure", "advancements.adventure.root.title", "minecraft:adventure/"},
    CategoryDefinition{"husbandry", "advancements.husbandry.root.title", "minecraft:husbandry/"}
};

bool hasPrefix(std::string_view value, std::string_view prefix) {
    return value.size() >= prefix.size() && value.substr(0, prefix.size()) == prefix;
}

std::vector<std::string> collectCategoryAdvancementIds(LoadResult const& definitions, CategoryDefinition const& category) {
    std::vector<std::string> ids;
    for (auto const& [id, _] : definitions.advancements) {
        if (hasPrefix(id, category.prefix)) {
            ids.emplace_back(id);
        }
    }
    std::ranges::sort(ids);
    return ids;
}

std::vector<std::string> orderCategoryAdvancementIds(
    LoadResult const&         definitions,
    CategoryDefinition const& category,
    std::vector<std::string> const& ids
) {
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

} // namespace

GuiCategorySnapshot const* AdvancementGuiIndex::findCategory(std::string_view key) const {
    auto const found = std::ranges::find_if(rootCategories, [&](GuiCategorySnapshot const& category) { return category.key == key; });
    return found == rootCategories.end() ? nullptr : &*found;
}

AdvancementGuiIndex buildAdvancementGuiIndex(LoadResult const& definitions) {
    AdvancementGuiIndex index;
    index.rootCategories.reserve(RootCategories.size());

    for (auto const& category : RootCategories) {
        auto advancementIds = collectCategoryAdvancementIds(definitions, category);
        index.rootCategories.push_back(GuiCategorySnapshot{
            .key                   = std::string{category.key},
            .titleKey              = std::string{category.titleKey},
            .advancementIds        = advancementIds,
            .orderedAdvancementIds = orderCategoryAdvancementIds(definitions, category, advancementIds),
        });
    }

    return index;
}

} // namespace advancements
