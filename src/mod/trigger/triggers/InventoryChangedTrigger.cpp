#include "mod/trigger/triggers/InventoryChangedTrigger.h"

#include "mod/Entry.h"
#include "mod/event/item/PlayerInventoryChangedEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include "mc/world/actor/player/Inventory.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"

namespace advancements {
namespace {

ll::event::ListenerPtr gInventoryChangedListener;

int countMatchingItems(Player const& player, std::string const& itemId) {
    auto const& inventory = player.getInventory();
    int         total     = 0;
    for (int slot = 0; slot < inventory.getContainerSize(); ++slot) {
        auto const& item = inventory.getItem(slot);
        if (item.isNull() || item.getTypeName() != itemId) {
            continue;
        }
        total += item.mCount;
    }
    return total;
}

void dispatchInventoryChanged(Entry& mod, Player& player, std::string const& itemId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:inventory_changed",
            ItemTriggerPayload{itemId, countMatchingItems(player, itemId)},
        }
    );
}

} // namespace

bool inventoryChangedTriggerRegistered() { return gInventoryChangedListener != nullptr; }

void registerInventoryChangedTrigger(Entry& mod) {
    if (inventoryChangedTriggerRegistered()) {
        return;
    }

    gInventoryChangedListener = ll::event::EventBus::getInstance().emplaceListener<event::item::PlayerInventoryChangedEvent>(
        [&mod](auto& event) {
            if (event.newItemId()) {
                dispatchInventoryChanged(mod, event.player(), *event.newItemId());
            }

            if (event.oldItemId() && event.oldItemId() != event.newItemId()) {
                dispatchInventoryChanged(mod, event.player(), *event.oldItemId());
            }
        }
    );
}

void unregisterInventoryChangedTrigger() {
    if (gInventoryChangedListener) {
        ll::event::EventBus::getInstance().removeListener(gInventoryChangedListener);
        gInventoryChangedListener.reset();
    }
}

} // namespace advancements
