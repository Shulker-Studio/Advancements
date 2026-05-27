#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::player {

class PlayerChargedRespawnAnchorEvent final : public ll::event::Event {
public:
    explicit PlayerChargedRespawnAnchorEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }

private:
    Player& mPlayer;
};

bool playerChargedRespawnAnchorEventSourceRegistered();
void registerPlayerChargedRespawnAnchorEventSource();
void unregisterPlayerChargedRespawnAnchorEventSource();

} // namespace advancements::event::player
