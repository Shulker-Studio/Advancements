#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::entity {

class PlayerAllayDeliveredItemEvent final : public ll::event::Event {
public:
    explicit PlayerAllayDeliveredItemEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }

private:
    Player& mPlayer;
};

bool playerAllayDeliveredItemEventSourceRegistered();
void registerPlayerAllayDeliveredItemEventSource();
void unregisterPlayerAllayDeliveredItemEventSource();

} // namespace advancements::event::entity
