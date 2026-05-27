#pragma once

#include "ll/api/event/Event.h"

#include <string>
#include <utility>

class Player;

namespace advancements::event::item {

class PlayerGeneratedContainerLootEvent final : public ll::event::Event {
public:
    PlayerGeneratedContainerLootEvent(Player& player, std::string lootTableId)
    : mPlayer(player),
      mLootTableId(std::move(lootTableId)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& lootTableId() const { return mLootTableId; }

private:
    Player&     mPlayer;
    std::string mLootTableId;
};

bool playerGeneratedContainerLootEventSourceRegistered();
void registerPlayerGeneratedContainerLootEventSource();
void unregisterPlayerGeneratedContainerLootEventSource();

} // namespace advancements::event::item
