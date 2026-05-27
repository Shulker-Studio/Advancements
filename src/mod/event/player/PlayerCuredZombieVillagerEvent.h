#pragma once

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::player {

class PlayerCuredZombieVillagerEvent final : public ll::event::Event {
public:
    explicit PlayerCuredZombieVillagerEvent(Player& player) : mPlayer(player) {}

    [[nodiscard]] Player& player() const { return mPlayer; }

private:
    Player& mPlayer;
};

bool playerCuredZombieVillagerEventSourceRegistered();
void registerPlayerCuredZombieVillagerEventSource();
void unregisterPlayerCuredZombieVillagerEventSource();

} // namespace advancements::event::player
