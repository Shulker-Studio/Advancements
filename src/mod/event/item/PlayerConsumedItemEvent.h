#pragma once

#include "ll/api/event/Event.h"

#include <string>
#include <utility>

class Player;

namespace advancements::event::item {

class PlayerConsumedItemEvent final : public ll::event::Event {
public:
    PlayerConsumedItemEvent(Player& player, std::string itemId) : mPlayer(player), mItemId(std::move(itemId)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& itemId() const { return mItemId; }

private:
    Player&     mPlayer;
    std::string mItemId;
};

bool playerConsumedItemEventSourceRegistered();
void registerPlayerConsumedItemEventSource();
void unregisterPlayerConsumedItemEventSource();

} // namespace advancements::event::item
