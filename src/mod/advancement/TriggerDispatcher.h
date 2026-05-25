#pragma once

#include "mod/advancement/AdvancementLoader.h"
#include "mod/advancement/ProgressService.h"
#include "mod/advancement/TriggerIndex.h"

#include <filesystem>

namespace advancements {

class TriggerDispatcher {
public:
    TriggerDispatcher(TriggerIndex const& index, ProgressService& progressService);

    void dispatch(
        std::filesystem::path const& worldDataDir,
        LoadResult const&            definitions,
        TriggerContext const&        context
    ) const;

private:
    [[nodiscard]] bool legacyMatches(CriterionBinding const& binding, TriggerContext const& context) const;

    TriggerIndex const& mIndex;
    ProgressService&    mProgressService;
};

} // namespace advancements
