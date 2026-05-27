#pragma once

#include <string>

#include "ll/api/event/Event.h"

class Player;

namespace advancements::event::player {

class PlayerKilledByEntityEvent final : public ll::event::Event {
public:
    PlayerKilledByEntityEvent(Player& player, std::string killerEntityTypeId)
    : mPlayer(player),
      mKillerEntityTypeId(std::move(killerEntityTypeId)) {}

    [[nodiscard]] Player& self() const { return mPlayer; }
    [[nodiscard]] std::string const& killerEntityTypeId() const { return mKillerEntityTypeId; }

private:
    Player&     mPlayer;
    std::string mKillerEntityTypeId;
};

bool playerKilledByEntityEventSourceRegistered();
void registerPlayerKilledByEntityEventSource();
void unregisterPlayerKilledByEntityEventSource();

} // namespace advancements::event::player
