#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::player {

class PlayerUsedTotemEvent final : public ll::event::Event {
public:
    explicit PlayerUsedTotemEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }

private:
    Player& mPlayer;
};

bool playerUsedTotemEventSourceRegistered();
void registerPlayerUsedTotemEventSource();
void unregisterPlayerUsedTotemEventSource();

} // namespace advancements::event::player
