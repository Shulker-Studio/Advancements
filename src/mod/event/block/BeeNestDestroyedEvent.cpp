#include "mod/event/block/BeeNestDestroyedEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/BeehiveBlock.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BeehiveBlockActor.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/BlockActorType.h"

#include <memory>
#include <string>

namespace advancements::event::block {
namespace {

BeehiveBlockActor const* asBeehiveBlockActor(BlockActor const* blockActor) {
    if (blockActor == nullptr || blockActor->mType != BlockActorType::Beehive) {
        return nullptr;
    }
    return static_cast<BeehiveBlockActor const*>(blockActor);
}

int countBeesInside(BlockSource const& region, BlockPos const& pos) {
    auto const* blockActor = asBeehiveBlockActor(region.getBlockEntity(pos));
    if (blockActor == nullptr) {
        return 0;
    }
    return static_cast<int>(blockActor->mOccupants->size());
}

LL_TYPE_INSTANCE_HOOK(
    BeeNestDestroyedEventHook,
    HookPriority::Normal,
    BeehiveBlock,
    &BeehiveBlock::$playerWillDestroy,
    Block const*,
    Player&         player,
    BlockPos const& pos,
    Block const&    block
) {
    BeeNestDestroyedEvent event{player, block.getTypeName(), countBeesInside(player.getDimensionBlockSourceConst(), pos)};
    auto const* result = origin(player, pos, block);

    ll::event::EventBus::getInstance().publish(event);
    return result;
}

std::unique_ptr<ll::event::EmitterBase> beeNestDestroyedEventEmitterFactory();

class BeeNestDestroyedEventEmitter
    : public ll::event::Emitter<beeNestDestroyedEventEmitterFactory, BeeNestDestroyedEvent> {
    ll::memory::HookRegistrar<BeeNestDestroyedEventHook> beeNestDestroyedHook;
};

std::unique_ptr<ll::event::EmitterBase> beeNestDestroyedEventEmitterFactory() {
    return std::make_unique<BeeNestDestroyedEventEmitter>();
}

bool gBeeNestDestroyedEventSourceRegistered = false;

} // namespace

bool beeNestDestroyedEventSourceRegistered() { return gBeeNestDestroyedEventSourceRegistered; }

void registerBeeNestDestroyedEventSource() {
    if (beeNestDestroyedEventSourceRegistered()) {
        return;
    }

    (void)BeeNestDestroyedEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<BeeNestDestroyedEvent>(beeNestDestroyedEventEmitterFactory);
    gBeeNestDestroyedEventSourceRegistered = true;
}

void unregisterBeeNestDestroyedEventSource() { gBeeNestDestroyedEventSourceRegistered = false; }

} // namespace advancements::event::block
