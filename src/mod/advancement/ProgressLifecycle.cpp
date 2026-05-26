#include "mod/advancement/ProgressLifecycle.h"

#include "mod/Entry.h"

#include "ll/api/coro/CoroTask.h"
#include "ll/api/coro/SleepAwaiter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/thread/ServerThreadExecutor.h"

#include <chrono>

namespace advancements {
namespace {

constexpr auto ProgressAutoFlushInterval = std::chrono::minutes(5);

ll::event::ListenerPtr gPlayerDisconnectListener;
bool                   gPeriodicProgressFlushRunning = false;

ll::coro::CoroTask<> periodicProgressFlushTask(Entry& mod) {
    while (gPeriodicProgressFlushRunning) {
        co_await ll::coro::SleepAwaiter{ProgressAutoFlushInterval};

        if (!gPeriodicProgressFlushRunning) {
            co_return;
        }

        auto worldDataDir = mod.getSelf().getWorldDataDir();
        if (!worldDataDir) {
            continue;
        }

        auto const flushErrors = mod.getProgressService().flushAll(*worldDataDir);
        for (auto const& error : flushErrors) {
            mod.getSelf().getLogger().error("{}", error);
        }
    }
}

} // namespace

bool progressLifecycleRegistered() { return gPlayerDisconnectListener || gPeriodicProgressFlushRunning; }

void registerProgressLifecycle(Entry& mod) {
    if (progressLifecycleRegistered()) {
        return;
    }

    auto& eventBus = ll::event::EventBus::getInstance();
    gPlayerDisconnectListener = eventBus.emplaceListener<ll::event::player::PlayerDisconnectEvent>([&mod](auto& event) {
        if (auto worldDataDir = mod.getSelf().getWorldDataDir(); worldDataDir) {
            auto const flushErrors = mod.getProgressService().flushPlayer(*worldDataDir, event.self().getUuid());
            for (auto const& error : flushErrors) {
                mod.getSelf().getLogger().error("{}", error);
            }
        }
        return true;
    });

    gPeriodicProgressFlushRunning = true;
    ll::coro::keepThis([&mod]() -> ll::coro::CoroTask<> { co_return co_await periodicProgressFlushTask(mod); })
        .launch(ll::thread::ServerThreadExecutor::getDefault());
}

void unregisterProgressLifecycle() {
    gPeriodicProgressFlushRunning = false;

    auto& eventBus = ll::event::EventBus::getInstance();
    if (gPlayerDisconnectListener) {
        eventBus.removeListener(gPlayerDisconnectListener);
        gPlayerDisconnectListener.reset();
    }
}

} // namespace advancements
