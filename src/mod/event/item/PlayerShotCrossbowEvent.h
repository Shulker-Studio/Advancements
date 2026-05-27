#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::item {

class PlayerShotCrossbowEvent final : public ll::event::Event {
public:
    explicit PlayerShotCrossbowEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }

private:
    Player& mPlayer;
};

bool playerShotCrossbowEventSourceRegistered();
void registerPlayerShotCrossbowEventSource();
void unregisterPlayerShotCrossbowEventSource();

} // namespace advancements::event::item
