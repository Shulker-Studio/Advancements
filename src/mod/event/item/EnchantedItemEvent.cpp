#include "mod/event/item/EnchantedItemEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/containers/managers/models/EnchantingContainerManagerModel.h"
#include "mc/world/inventory/network/ItemStackNetResult.h"
#include "mc/world/inventory/network/crafting/CraftHandlerEnchant.h"
#include "mc/world/inventory/network/crafting/ItemStackRequestActionCraft.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/enchanting/EnchantUtils.h"
#include "mc/world/item/enchanting/ItemEnchantOption.h"

#include <memory>

namespace advancements::event::item {
namespace {
LL_TYPE_INSTANCE_HOOK(
    EnchantedItemEventHook,
    HookPriority::Normal,
    CraftHandlerEnchant,
    &CraftHandlerEnchant::_handleEnchant,
    ::ItemStackNetResult,
    ::ItemStackRequestActionCraft<::RecipeNetId, 12> const& requestAction
) {
    auto& model       = _getEnchantingModel();
    auto const copies = model.getItemCopies();
    auto const& inputItem = copies.empty() ? model.getSlot(14) : copies.front();
    auto const& options   = *model.mEnchantOptions;

    auto const result = origin(requestAction);
    if (result != ItemStackNetResult::Success) {
        return result;
    }

    auto const optionIndex = mEnchantCost > 0 ? mEnchantCost - 1 : 0;
    auto const* option = optionIndex < static_cast<int>(options.size()) ? &options[static_cast<std::size_t>(optionIndex)] : nullptr;

    ItemStack enchantedItem;
    if (!inputItem.isNull() && option != nullptr) {
        enchantedItem = ItemStack{inputItem};
        EnchantUtils::applyEnchant(enchantedItem, *option->mEnchants, false);
    }

    if (enchantedItem.isNull()) {
        return result;
    }

    EnchantedItemEvent event{mPlayer, enchantedItem.getTypeName(), mPlayer.isCreative() ? 0 : mEnchantCost, 0};
    ll::event::EventBus::getInstance().publish(event);
    return result;
}

std::unique_ptr<ll::event::EmitterBase> enchantedItemEventEmitterFactory();

class EnchantedItemEventEmitter : public ll::event::Emitter<enchantedItemEventEmitterFactory, EnchantedItemEvent> {
    ll::memory::HookRegistrar<EnchantedItemEventHook> enchantedItemHook;
};

std::unique_ptr<ll::event::EmitterBase> enchantedItemEventEmitterFactory() {
    return std::make_unique<EnchantedItemEventEmitter>();
}

bool gEnchantedItemEventSourceRegistered = false;

} // namespace

bool enchantedItemEventSourceRegistered() { return gEnchantedItemEventSourceRegistered; }

void registerEnchantedItemEventSource() {
    if (enchantedItemEventSourceRegistered()) {
        return;
    }

    (void)EnchantedItemEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<EnchantedItemEvent>(enchantedItemEventEmitterFactory);
    gEnchantedItemEventSourceRegistered = true;
}

void unregisterEnchantedItemEventSource() { gEnchantedItemEventSourceRegistered = false; }

} // namespace advancements::event::item
