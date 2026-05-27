#pragma once

#include "ll/api/event/Event.h"

class BlockSource;
class Vec3;

namespace advancements::event::block {

class WitherSummonedEvent final : public ll::event::Event {
public:
    WitherSummonedEvent(BlockSource& region, Vec3 const& pos)
    : mRegion(region),
      mPos(pos) {}

    [[nodiscard]] BlockSource& region() const { return mRegion; }
    [[nodiscard]] Vec3 const& pos() const { return mPos; }

private:
    BlockSource& mRegion;
    Vec3 const&  mPos;
};

bool witherSummonedEventSourceRegistered();
void registerWitherSummonedEventSource();
void unregisterWitherSummonedEventSource();

} // namespace advancements::event::block
