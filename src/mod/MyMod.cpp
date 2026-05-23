#include "mod/MyMod.h"

#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/RegisterHelper.h"
#include "mod/advancement/RuntimeTriggerAdapters.h"
#include "mod/commands/AdvancementsCommand.h"

namespace my_mod {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    getSelf().getLogger().debug("Loading...");
    if (auto result = ll::i18n::getInstance().load(getSelf().getLangDir()); !result) {
        getSelf().getLogger().error("Failed to load advancement translations.");
        result.error().log(getSelf().getLogger());
    }
    commands::registerAdvancementsCommand(*this);
    return true;
}

bool MyMod::enable() {
    getSelf().getLogger().debug("Enabling...");
    reloadAdvancements();
    advancement::registerRuntimeTriggerAdapters(*this);
    return true;
}

bool MyMod::disable() const {
    getSelf().getLogger().debug("Disabling...");
    if (auto worldDataDir = getSelf().getWorldDataDir(); worldDataDir) {
        auto const flushErrors = mProgressService.flushAll(*worldDataDir);
        for (auto const& error : flushErrors) {
            getSelf().getLogger().error("{}", error);
        }
    }
    advancement::unregisterRuntimeTriggerAdapters();
    commands::unregisterAdvancementsCommand();
    return true;
}

void MyMod::reloadAdvancements() {
    auto loadResult = advancement::loadAdvancements(getSelf().getModDir());
    auto triggerIndex = advancement::TriggerIndex{};
    triggerIndex.rebuild(loadResult);

    mAdvancementLoadResult = std::move(loadResult);
    mTriggerIndex = std::move(triggerIndex);
    commands::updateAdvancementCommandEnums(mAdvancementLoadResult);
    auto& logger = getSelf().getLogger();
    logger.info(
        "Loaded {} advancement definitions with {} errors, {} triggers, and {} trigger bindings.",
        mAdvancementLoadResult.loadedCount(),
        mAdvancementLoadResult.errorCount(),
        mTriggerIndex.triggerCount(),
        mTriggerIndex.bindingCount()
    );

    for (auto const& issue : mAdvancementLoadResult.issues) {
        logger.error("{}: {}", issue.path.string(), issue.message);
    }
}

} // namespace my_mod

LL_REGISTER_MOD(my_mod::MyMod, my_mod::MyMod::getInstance());
