#pragma once

#include "ll/api/event/Event.h"

class Actor;
class Player;

namespace advancements::event::block {

class SculkCatalystMobKilledEvent final : public ll::event::Event {
public:
    SculkCatalystMobKilledEvent(Player& player, Actor& mob) : mPlayer(player), mMob(mob) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] Actor& mob() const { return mMob; }

private:
    Player& mPlayer;
    Actor&  mMob;
};

bool sculkCatalystMobKilledEventSourceRegistered();
void registerSculkCatalystMobKilledEventSource();
void unregisterSculkCatalystMobKilledEventSource();

} // namespace advancements::event::block
