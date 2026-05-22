#pragma once

#include "mod/advancement/AdvancementLoader.h"

namespace my_mod {

class MyMod;

namespace commands {

void registerAdvancementsCommand(MyMod& mod);
void unregisterAdvancementsCommand();
void updateAdvancementCommandEnums(advancement::LoadResult const& result);

} // namespace commands
} // namespace my_mod
