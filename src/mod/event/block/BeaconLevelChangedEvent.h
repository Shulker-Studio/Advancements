#pragma once

#include "ll/api/event/Event.h"

class BlockSource;
class BlockPos;

namespace advancements::event::block {

class BeaconLevelChangedEvent final : public ll::event::Event {
public:
    BeaconLevelChangedEvent(BlockSource& region, BlockPos const& pos, int level)
    : mRegion(region),
      mPos(pos),
      mLevel(level) {}

    [[nodiscard]] BlockSource& region() const { return mRegion; }
    [[nodiscard]] BlockPos const& pos() const { return mPos; }
    [[nodiscard]] int level() const { return mLevel; }

private:
    BlockSource&     mRegion;
    BlockPos const&  mPos;
    int              mLevel;
};

bool beaconLevelChangedEventSourceRegistered();
void registerBeaconLevelChangedEventSource();
void unregisterBeaconLevelChangedEventSource();

} // namespace advancements::event::block
