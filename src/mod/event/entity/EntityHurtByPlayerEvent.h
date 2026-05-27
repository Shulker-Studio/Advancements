#pragma once

#include <optional>
#include <string>

#include "ll/api/event/Event.h"

class Actor;
class Player;

namespace advancements::event::entity {

class EntityHurtByPlayerEvent final : public ll::event::Event {
public:
    EntityHurtByPlayerEvent(
        Player&                     player,
        Actor&                      target,
        bool                        directEntityIsArrow,
        std::optional<std::string>  directEntityTypeId,
        bool                        isProjectileDamage,
        bool                        mainhandItemIsMace,
        float                       damageDealt
    )
    : mPlayer(player),
      mTarget(target),
      mDirectEntityIsArrow(directEntityIsArrow),
      mDirectEntityTypeId(std::move(directEntityTypeId)),
      mIsProjectileDamage(isProjectileDamage),
      mMainhandItemIsMace(mainhandItemIsMace),
      mDamageDealt(damageDealt) {}

    [[nodiscard]] Player& self() const { return mPlayer; }
    [[nodiscard]] Actor& target() const { return mTarget; }
    [[nodiscard]] bool directEntityIsArrow() const { return mDirectEntityIsArrow; }
    [[nodiscard]] std::optional<std::string> const& directEntityTypeId() const { return mDirectEntityTypeId; }
    [[nodiscard]] bool isProjectileDamage() const { return mIsProjectileDamage; }
    [[nodiscard]] bool mainhandItemIsMace() const { return mMainhandItemIsMace; }
    [[nodiscard]] float damageDealt() const { return mDamageDealt; }

private:
    Player&                    mPlayer;
    Actor&                     mTarget;
    bool                       mDirectEntityIsArrow;
    std::optional<std::string> mDirectEntityTypeId;
    bool                       mIsProjectileDamage;
    bool                       mMainhandItemIsMace;
    float                      mDamageDealt;
};

bool entityHurtByPlayerEventSourceRegistered();
void registerEntityHurtByPlayerEventSource();
void unregisterEntityHurtByPlayerEventSource();

} // namespace advancements::event::entity
