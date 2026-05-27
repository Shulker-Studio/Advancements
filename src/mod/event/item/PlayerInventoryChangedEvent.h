#pragma once

#include "ll/api/event/Event.h"

#include <optional>
#include <string>
#include <utility>

class Player;

namespace advancements::event::item {

class PlayerInventoryChangedEvent final : public ll::event::Event {
public:
    PlayerInventoryChangedEvent(
        Player&                    player,
        std::optional<std::string> oldItemId,
        std::optional<std::string> newItemId
    )
    : mPlayer(player), mOldItemId(std::move(oldItemId)), mNewItemId(std::move(newItemId)) {}

    [[nodiscard]] Player&                           player() const { return mPlayer; }
    [[nodiscard]] std::optional<std::string> const& oldItemId() const { return mOldItemId; }
    [[nodiscard]] std::optional<std::string> const& newItemId() const { return mNewItemId; }

private:
    Player&                    mPlayer;
    std::optional<std::string> mOldItemId;
    std::optional<std::string> mNewItemId;
};

bool playerInventoryChangedEventSourceRegistered();
void registerPlayerInventoryChangedEventSource();
void unregisterPlayerInventoryChangedEventSource();

} // namespace advancements::event::item
