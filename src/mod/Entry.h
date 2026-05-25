#pragma once

#include "ll/api/mod/NativeMod.h"
#include "mod/advancement/AdvancementLoader.h"
#include "mod/advancement/ProgressService.h"
#include "mod/trigger/TriggerIndex.h"
#include "mod/commands/AdvancementCommandIndex.h"
#include "mod/gui/AdvancementGuiIndex.h"

namespace advancements {

class Entry {

public:
    static Entry& getInstance();

    Entry() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    [[nodiscard]] bool load();
    [[nodiscard]] bool enable();
    [[nodiscard]] bool disable();

    [[nodiscard]] ProgressService&           getProgressService() { return mProgressService; }
    [[nodiscard]] LoadResult const&          getAdvancementLoadResult() const { return mAdvancementLoadResult; }
    [[nodiscard]] AdvancementGuiIndex const& getAdvancementGuiIndex() const { return mAdvancementGuiIndex; }
    [[nodiscard]] commands::AdvancementCommandIndex const& getAdvancementCommandIndex() const {
        return mAdvancementCommandIndex;
    }
    [[nodiscard]] ProgressService const& getProgressService() const { return mProgressService; }
    [[nodiscard]] TriggerIndex const&    getTriggerIndex() const { return mTriggerIndex; }

    void reloadAdvancements();

private:
    ll::mod::NativeMod&               mSelf;
    LoadResult                        mAdvancementLoadResult;
    AdvancementGuiIndex               mAdvancementGuiIndex;
    commands::AdvancementCommandIndex mAdvancementCommandIndex;
    ProgressService                   mProgressService;
    TriggerIndex                      mTriggerIndex;
};

} // namespace advancements
