#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::player {

class PlayerEffectsChangedEvent final : public ll::event::Event {
public:
    explicit PlayerEffectsChangedEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
private:
    Player& mPlayer;
};

bool playerEffectsChangedEventSourceRegistered();
void registerPlayerEffectsChangedEventSource();
void unregisterPlayerEffectsChangedEventSource();

} // namespace advancements::event::player
