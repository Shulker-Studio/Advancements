#pragma once

#include "ll/api/event/Event.h"

#include <string>

class Player;

namespace advancements::event::entity {

class PlayerProjectileLightningHitEvent final : public ll::event::Event {
public:
    PlayerProjectileLightningHitEvent(
        Player&     player,
        std::string projectileTypeId,
        bool        projectileIsChanneling,
        std::string struckEntityTypeId,
        bool        dimensionIsLightning,
        bool        hitPositionCanSeeSky
    )
    : mPlayer(player),
      mProjectileTypeId(std::move(projectileTypeId)),
      mProjectileIsChanneling(projectileIsChanneling),
      mStruckEntityTypeId(std::move(struckEntityTypeId)),
      mDimensionIsLightning(dimensionIsLightning),
      mHitPositionCanSeeSky(hitPositionCanSeeSky) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& projectileTypeId() const { return mProjectileTypeId; }
    [[nodiscard]] bool projectileIsChanneling() const { return mProjectileIsChanneling; }
    [[nodiscard]] std::string const& struckEntityTypeId() const { return mStruckEntityTypeId; }
    [[nodiscard]] bool dimensionIsLightning() const { return mDimensionIsLightning; }
    [[nodiscard]] bool hitPositionCanSeeSky() const { return mHitPositionCanSeeSky; }

private:
    Player&     mPlayer;
    std::string mProjectileTypeId;
    bool        mProjectileIsChanneling;
    std::string mStruckEntityTypeId;
    bool        mDimensionIsLightning;
    bool        mHitPositionCanSeeSky;
};

bool playerProjectileLightningHitEventSourceRegistered();
void registerPlayerProjectileLightningHitEventSource();
void unregisterPlayerProjectileLightningHitEventSource();

} // namespace advancements::event::entity