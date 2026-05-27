#include "mod/event/player/PlayerKilledByEntityEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDieEvent.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <memory>
#include <optional>

namespace advancements::event::player {
namespace {

ll::event::ListenerPtr gPlayerDieListener;

std::optional<std::string> findKillerEntityTypeId(ActorDamageSource const& source) {
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* actor = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (actor == nullptr) {
        return std::nullopt;
    }
    if (source.isChildEntitySource()) {
        actor = actor->getOwner();
    }
    if (actor == nullptr) {
        return std::nullopt;
    }
    return actor->getTypeName();
}

std::unique_ptr<ll::event::EmitterBase> playerKilledByEntityEventEmitterFactory();

class PlayerKilledByEntityEventEmitter : public ll::event::Emitter<playerKilledByEntityEventEmitterFactory, PlayerKilledByEntityEvent> {
public:
    PlayerKilledByEntityEventEmitter() {
        gPlayerDieListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerDieEvent>([](auto& event) {
            auto const killerEntityTypeId = findKillerEntityTypeId(event.source());
            if (!killerEntityTypeId) {
                return true;
            }

            PlayerKilledByEntityEvent killedEvent{event.self(), *killerEntityTypeId};
            ll::event::EventBus::getInstance().publish(killedEvent);
            return true;
        });
    }

    ~PlayerKilledByEntityEventEmitter() override {
        if (gPlayerDieListener) {
            ll::event::EventBus::getInstance().removeListener(gPlayerDieListener);
            gPlayerDieListener.reset();
        }
    }
};

std::unique_ptr<ll::event::EmitterBase> playerKilledByEntityEventEmitterFactory() {
    return std::make_unique<PlayerKilledByEntityEventEmitter>();
}

bool gPlayerKilledByEntityEventSourceRegistered = false;

} // namespace

bool playerKilledByEntityEventSourceRegistered() { return gPlayerKilledByEntityEventSourceRegistered; }

void registerPlayerKilledByEntityEventSource() {
    if (playerKilledByEntityEventSourceRegistered()) {
        return;
    }

    ll::event::EventBus::getInstance().setEventEmitter<PlayerKilledByEntityEvent>(playerKilledByEntityEventEmitterFactory);
    gPlayerKilledByEntityEventSourceRegistered = true;
}

void unregisterPlayerKilledByEntityEventSource() { gPlayerKilledByEntityEventSourceRegistered = false; }

} // namespace advancements::event::player
