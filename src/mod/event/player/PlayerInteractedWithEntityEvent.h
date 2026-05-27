#pragma once

#include "ll/api/event/Event.h"

#include <string>

class Player;

namespace advancements::event::player {

class PlayerInteractedWithEntityEvent final : public ll::event::Event {
public:
    PlayerInteractedWithEntityEvent(Player& player, std::string itemId, std::string entityTypeId, std::string entityVariantId)
    : mPlayer(player),
      mItemId(std::move(itemId)),
      mEntityTypeId(std::move(entityTypeId)),
      mEntityVariantId(std::move(entityVariantId)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& itemId() const { return mItemId; }
    [[nodiscard]] std::string const& entityTypeId() const { return mEntityTypeId; }
    [[nodiscard]] std::string const& entityVariantId() const { return mEntityVariantId; }

private:
    Player&     mPlayer;
    std::string mItemId;
    std::string mEntityTypeId;
    std::string mEntityVariantId;
};

bool playerInteractedWithEntityEventSourceRegistered();
void registerPlayerInteractedWithEntityEventSource();
void unregisterPlayerInteractedWithEntityEventSource();

} // namespace advancements::event::player
