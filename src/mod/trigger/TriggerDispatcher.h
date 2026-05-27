#pragma once

#include "mod/advancement/AdvancementLoader.h"
#include "mod/advancement/ProgressService.h"
#include "mod/trigger/TriggerIndex.h"

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
    TriggerIndex const& mIndex;
    ProgressService&    mProgressService;
};

} // namespace advancements
