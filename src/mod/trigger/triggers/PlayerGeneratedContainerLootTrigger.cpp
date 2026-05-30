#include "mod/trigger/triggers/PlayerGeneratedContainerLootTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/PlayerGeneratedContainerLootEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gPlayerGeneratedContainerLootListener;

} // namespace

bool playerGeneratedContainerLootTriggerRegistered() { return gPlayerGeneratedContainerLootListener != nullptr; }

void registerPlayerGeneratedContainerLootTrigger(Entry& mod) {
    if (playerGeneratedContainerLootTriggerRegistered()) {
        return;
    }

    gPlayerGeneratedContainerLootListener =
        ll::event::EventBus::getInstance().emplaceListener<event::item::PlayerGeneratedContainerLootEvent>([&mod](auto& event) {
            dispatchTrigger(
                mod,
                TriggerContext{
                    event.player(),
                    "minecraft:player_generates_container_loot",
                    LootTablePayload{event.lootTableId(), event.generatedItemIds()},
                }
            );
        });
}

void unregisterPlayerGeneratedContainerLootTrigger() {
    if (gPlayerGeneratedContainerLootListener) {
        ll::event::EventBus::getInstance().removeListener(gPlayerGeneratedContainerLootListener);
        gPlayerGeneratedContainerLootListener.reset();
    }
}

} // namespace advancements
