#include "mod/Entry.h"

#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/RegisterHelper.h"
#include "mod/trigger/RuntimeTriggerAdapters.h"
#include "mod/commands/AdvancementsCommand.h"

namespace advancements {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

bool Entry::load() {
    getSelf().getLogger().debug("Loading...");
    if (auto result = ll::i18n::getInstance().load(getSelf().getLangDir()); !result) {
        getSelf().getLogger().error("Failed to load advancement translations.");
        result.error().log(getSelf().getLogger());
    }
    commands::registerAdvancementsCommand(*this);
    return true;
}

bool Entry::enable() {
    getSelf().getLogger().debug("Enabling...");
    reloadAdvancements();
    registerRuntimeTriggerAdapters(*this);
    return true;
}

bool Entry::disable() {
    getSelf().getLogger().debug("Disabling...");
    if (auto worldDataDir = getSelf().getWorldDataDir(); worldDataDir) {
        auto const flushErrors = mProgressService.flushAll(*worldDataDir);
        for (auto const& error : flushErrors) {
            getSelf().getLogger().error("{}", error);
        }
    }
    unregisterRuntimeTriggerAdapters();
    commands::unregisterAdvancementsCommand();
    return true;
}

void Entry::reloadAdvancements() {
    auto loadResult   = loadAdvancements(getSelf().getModDir());
    auto guiIndex     = buildAdvancementGuiIndex(loadResult);
    auto commandIndex = commands::buildAdvancementCommandIndex(loadResult);
    auto triggerIndex = TriggerIndex{};
    triggerIndex.rebuild(loadResult);

    mAdvancementLoadResult   = std::move(loadResult);
    mAdvancementGuiIndex     = std::move(guiIndex);
    mAdvancementCommandIndex = std::move(commandIndex);
    mTriggerIndex            = std::move(triggerIndex);
    commands::updateAdvancementCommandEnums(mAdvancementCommandIndex);
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

} // namespace advancements

LL_REGISTER_MOD(advancements::Entry, advancements::Entry::getInstance());
