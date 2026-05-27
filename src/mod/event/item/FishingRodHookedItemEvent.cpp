#include "mod/event/item/FishingRodHookedItemEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/FishingHook.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"

#include <memory>
#include <optional>

namespace advancements::event::item {
namespace {

LL_TYPE_INSTANCE_HOOK(
    FishingRodHookedItemEventHook,
    HookPriority::Normal,
    FishingHook,
    &FishingHook::_pullCloser,
    void,
    Actor& inEntity,
    float  inSpeed
) {
    auto* player = getPlayerOwner();
    std::optional<std::string> itemId;
    if (inEntity.isType(ActorType::ItemEntity)) {
        auto const& item = static_cast<ItemActor&>(inEntity).item();
        if (!item.isNull()) {
            itemId = item.getTypeName();
        }
    }

    origin(inEntity, inSpeed);

    if (player == nullptr || !itemId) {
        return;
    }

    FishingRodHookedItemEvent event{*player, *itemId};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> fishingRodHookedItemEventEmitterFactory();

class FishingRodHookedItemEventEmitter
    : public ll::event::Emitter<fishingRodHookedItemEventEmitterFactory, FishingRodHookedItemEvent> {
    ll::memory::HookRegistrar<FishingRodHookedItemEventHook> fishingRodHookedHook;
};

std::unique_ptr<ll::event::EmitterBase> fishingRodHookedItemEventEmitterFactory() {
    return std::make_unique<FishingRodHookedItemEventEmitter>();
}

bool gFishingRodHookedItemEventSourceRegistered = false;

} // namespace

bool fishingRodHookedItemEventSourceRegistered() { return gFishingRodHookedItemEventSourceRegistered; }

void registerFishingRodHookedItemEventSource() {
    if (fishingRodHookedItemEventSourceRegistered()) {
        return;
    }

    (void)FishingRodHookedItemEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<FishingRodHookedItemEvent>(
        fishingRodHookedItemEventEmitterFactory
    );
    gFishingRodHookedItemEventSourceRegistered = true;
}

void unregisterFishingRodHookedItemEventSource() { gFishingRodHookedItemEventSourceRegistered = false; }

} // namespace advancements::event::item
