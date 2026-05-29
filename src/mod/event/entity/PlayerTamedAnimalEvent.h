#pragma once

#include "ll/api/event/Event.h"

#include <string>

class Player;

namespace advancements::event::entity {

class PlayerTamedAnimalEvent final : public ll::event::Event {
public:
    PlayerTamedAnimalEvent(Player& player, std::string entityTypeId)
    : mPlayer(player),
      mEntityTypeId(std::move(entityTypeId)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& entityTypeId() const { return mEntityTypeId; }

private:
    Player&     mPlayer;
    std::string mEntityTypeId;
};

bool playerTamedAnimalEventSourceRegistered();
void registerPlayerTamedAnimalEventSource();
void unregisterPlayerTamedAnimalEventSource();

} // namespace advancements::event::entity
