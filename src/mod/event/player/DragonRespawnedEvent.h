#pragma once

#include "ll/api/event/Event.h"

namespace advancements::event::player {

class DragonRespawnedEvent final : public ll::event::Event {};

bool dragonRespawnedEventSourceRegistered();
void registerDragonRespawnedEventSource();
void unregisterDragonRespawnedEventSource();

} // namespace advancements::event::player
