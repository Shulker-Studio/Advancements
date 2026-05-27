#include "mod/trigger/triggers/LevitationTrigger.h"

#include "mod/Entry.h"
#include "mod/event/player/PlayerTickEvent.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/effect/MobEffect.h"

#include <unordered_map>

namespace advancements {
namespace {

struct LevitationPlayerState {
    Vec3 startPosition;
};

ll::event::ListenerPtr gLevitationTickListener;
std::unordered_map<mce::UUID, LevitationPlayerState> gLevitationPlayerStates;

void dispatchLevitation(Entry& mod, Player& player, float verticalDistance) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:levitation",
            LevitationTriggerPayload{verticalDistance},
        }
    );
}

void checkLevitation(Entry& mod, Player& player) {
    auto* levitation = MobEffect::LEVITATION();
    if (levitation == nullptr || !player.hasEffect(*levitation)) {
        auto const found = gLevitationPlayerStates.find(player.getUuid());
        if (found != gLevitationPlayerStates.end()) {
            auto const endPosition = player.getPosition();
            dispatchLevitation(mod, player, endPosition.y - found->second.startPosition.y);
            gLevitationPlayerStates.erase(found);
        }
        return;
    }

    gLevitationPlayerStates.try_emplace(player.getUuid(), LevitationPlayerState{player.getPosition()});
}

} // namespace

bool levitationTriggerRegistered() { return gLevitationTickListener != nullptr; }

void registerLevitationTrigger(Entry& mod) {
    if (levitationTriggerRegistered()) {
        return;
    }

    gLevitationTickListener = ll::event::EventBus::getInstance().emplaceListener<event::player::PlayerTickEvent>([&mod](auto& event) {
        checkLevitation(mod, event.self());
    });
}

void unregisterLevitationTrigger() {
    gLevitationPlayerStates.clear();
    if (gLevitationTickListener) {
        ll::event::EventBus::getInstance().removeListener(gLevitationTickListener);
        gLevitationTickListener.reset();
    }
}

} // namespace advancements
