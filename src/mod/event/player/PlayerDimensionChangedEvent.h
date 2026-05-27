#pragma once

#include "ll/api/event/Event.h"

#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/dimension/Dimension.h"

class Player;

namespace advancements::event::player {

class PlayerDimensionChangedEvent final : public ll::event::Event {
public:
    PlayerDimensionChangedEvent(Player& player, DimensionType fromDimension, DimensionType toDimension, Vec3 const& positionBeforeChange)
    : mPlayer(player),
      mFromDimension(fromDimension),
      mToDimension(toDimension),
      mPositionBeforeChange(positionBeforeChange) {}

    [[nodiscard]] Player& self() const { return mPlayer; }
    [[nodiscard]] DimensionType fromDimension() const { return mFromDimension; }
    [[nodiscard]] DimensionType toDimension() const { return mToDimension; }
    [[nodiscard]] Vec3 const& positionBeforeChange() const { return mPositionBeforeChange; }

private:
    Player&       mPlayer;
    DimensionType mFromDimension;
    DimensionType mToDimension;
    Vec3          mPositionBeforeChange;
};

bool playerDimensionChangedEventSourceRegistered();
void registerPlayerDimensionChangedEventSource();
void unregisterPlayerDimensionChangedEventSource();

} // namespace advancements::event::player
