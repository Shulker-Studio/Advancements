#include "mod/event/player/PlayerChargedRespawnAnchorEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/RespawnAnchorBlock.h"
#include "mc/world/level/block/VanillaStates.h"

#include <memory>
#include <optional>

namespace advancements::event::player {
namespace {

constexpr int RespawnAnchorFullCharge = 4;

std::optional<int> respawnAnchorCharge(BlockSource const& region, BlockPos const& pos) {
    return region.getBlock(pos).getState<int>(VanillaStates::RespawnAnchorCharge());
}

LL_TYPE_STATIC_HOOK(
    PlayerChargedRespawnAnchorEventHook,
    HookPriority::Normal,
    RespawnAnchorBlock,
    &RespawnAnchorBlock::_bumpCharge,
    void,
    BlockSource&    region,
    BlockPos const& pos,
    Player*         source,
    short           delta
) {
    auto const previousCharge = respawnAnchorCharge(region, pos);
    origin(region, pos, source, delta);
    auto const currentCharge = respawnAnchorCharge(region, pos);

    if (source == nullptr || delta <= 0 || !previousCharge || !currentCharge) {
        return;
    }
    if (*previousCharge < RespawnAnchorFullCharge && *currentCharge == RespawnAnchorFullCharge) {
        PlayerChargedRespawnAnchorEvent event{*source};
        ll::event::EventBus::getInstance().publish(event);
    }
}

std::unique_ptr<ll::event::EmitterBase> playerChargedRespawnAnchorEventEmitterFactory();

class PlayerChargedRespawnAnchorEventEmitter
    : public ll::event::Emitter<playerChargedRespawnAnchorEventEmitterFactory, PlayerChargedRespawnAnchorEvent> {
    ll::memory::HookRegistrar<PlayerChargedRespawnAnchorEventHook> playerChargedRespawnAnchorHook;
};

std::unique_ptr<ll::event::EmitterBase> playerChargedRespawnAnchorEventEmitterFactory() {
    return std::make_unique<PlayerChargedRespawnAnchorEventEmitter>();
}

bool gPlayerChargedRespawnAnchorEventSourceRegistered = false;

} // namespace

bool playerChargedRespawnAnchorEventSourceRegistered() { return gPlayerChargedRespawnAnchorEventSourceRegistered; }

void registerPlayerChargedRespawnAnchorEventSource() {
    if (playerChargedRespawnAnchorEventSourceRegistered()) {
        return;
    }

    (void)PlayerChargedRespawnAnchorEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerChargedRespawnAnchorEvent>(
        playerChargedRespawnAnchorEventEmitterFactory
    );
    gPlayerChargedRespawnAnchorEventSourceRegistered = true;
}

void unregisterPlayerChargedRespawnAnchorEventSource() { gPlayerChargedRespawnAnchorEventSourceRegistered = false; }

} // namespace advancements::event::player
