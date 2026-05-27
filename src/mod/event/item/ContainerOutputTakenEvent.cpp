#include "mod/event/item/ContainerOutputTakenEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/inventory/network/ContainerScreenContext.h"
#include "mc/world/inventory/network/ItemStackNetManagerServer.h"
#include "mc/world/inventory/network/ItemStackNetResult.h"
#include "mc/world/inventory/network/ItemStackRequestActionHandler.h"
#include "mc/world/inventory/network/ItemStackRequestActionTransferBase.h"
#include "mc/world/inventory/network/ItemStackRequestSlotInfo.h"

#include <memory>

namespace advancements::event::item {
namespace {

LL_TYPE_INSTANCE_HOOK(
    ContainerOutputTakenEventHook,
    HookPriority::Normal,
    ItemStackRequestActionHandler,
    &ItemStackRequestActionHandler::_handleTransfer,
    ::ItemStackNetResult,
    ::ItemStackRequestActionTransferBase const& requestAction,
    bool                                        isSrcHintSlot,
    bool                                        isDstHintSlot,
    bool                                        isSwap
) {
    auto const& sourceSlot      = *requestAction.mSrc;
    auto const  sourceContainer = sourceSlot.mFullContainerName.mName;
    auto const  screenType      = mItemStackNetManager.getScreenContext().mScreenContainerType;

    auto const result = origin(requestAction, isSrcHintSlot, isDstHintSlot, isSwap);
    if (result != ItemStackNetResult::Success) {
        return result;
    }

    ContainerOutputTakenEvent event{mPlayer, sourceContainer, screenType};
    ll::event::EventBus::getInstance().publish(event);
    return result;
}

std::unique_ptr<ll::event::EmitterBase> containerOutputTakenEventEmitterFactory();

class ContainerOutputTakenEventEmitter
    : public ll::event::Emitter<containerOutputTakenEventEmitterFactory, ContainerOutputTakenEvent> {
    ll::memory::HookRegistrar<ContainerOutputTakenEventHook> containerOutputTakenHook;
};

std::unique_ptr<ll::event::EmitterBase> containerOutputTakenEventEmitterFactory() {
    return std::make_unique<ContainerOutputTakenEventEmitter>();
}

bool gContainerOutputTakenEventSourceRegistered = false;

} // namespace

bool containerOutputTakenEventSourceRegistered() { return gContainerOutputTakenEventSourceRegistered; }

void registerContainerOutputTakenEventSource() {
    if (containerOutputTakenEventSourceRegistered()) {
        return;
    }

    (void)ContainerOutputTakenEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<ContainerOutputTakenEvent>(containerOutputTakenEventEmitterFactory);
    gContainerOutputTakenEventSourceRegistered = true;
}

void unregisterContainerOutputTakenEventSource() { gContainerOutputTakenEventSourceRegistered = false; }

} // namespace advancements::event::item
