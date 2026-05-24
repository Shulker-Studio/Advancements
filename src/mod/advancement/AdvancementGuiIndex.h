#pragma once

#include "mod/advancement/AdvancementLoader.h"

#include <string>
#include <string_view>
#include <vector>

namespace my_mod::advancement {

struct GuiCategorySnapshot {
    std::string              key;
    std::string              titleKey;
    std::vector<std::string> advancementIds;
    std::vector<std::string> orderedAdvancementIds;
};

struct AdvancementGuiIndex {
    std::vector<GuiCategorySnapshot> rootCategories;

    [[nodiscard]] GuiCategorySnapshot const* findCategory(std::string_view key) const;
};

[[nodiscard]] AdvancementGuiIndex buildAdvancementGuiIndex(LoadResult const& definitions);

} // namespace my_mod::advancement
