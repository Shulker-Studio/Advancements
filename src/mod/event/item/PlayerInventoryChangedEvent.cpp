#include "mod/event/item/PlayerInventoryChangedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/Container.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"

#include <memory>
#include <optional>
#include <string>

namespace advancements::event::item {
namespace {

LL_TYPE_INSTANCE_HOOK(
    PlayerInventoryChangedEventHook,
    HookPriority::Normal,
    Player,
    &Player::inventoryChanged,
    void,
    Container&       container,
    int              slot,
    ItemStack const& oldItem,
    ItemStack const& newItem,
    bool             forceBalanced
) {
    std::optional<std::string> const newItemId =
        !newItem.isNull() ? std::optional<std::string>{newItem.getTypeName()} : std::nullopt;
    std::optional<std::string> const oldItemId = !oldItem.isNull() ? std::optional<std::string>{oldItem.getTypeName()} : std::nullopt;

    origin(container, slot, oldItem, newItem, forceBalanced);

    PlayerInventoryChangedEvent event{*this, oldItemId, newItemId};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerInventoryChangedEventEmitterFactory();

class PlayerInventoryChangedEventEmitter
    : public ll::event::Emitter<playerInventoryChangedEventEmitterFactory, PlayerInventoryChangedEvent> {
    ll::memory::HookRegistrar<PlayerInventoryChangedEventHook> playerInventoryChangedHook;
};

std::unique_ptr<ll::event::EmitterBase> playerInventoryChangedEventEmitterFactory() {
    return std::make_unique<PlayerInventoryChangedEventEmitter>();
}

bool gPlayerInventoryChangedEventSourceRegistered = false;

} // namespace

bool playerInventoryChangedEventSourceRegistered() { return gPlayerInventoryChangedEventSourceRegistered; }

void registerPlayerInventoryChangedEventSource() {
    if (playerInventoryChangedEventSourceRegistered()) {
        return;
    }

    (void)PlayerInventoryChangedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerInventoryChangedEvent>(playerInventoryChangedEventEmitterFactory);
    gPlayerInventoryChangedEventSourceRegistered = true;
}

void unregisterPlayerInventoryChangedEventSource() { gPlayerInventoryChangedEventSourceRegistered = false; }

} // namespace advancements::event::item
