#include "mod/advancement/Localization.h"

#include "ll/api/i18n/I18n.h"

#include "mc/world/actor/player/Player.h"

namespace advancements {

std::string localizeKey(std::string_view key, Player const& player) {
    auto const localized = ll::i18n::getInstance().get(key, player.getLocaleCode());
    if (!localized.empty()) {
        return std::string{localized};
    }
    return std::string{key};
}

std::string localizeText(TextComponent const& component, Player const& player) {
    if (component.kind == TextComponentKind::Text) {
        return component.value;
    }

    return localizeKey(component.value, player);
}

} // namespace advancements
