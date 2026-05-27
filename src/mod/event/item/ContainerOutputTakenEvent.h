#pragma once

#include "ll/api/event/Event.h"

#include "mc/deps/shared_types/legacy/ContainerType.h"
#include "mc/world/containers/ContainerEnumName.h"

class Player;

namespace advancements::event::item {

class ContainerOutputTakenEvent final : public ll::event::Event {
public:
    ContainerOutputTakenEvent(
        Player&                            player,
        ContainerEnumName                  sourceContainer,
        SharedTypes::Legacy::ContainerType screenType
    )
    : mPlayer(player),
      mSourceContainer(sourceContainer),
      mScreenType(screenType) {}

    [[nodiscard]] Player& player() const { return mPlayer; }
    [[nodiscard]] ContainerEnumName sourceContainer() const { return mSourceContainer; }
    [[nodiscard]] SharedTypes::Legacy::ContainerType screenType() const { return mScreenType; }

private:
    Player&                            mPlayer;
    ContainerEnumName                  mSourceContainer;
    SharedTypes::Legacy::ContainerType mScreenType;
};

bool containerOutputTakenEventSourceRegistered();
void registerContainerOutputTakenEventSource();
void unregisterContainerOutputTakenEventSource();

} // namespace advancements::event::item
