#pragma once

#include "ll/api/event/Event.h"

#include <string>

class Player;

namespace advancements::event::item {

class EnchantedItemEvent final : public ll::event::Event {
public:
    EnchantedItemEvent(Player& player, std::string itemId, int levelCost, int requiredLevel)
    : mPlayer(player),
      mItemId(std::move(itemId)),
      mLevelCost(levelCost),
      mRequiredLevel(requiredLevel) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& itemId() const { return mItemId; }
    [[nodiscard]] int levelCost() const { return mLevelCost; }
    [[nodiscard]] int requiredLevel() const { return mRequiredLevel; }

private:
    Player&      mPlayer;
    std::string  mItemId;
    int          mLevelCost;
    int          mRequiredLevel;
};

bool enchantedItemEventSourceRegistered();
void registerEnchantedItemEventSource();
void unregisterEnchantedItemEventSource();

} // namespace advancements::event::item
