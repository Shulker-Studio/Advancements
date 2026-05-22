#include "mod/MyMod.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/mod/RegisterHelper.h"

#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"

namespace my_mod {

MyMod& MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

bool MyMod::load() {
    getSelf().getLogger().debug("Loading...");
    registerCommands();
    return true;
}

bool MyMod::enable() {
    getSelf().getLogger().debug("Enabling...");
    reloadAdvancements();
    return true;
}

bool MyMod::disable() {
    getSelf().getLogger().debug("Disabling...");
    return true;
}

void MyMod::registerCommands() {
    auto& command = ll::command::CommandRegistrar::getServerInstance().getOrCreateCommand(
        "advancements",
        "Advancements management",
        CommandPermissionLevel::GameDirectors
    );
    command.overload().execute([](CommandOrigin const&, CommandOutput& output) {
        auto const& result = MyMod::getInstance().getAdvancementLoadResult();
        output.success("Loaded {} advancement definitions with {} errors.", result.loadedCount(), result.errorCount());
    });
    command.overload().text("reload").execute([](CommandOrigin const&, CommandOutput& output) {
        auto& mod = MyMod::getInstance();
        mod.reloadAdvancements();
        auto const& result = mod.getAdvancementLoadResult();

        if (result.errorCount() == 0) {
            output.success("Reloaded {} advancement definitions with no errors.", result.loadedCount());
            return;
        }

        output.error("Reloaded {} advancement definitions with {} errors.", result.loadedCount(), result.errorCount());
    });
}

void MyMod::reloadAdvancements() {
    mAdvancementLoadResult = advancement::loadAdvancements(getSelf().getModDir());
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
