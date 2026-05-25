#pragma once

#include "mod/advancement/Advancement.h"

class Player;

namespace advancements {
class Entry;
}

namespace advancements {

void notifyAdvancementCompleted(Entry& mod, Player& player, AdvancementDefinition const& advancement);

} // namespace advancements
