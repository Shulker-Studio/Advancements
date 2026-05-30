#pragma once

#include "ll/api/event/Event.h"

#include <string>
#include <utility>
#include <vector>

class Player;

namespace advancements::event::item {

class PlayerGeneratedContainerLootEvent final : public ll::event::Event {
public:
    PlayerGeneratedContainerLootEvent(Player& player, std::string lootTableId, std::vector<std::string> generatedItemIds)
    : mPlayer(player),
      mLootTableId(std::move(lootTableId)),
      mGeneratedItemIds(std::move(generatedItemIds)) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] std::string const& lootTableId() const { return mLootTableId; }
    [[nodiscard]] std::vector<std::string> const& generatedItemIds() const { return mGeneratedItemIds; }

private:
    Player&                  mPlayer;
    std::string              mLootTableId;
    std::vector<std::string> mGeneratedItemIds;
};

bool playerGeneratedContainerLootEventSourceRegistered();
void registerPlayerGeneratedContainerLootEventSource();
void unregisterPlayerGeneratedContainerLootEventSource();

} // namespace advancements::event::item
