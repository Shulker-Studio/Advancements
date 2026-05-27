#pragma once

#include <optional>
#include <string>

#include "ll/api/event/Event.h"

class Actor;
class Player;

namespace advancements::event::entity {

class EntityKilledByPlayerEvent final : public ll::event::Event {
public:
    EntityKilledByPlayerEvent(
        Player&                    player,
        Actor&                     entity,
        std::string                killedEntityTypeId,
        float                      horizontalDistance,
        bool                       killingBlowIsProjectile,
        std::optional<std::string> directEntityTypeId
    )
    : mPlayer(player),
      mEntity(entity),
      mKilledEntityTypeId(std::move(killedEntityTypeId)),
      mHorizontalDistance(horizontalDistance),
      mKillingBlowIsProjectile(killingBlowIsProjectile),
      mDirectEntityTypeId(std::move(directEntityTypeId)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] Actor& entity() const { return mEntity; }
    [[nodiscard]] std::string const& killedEntityTypeId() const { return mKilledEntityTypeId; }
    [[nodiscard]] float horizontalDistance() const { return mHorizontalDistance; }
    [[nodiscard]] bool killingBlowIsProjectile() const { return mKillingBlowIsProjectile; }
    [[nodiscard]] std::optional<std::string> const& directEntityTypeId() const { return mDirectEntityTypeId; }

private:
    Player&                    mPlayer;
    Actor&                     mEntity;
    std::string                mKilledEntityTypeId;
    float                      mHorizontalDistance;
    bool                       mKillingBlowIsProjectile;
    std::optional<std::string> mDirectEntityTypeId;
};

bool entityKilledByPlayerEventSourceRegistered();
void registerEntityKilledByPlayerEventSource();
void unregisterEntityKilledByPlayerEventSource();

} // namespace advancements::event::entity
