#pragma once

class Player;

namespace my_mod {
class MyMod;
}

namespace my_mod::advancement {

void showAdvancementsGui(MyMod& mod, Player& player);

} // namespace my_mod::advancement
