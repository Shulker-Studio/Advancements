#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::player {

class PlayerEnteredEndGatewayEvent final : public ll::event::Event {
public:
    explicit PlayerEnteredEndGatewayEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }

private:
    Player& mPlayer;
};

bool playerEnteredEndGatewayEventSourceRegistered();
void registerPlayerEnteredEndGatewayEventSource();
void unregisterPlayerEnteredEndGatewayEventSource();

} // namespace advancements::event::player
