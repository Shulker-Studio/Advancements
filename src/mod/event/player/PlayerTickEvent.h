#pragma once

#include "ll/api/event/Event.h"

class Player;
class Tick;

namespace advancements::event::player {

class PlayerTickEvent final : public ll::event::Event {
public:
    explicit PlayerTickEvent(Player& player, Tick const& currentTick) : mPlayer(player), mCurrentTick(currentTick) {}

    [[nodiscard]] Player& self() const { return mPlayer; }
    [[nodiscard]] Tick const& currentTick() const { return mCurrentTick; }

private:
    Player&     mPlayer;
    Tick const& mCurrentTick;
};

bool playerTickEventSourceRegistered();
void registerPlayerTickEventSource();
void unregisterPlayerTickEventSource();

} // namespace advancements::event::player
