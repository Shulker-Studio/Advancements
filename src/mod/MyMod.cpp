#include "mod/MyMod.h"

#include "ll/api/mod/RegisterHelper.h"
#include "mod/commands/AdvancementsCommand.h"

namespace my_mod {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    getSelf().getLogger().debug("Loading...");
    commands::registerAdvancementsCommand(*this);
    return true;
}

bool MyMod::enable() {
    getSelf().getLogger().debug("Enabling...");
    reloadAdvancements();
    return true;
}

bool MyMod::disable() const {
    getSelf().getLogger().debug("Disabling...");
    return true;
}

void MyMod::reloadAdvancements() {
    mAdvancementLoadResult = advancement::loadAdvancements(getSelf().getModDir());
    commands::updateAdvancementCommandEnums(mAdvancementLoadResult);
    auto& logger = getSelf().getLogger();
    logger.info(
        "Loaded {} advancement definitions with {} errors.",
        mAdvancementLoadResult.loadedCount(),
        mAdvancementLoadResult.errorCount()
    );

    for (auto const& issue : mAdvancementLoadResult.issues) {
        logger.error("{}: {}", issue.path.string(), issue.message);
    }
}

} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::MyMod::getInstance());
