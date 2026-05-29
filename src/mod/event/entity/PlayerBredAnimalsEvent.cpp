#include "mod/event/entity/PlayerBredAnimalsEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"

#include "mc/deps/ecs/WeakEntityRef.h"
#include "mc/entity/components_json_legacy/BreedableComponent.h"
#include "mc/legacy/ActorUniqueID.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

#include <memory>
#include <string>
#include <vector>

namespace advancements::event::entity {
namespace {

Player* resolveLoveCausePlayer(ActorUniqueID const& loveCause) {
    if (loveCause == ActorUniqueID::INVALID_ID()) {
        return nullptr;
    }

    auto* level = ll::service::getLevel().as_ptr();
    if (level == nullptr) {
        return nullptr;
    }
    return level->getPlayer(loveCause);
}

std::vector<std::string> collectChildTypeIds(std::vector<WeakEntityRef> const& children) {
    std::vector<std::string> childTypeIds;
    childTypeIds.reserve(children.size());
    for (auto const& child : children) {
        auto childActor = child.tryUnwrap<Actor>();
        if (!childActor) {
            continue;
        }
        childTypeIds.emplace_back(childActor->getTypeName());
    }
    return childTypeIds;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerBredAnimalsEventHook,
    HookPriority::Normal,
    BreedableComponent,
    &BreedableComponent::mate,
    std::vector<WeakEntityRef>,
    Actor& owner,
    Actor& partner
) {
    auto const loveCause = mLoveCause.get();
    auto result = origin(owner, partner);
    if (result.empty()) {
        return result;
    }

    auto childTypeIds = collectChildTypeIds(result);
    if (childTypeIds.empty()) {
        return result;
    }

    auto* player = resolveLoveCausePlayer(loveCause);
    if (player == nullptr) {
        return result;
    }

    PlayerBredAnimalsEvent event{*player, std::move(childTypeIds)};
    ll::event::EventBus::getInstance().publish(event);
    return result;
}

std::unique_ptr<ll::event::EmitterBase> playerBredAnimalsEventEmitterFactory();

class PlayerBredAnimalsEventEmitter
    : public ll::event::Emitter<playerBredAnimalsEventEmitterFactory, PlayerBredAnimalsEvent> {
    ll::memory::HookRegistrar<PlayerBredAnimalsEventHook> playerBredAnimalsHook;
};

std::unique_ptr<ll::event::EmitterBase> playerBredAnimalsEventEmitterFactory() {
    return std::make_unique<PlayerBredAnimalsEventEmitter>();
}

bool gPlayerBredAnimalsEventSourceRegistered = false;

} // namespace

bool playerBredAnimalsEventSourceRegistered() { return gPlayerBredAnimalsEventSourceRegistered; }

void registerPlayerBredAnimalsEventSource() {
    if (gPlayerBredAnimalsEventSourceRegistered) {
        return;
    }
    (void)PlayerBredAnimalsEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerBredAnimalsEvent>(playerBredAnimalsEventEmitterFactory);
    gPlayerBredAnimalsEventSourceRegistered = true;
}

void unregisterPlayerBredAnimalsEventSource() { gPlayerBredAnimalsEventSourceRegistered = false; }

} // namespace advancements::event::entity
