#pragma once

#include "ll/api/event/Event.h"

#include <string>
#include <utility>

class Player;

namespace advancements::event::item {

class PlayerPiglinAdmireItemEvent final : public ll::event::Event {
public:
    PlayerPiglinAdmireItemEvent(Player& player, std::string itemId, bool wasTargetingBarteringPlayer)
    : mPlayer(player),
      mItemId(std::move(itemId)),
      mWasTargetingBarteringPlayer(wasTargetingBarteringPlayer) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& itemId() const { return mItemId; }
    [[nodiscard]] bool wasTargetingBarteringPlayer() const { return mWasTargetingBarteringPlayer; }

private:
    Player&     mPlayer;
    std::string mItemId;
    bool        mWasTargetingBarteringPlayer;
};

bool playerPiglinAdmireItemEventSourceRegistered();
void registerPlayerPiglinAdmireItemEventSource();
void unregisterPlayerPiglinAdmireItemEventSource();

} // namespace advancements::event::item
