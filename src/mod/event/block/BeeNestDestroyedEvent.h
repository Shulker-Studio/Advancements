#pragma once

#include "ll/api/event/Event.h"

#include <string>
#include <utility>

class Player;

namespace advancements::event::block {

class BeeNestDestroyedEvent final : public ll::event::Event {
public:
    BeeNestDestroyedEvent(Player& player, std::string blockId, int numBeesInside)
    : mPlayer(player),
      mBlockId(std::move(blockId)),
      mNumBeesInside(numBeesInside) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& blockId() const { return mBlockId; }
    [[nodiscard]] int numBeesInside() const { return mNumBeesInside; }

private:
    Player&     mPlayer;
    std::string mBlockId;
    int         mNumBeesInside;
};

bool beeNestDestroyedEventSourceRegistered();
void registerBeeNestDestroyedEventSource();
void unregisterBeeNestDestroyedEventSource();

} // namespace advancements::event::block
