#include "mod/event/entity/EntityHurtByPlayerEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/entity/ActorHurtEvent.h"
#include "ll/api/service/Bedrock.h"

#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorDamageSource.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <memory>

namespace advancements::event::entity {
namespace {

ll::event::ListenerPtr gActorHurtListener;

std::optional<Player*> findHurtingPlayer(ActorDamageSource const& source) {
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
    if (actor == nullptr || !actor->isPlayer()) {
        return std::nullopt;
    }
    return static_cast<Player*>(actor);
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

bool playerMainhandItemIsMace(Player const& player) {
    auto const& selectedItem = player.getSelectedItem();
    return !selectedItem.isNull() && selectedItem.getTypeName() == "minecraft:mace";
}

std::unique_ptr<ll::event::EmitterBase> entityHurtByPlayerEventEmitterFactory();

class EntityHurtByPlayerEventEmitter : public ll::event::Emitter<entityHurtByPlayerEventEmitterFactory, EntityHurtByPlayerEvent> {
public:
    EntityHurtByPlayerEventEmitter() {
        gActorHurtListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::ActorHurtEvent>([](auto& event) {
            auto hurtingPlayer = findHurtingPlayer(event.source());
            if (!hurtingPlayer) {
                return true;
            }

            EntityHurtByPlayerEvent hurtEvent{
                **hurtingPlayer,
                event.self(),
                damageSourceDirectEntityIsArrow(event.source()),
                findDirectDamagerTypeId(event.source()),
                damageSourceIsSupportedProjectile(event.source()),
                playerMainhandItemIsMace(**hurtingPlayer),
                event.damage(),
            };
            ll::event::EventBus::getInstance().publish(hurtEvent);
            return true;
        });
    }

    ~EntityHurtByPlayerEventEmitter() override {
        if (gActorHurtListener) {
            ll::event::EventBus::getInstance().removeListener(gActorHurtListener);
            gActorHurtListener.reset();
        }
    }
};

std::unique_ptr<ll::event::EmitterBase> entityHurtByPlayerEventEmitterFactory() {
    return std::make_unique<EntityHurtByPlayerEventEmitter>();
}

bool gEntityHurtByPlayerEventSourceRegistered = false;

} // namespace

bool entityHurtByPlayerEventSourceRegistered() { return gEntityHurtByPlayerEventSourceRegistered; }

void registerEntityHurtByPlayerEventSource() {
    if (entityHurtByPlayerEventSourceRegistered()) {
        return;
    }

    ll::event::EventBus::getInstance().setEventEmitter<EntityHurtByPlayerEvent>(entityHurtByPlayerEventEmitterFactory);
    gEntityHurtByPlayerEventSourceRegistered = true;
}

void unregisterEntityHurtByPlayerEventSource() { gEntityHurtByPlayerEventSourceRegistered = false; }

} // namespace advancements::event::entity
