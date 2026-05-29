#pragma once

#include "ll/api/event/Event.h"

#include <string>

class Player;

namespace advancements::event::entity {

class PlayerTamedAnimalEvent final : public ll::event::Event {
public:
    PlayerTamedAnimalEvent(Player& player, std::string entityTypeId, std::string entityVariantId)
    : mPlayer(player),
      mEntityTypeId(std::move(entityTypeId)),
      mEntityVariantId(std::move(entityVariantId)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& entityTypeId() const { return mEntityTypeId; }
    [[nodiscard]] std::string const& entityVariantId() const { return mEntityVariantId; }

private:
    Player&     mPlayer;
    std::string mEntityTypeId;
    std::string mEntityVariantId;
};

bool playerTamedAnimalEventSourceRegistered();
void registerPlayerTamedAnimalEventSource();
void unregisterPlayerTamedAnimalEventSource();

} // namespace advancements::event::entity
