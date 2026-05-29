#pragma once

#include "ll/api/event/Event.h"

#include <string>
#include <utility>
#include <vector>

class Player;

namespace advancements::event::entity {

class PlayerBredAnimalsEvent final : public ll::event::Event {
public:
    explicit PlayerBredAnimalsEvent(Player& player, std::vector<std::string> childTypeIds)
    : mPlayer(player), mChildTypeIds(std::move(childTypeIds)) {}

    [[nodiscard]] Player&                         player() const { return mPlayer; }
    [[nodiscard]] std::vector<std::string> const& childTypeIds() const { return mChildTypeIds; }

private:
    Player&                  mPlayer;
    std::vector<std::string> mChildTypeIds;
};

bool playerBredAnimalsEventSourceRegistered();
void registerPlayerBredAnimalsEventSource();
void unregisterPlayerBredAnimalsEventSource();

} // namespace advancements::event::entity
