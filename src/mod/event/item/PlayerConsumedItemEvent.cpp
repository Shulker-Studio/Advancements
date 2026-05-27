#include "mod/event/item/PlayerConsumedItemEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStackBase.h"

#include <memory>
#include <string>

namespace advancements::event::item {
namespace {

bool isConsumeItemUseMethod(ItemUseMethod useMethod) {
    return useMethod == ItemUseMethod::Eat || useMethod == ItemUseMethod::Consume;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerConsumedItemEventHook,
    HookPriority::Normal,
    Player,
    &Player::$useItem,
    void,
    ItemStackBase& item,
    ItemUseMethod  useMethod,
    bool           consumeItem
) {
    auto const itemId = item.isNull() ? std::string{} : item.getTypeName();

    origin(item, useMethod, consumeItem);

    if (!consumeItem || itemId.empty() || !isConsumeItemUseMethod(useMethod)) {
        return;
    }

    PlayerConsumedItemEvent event{*this, itemId};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerConsumedItemEventEmitterFactory();

class PlayerConsumedItemEventEmitter
    : public ll::event::Emitter<playerConsumedItemEventEmitterFactory, PlayerConsumedItemEvent> {
    ll::memory::HookRegistrar<PlayerConsumedItemEventHook> playerConsumedItemHook;
};

std::unique_ptr<ll::event::EmitterBase> playerConsumedItemEventEmitterFactory() {
    return std::make_unique<PlayerConsumedItemEventEmitter>();
}

bool gPlayerConsumedItemEventSourceRegistered = false;

} // namespace

bool playerConsumedItemEventSourceRegistered() { return gPlayerConsumedItemEventSourceRegistered; }

void registerPlayerConsumedItemEventSource() {
    if (playerConsumedItemEventSourceRegistered()) {
        return;
    }

    (void)PlayerConsumedItemEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerConsumedItemEvent>(playerConsumedItemEventEmitterFactory);
    gPlayerConsumedItemEventSourceRegistered = true;
}

void unregisterPlayerConsumedItemEventSource() { gPlayerConsumedItemEventSourceRegistered = false; }

} // namespace advancements::event::item
