#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::player {

class PlayerSleptInBedEvent final : public ll::event::Event {
public:
    explicit PlayerSleptInBedEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }

private:
    Player& mPlayer;
};

bool playerSleptInBedEventSourceRegistered();
void registerPlayerSleptInBedEventSource();
void unregisterPlayerSleptInBedEventSource();

} // namespace advancements::event::player
