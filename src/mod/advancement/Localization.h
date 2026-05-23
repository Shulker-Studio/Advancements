#pragma once

#include "mod/advancement/Advancement.h"

#include <string>
#include <string_view>

class Player;

namespace my_mod::advancement {

[[nodiscard]] std::string localizeText(TextComponent const& component, Player const& player);
[[nodiscard]] std::string localizeKey(std::string_view key, Player const& player);

} // namespace my_mod::advancement
