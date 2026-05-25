#pragma once

#include "mod/commands/AdvancementCommandIndex.h"

namespace advancements {

class Entry;

namespace commands {

void registerAdvancementsCommand(Entry& mod);
void unregisterAdvancementsCommand();
void updateAdvancementCommandEnums(AdvancementCommandIndex const& index);

} // namespace commands
} // namespace advancements
