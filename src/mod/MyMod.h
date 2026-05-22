#pragma once

#include "ll/api/mod/NativeMod.h"
#include "mod/advancement/AdvancementLoader.h"

namespace my_mod {

class MyMod {

public:
    static MyMod& getInstance();

    MyMod() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    [[nodiscard]] advancement::LoadResult const& getAdvancementLoadResult() const { return mAdvancementLoadResult; }

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();

private:
    void registerCommands();
    void reloadAdvancements();

    ll::mod::NativeMod& mSelf;
    advancement::LoadResult mAdvancementLoadResult;
};

} // namespace my_mod
