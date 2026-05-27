#pragma once

#include "ll/api/event/Event.h"

class Actor;
class BlockPos;
class BlockSource;

namespace advancements::event::block {

class TargetBlockHitEvent final : public ll::event::Event {
public:
    TargetBlockHitEvent(BlockSource& region, BlockPos const& pos, Actor const& projectile)
    : mRegion(region),
      mPos(pos),
      mProjectile(projectile) {}

    [[nodiscard]] BlockSource& region() const { return mRegion; }
    [[nodiscard]] BlockPos const& pos() const { return mPos; }
    [[nodiscard]] Actor const& projectile() const { return mProjectile; }

private:
    BlockSource&  mRegion;
    BlockPos const& mPos;
    Actor const&  mProjectile;
};

bool targetBlockHitEventSourceRegistered();
void registerTargetBlockHitEventSource();
void unregisterTargetBlockHitEventSource();

} // namespace advancements::event::block
