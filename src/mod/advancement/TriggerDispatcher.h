#pragma once

#include "mod/advancement/AdvancementLoader.h"
#include "mod/advancement/ProgressService.h"
#include "mod/advancement/TriggerIndex.h"

#include <filesystem>
#include <optional>
#include <string>

class Player;

namespace my_mod::advancement {

struct TriggerContext {
    Player&                    player;
    std::string                triggerId;
    std::optional<std::string> blockId;
    std::optional<std::string> itemId;
    std::optional<int>         itemCount;
    std::optional<std::string> entityTypeId;
    std::optional<std::string> fromDimension;
    std::optional<std::string> toDimension;
};

class TriggerDispatcher {
public:
    TriggerDispatcher(TriggerIndex const& index, ProgressService const& progressService);

    void dispatch(
        std::filesystem::path const& worldDataDir,
        LoadResult const&            definitions,
        TriggerContext const&        context
    ) const;

private:
    [[nodiscard]] bool matches(CriterionBinding const& binding, TriggerContext const& context) const;

    TriggerIndex const&     mIndex;
    ProgressService const&  mProgressService;
};

} // namespace my_mod::advancement
