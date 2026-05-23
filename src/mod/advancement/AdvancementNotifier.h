#pragma once

#include "mod/advancement/Advancement.h"

class Player;

namespace my_mod {
class MyMod;
}

namespace my_mod::advancement {

void notifyAdvancementCompleted(MyMod& mod, Player& player, AdvancementDefinition const& advancement);

} // namespace my_mod::advancement
