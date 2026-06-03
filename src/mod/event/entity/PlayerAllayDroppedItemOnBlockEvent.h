#pragma once

#include "ll/api/event/Event.h"

#include <string>
#include <utility>

class Player;

namespace advancements::event::entity {

class PlayerAllayDroppedItemOnBlockEvent final : public ll::event::Event {
public:
    PlayerAllayDroppedItemOnBlockEvent(Player& player, std::string itemId, std::string blockId)
    : mPlayer(player), mItemId(std::move(itemId)), mBlockId(std::move(blockId)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& itemId() const { return mItemId; }
    [[nodiscard]] std::string const& blockId() const { return mBlockId; }

private:
    Player&     mPlayer;
    std::string mItemId;
    std::string mBlockId;
};

bool playerAllayDroppedItemOnBlockEventSourceRegistered();
void registerPlayerAllayDroppedItemOnBlockEventSource();
void unregisterPlayerAllayDroppedItemOnBlockEventSource();

} // namespace advancements::event::entity
