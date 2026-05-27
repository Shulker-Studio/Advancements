#pragma once

#include "ll/api/event/Event.h"

class ActorDamageSource;
class Player;

namespace advancements::event::player {

class PlayerBlockUsingShieldEvent final : public ll::event::Event {
public:
    PlayerBlockUsingShieldEvent(Player& player, ActorDamageSource const& source) : mPlayer(player), mSource(source) {}

    [[nodiscard]] Player& self() const { return mPlayer; }
    [[nodiscard]] ActorDamageSource const& source() const { return mSource; }

private:
    Player&                  mPlayer;
    ActorDamageSource const& mSource;
};

bool playerBlockUsingShieldEventSourceRegistered();
void registerPlayerBlockUsingShieldEventSource();
void unregisterPlayerBlockUsingShieldEventSource();

} // namespace advancements::event::player
