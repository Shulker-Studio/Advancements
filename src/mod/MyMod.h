#pragma once

#include "ll/api/mod/NativeMod.h"
#include "mod/advancement/AdvancementLoader.h"
#include "mod/advancement/ProgressService.h"
#include "mod/advancement/TriggerIndex.h"

namespace my_mod {

class MyMod {

public:
    static MyMod& getInstance();

    MyMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    [[nodiscard]]
    bool load();

    /// @return True if the mod is enabled successfully.
    [[nodiscard]]
    bool enable();

    /// @return True if the mod is disabled successfully.
    [[nodiscard]]
    bool disable() const;

    [[nodiscard]] advancement::LoadResult const& getAdvancementLoadResult() const { return mAdvancementLoadResult; }
    [[nodiscard]] advancement::ProgressService const& getProgressService() const { return mProgressService; }
    [[nodiscard]] advancement::TriggerIndex const& getTriggerIndex() const { return mTriggerIndex; }

    void reloadAdvancements();

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();

private:
    ll::mod::NativeMod&         mSelf;
    advancement::LoadResult     mAdvancementLoadResult;
    advancement::ProgressService mProgressService;
    advancement::TriggerIndex   mTriggerIndex;
};

} // namespace my_mod
