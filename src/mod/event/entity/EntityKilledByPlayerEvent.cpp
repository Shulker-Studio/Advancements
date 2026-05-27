#include "mod/event/entity/EntityKilledByPlayerEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/MobDieEvent.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <cmath>
#include <memory>
#include <optional>

namespace advancements::event::entity {
namespace {

ll::event::ListenerPtr gMobDieListener;

std::optional<Player*> findKillingPlayer(ll::event::MobDieEvent& event) {
    auto const& source = event.source();
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* player = event.self().getLastHurtByPlayer();
    if (player == nullptr) {
        return std::nullopt;
    }
    return player;
}

std::optional<std::string> findDirectDamagerTypeId(ActorDamageSource const& source) {
    if (!source.isEntitySource()) {
        return std::nullopt;
    }

    auto* actor = ll::service::getLevel()->fetchEntity(source.getDamagingEntityUniqueID(), false);
    if (actor == nullptr) {
        return std::nullopt;
    }
    return actor->getTypeName();
}

bool damageSourceDirectEntityIsArrow(ActorDamageSource const& source) {
    return findDirectDamagerTypeId(source) == "minecraft:arrow";
}

bool damageSourceIsSupportedProjectile(ActorDamageSource const& source) {
    return source.isChildEntitySource() || damageSourceDirectEntityIsArrow(source);
}

float horizontalDistance(Vec3 const& lhs, Vec3 const& rhs) {
    auto const dx = lhs.x - rhs.x;
    auto const dz = lhs.z - rhs.z;
    return std::sqrt(dx * dx + dz * dz);
}

std::unique_ptr<ll::event::EmitterBase> entityKilledByPlayerEventEmitterFactory();

class EntityKilledByPlayerEventEmitter : public ll::event::Emitter<entityKilledByPlayerEventEmitterFactory, EntityKilledByPlayerEvent> {
public:
    EntityKilledByPlayerEventEmitter() {
        gMobDieListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::MobDieEvent>([](auto& event) {
            auto player = findKillingPlayer(event);
            if (!player) {
                return true;
            }

            EntityKilledByPlayerEvent killedEvent{
                **player,
                event.self(),
                event.self().getTypeName(),
                horizontalDistance((**player).getPosition(), event.self().getPosition()),
                damageSourceIsSupportedProjectile(event.source()),
                findDirectDamagerTypeId(event.source()),
            };
            ll::event::EventBus::getInstance().publish(killedEvent);
            return true;
        });
    }

    ~EntityKilledByPlayerEventEmitter() override {
        if (gMobDieListener) {
            ll::event::EventBus::getInstance().removeListener(gMobDieListener);
            gMobDieListener.reset();
        }
    }
};

std::unique_ptr<ll::event::EmitterBase> entityKilledByPlayerEventEmitterFactory() {
    return std::make_unique<EntityKilledByPlayerEventEmitter>();
}

bool gEntityKilledByPlayerEventSourceRegistered = false;

} // namespace

bool entityKilledByPlayerEventSourceRegistered() { return gEntityKilledByPlayerEventSourceRegistered; }

void registerEntityKilledByPlayerEventSource() {
    if (entityKilledByPlayerEventSourceRegistered()) {
        return;
    }

    ll::event::EventBus::getInstance().setEventEmitter<EntityKilledByPlayerEvent>(entityKilledByPlayerEventEmitterFactory);
    gEntityKilledByPlayerEventSourceRegistered = true;
}

void unregisterEntityKilledByPlayerEventSource() { gEntityKilledByPlayerEventSourceRegistered = false; }

} // namespace advancements::event::entity
