#include "mod/advancement/AdvancementNotifier.h"

#include "mod/MyMod.h"
#include "mod/advancement/Localization.h"

#include "mc/network/packet/TextPacket.h"
#include "mc/network/packet/ToastRequestPacket.h"
#include "mc/world/actor/player/Player.h"

#include <format>
#include <string>

ToastRequestPacket::ToastRequestPacket() { mSerializationMode = SerializationMode::ManualOnly; }
ToastRequestPacketPayload::ToastRequestPacketPayload() = default;

namespace my_mod::advancement {
namespace {

std::string advancementTitle(AdvancementDefinition const& advancement, Player const& player) {
    if (advancement.display) {
        return localizeText(advancement.display->title, player);
    }
    return advancement.id;
}

bool shouldShowToast(AdvancementDefinition const& advancement) {
    return advancement.display && advancement.display->showToast.value_or(true);
}

bool shouldAnnounceToChat(AdvancementDefinition const& advancement) {
    return advancement.display && advancement.display->announceToChat.value_or(true);
}

std::string_view chatNotificationKey(AdvancementDefinition const& advancement) {
    if (!advancement.display || !advancement.display->frame) {
        return "advancements.notification.chat.task";
    }

    if (*advancement.display->frame == "goal") {
        return "advancements.notification.chat.goal";
    }
    if (*advancement.display->frame == "challenge") {
        return "advancements.notification.chat.challenge";
    }
    return "advancements.notification.chat.task";
}

std::string_view toastTitleKey(AdvancementDefinition const& advancement) {
    if (!advancement.display || !advancement.display->frame) {
        return "advancements.notification.toast_title.task";
    }

    if (*advancement.display->frame == "challenge") {
        return "advancements.notification.toast_title.challenge";
    }
    return "advancements.notification.toast_title.goal";
}

} // namespace

void notifyAdvancementCompleted(MyMod& mod, Player& player, AdvancementDefinition const& advancement) {
    auto const title = advancementTitle(advancement, player);

    if (shouldShowToast(advancement)) {
        ToastRequestPacket packet;
        packet.mTitle   = localizeKey(toastTitleKey(advancement), player);
        packet.mContent = title;
        player.sendNetworkPacket(packet);
    }

    if (shouldAnnounceToChat(advancement)) {
        auto const playerName = player.getRealName();
        auto packet = TextPacket::createRawMessage(
            std::vformat(localizeKey(chatNotificationKey(advancement), player), std::make_format_args(playerName, title))
        );
        packet.sendToClients();
    }

    mod.getSelf().getLogger().debug("Advancements debug: notified completion advancement={} player={}", advancement.id, player.getRealName());
}

} // namespace my_mod::advancement
