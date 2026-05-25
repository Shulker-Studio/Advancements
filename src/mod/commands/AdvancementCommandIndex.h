#pragma once

#include "mod/advancement/AdvancementLoader.h"

#include <string>
#include <vector>

namespace advancements::commands {

struct AdvancementCommandIndex {
    std::vector<std::string> advancementIds;
    std::vector<std::string> criterionNames;
};

[[nodiscard]] AdvancementCommandIndex buildAdvancementCommandIndex(LoadResult const& result);

} // namespace advancements::commands
