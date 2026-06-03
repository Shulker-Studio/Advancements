#include "mod/event/entity/PlayerAllayDroppedItemOnBlockEvent.h"

#include "ll/api/event/Emitter.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/memory/Hook.h"

#include "mc/legacy/ActorUniqueID.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/actor/ai/goal/GoAndGiveItemsToNoteblockGoal.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace advancements::event::entity {
namespace {

constexpr auto AllayId     = "minecraft:allay";
constexpr auto CakeId      = "minecraft:cake";
constexpr auto NoteBlockId = "minecraft:note_block";
constexpr auto BedrockNoteBlockId = "minecraft:noteblock";

std::unordered_map<int64, uint64> gPublishedAllayDropTargets;

bool isNoteBlockId(std::string_view blockId) {
    return blockId == NoteBlockId || blockId == BedrockNoteBlockId;
}

bool alreadyPublishedForTarget(int64 allayId, uint64 blockPosHash) {
    auto const found = gPublishedAllayDropTargets.find(allayId);
    if (found != gPublishedAllayDropTargets.end() && found->second == blockPosHash) {
        return true;
    }

    gPublishedAllayDropTargets[allayId] = blockPosHash;
    return false;
}

void clearPublishedDropForAllay(int64 allayId) {
    gPublishedAllayDropTargets.erase(allayId);
}

LL_TYPE_INSTANCE_HOOK(
    PlayerAllayDroppedItemOnBlockEventHook,
    HookPriority::Normal,
    GoAndGiveItemsToNoteblockGoal,
    &GoAndGiveItemsToNoteblockGoal::$tick,
    void
) {
    auto& mob = mMob;
    if (mob.getTypeName() != AllayId) {
        origin();
        return;
    }

    auto const& carriedItem = mob.getCarriedItem();
    auto const  carriedItemId = carriedItem.isNull() ? std::string{} : carriedItem.getTypeName();
    auto const  targetBlockId = mob.getDimensionBlockSourceConst().getBlock(mTargetBlockPos).getTypeName();
    auto*       player        = mob.getPlayerOwner();
    auto const  allayId       = mob.getOrCreateUniqueID().rawID;
    BlockPos const targetBlockPos = mTargetBlockPos;
    auto const  blockPosHash  = targetBlockPos.hashCode();

    origin();

    if (player == nullptr || carriedItemId != CakeId || !isNoteBlockId(targetBlockId)) {
        clearPublishedDropForAllay(allayId);
        return;
    }

    if (alreadyPublishedForTarget(allayId, blockPosHash)) {
        return;
    }

    PlayerAllayDroppedItemOnBlockEvent event{*player, CakeId, NoteBlockId};
    ll::event::EventBus::getInstance().publish(event);
}

std::unique_ptr<ll::event::EmitterBase> playerAllayDroppedItemOnBlockEventEmitterFactory();

class PlayerAllayDroppedItemOnBlockEventEmitter
    : public ll::event::Emitter<playerAllayDroppedItemOnBlockEventEmitterFactory, PlayerAllayDroppedItemOnBlockEvent> {
    ll::memory::HookRegistrar<PlayerAllayDroppedItemOnBlockEventHook> playerAllayDroppedItemOnBlockHook;
};

std::unique_ptr<ll::event::EmitterBase> playerAllayDroppedItemOnBlockEventEmitterFactory() {
    return std::make_unique<PlayerAllayDroppedItemOnBlockEventEmitter>();
}

bool gPlayerAllayDroppedItemOnBlockEventSourceRegistered = false;

} // namespace

bool playerAllayDroppedItemOnBlockEventSourceRegistered() {
    return gPlayerAllayDroppedItemOnBlockEventSourceRegistered;
}

void registerPlayerAllayDroppedItemOnBlockEventSource() {
    if (playerAllayDroppedItemOnBlockEventSourceRegistered()) {
        return;
    }

    (void)PlayerAllayDroppedItemOnBlockEventHook::_AutoHookCount;
    ll::event::EventBus::getInstance().setEventEmitter<PlayerAllayDroppedItemOnBlockEvent>(
        playerAllayDroppedItemOnBlockEventEmitterFactory
    );
    gPlayerAllayDroppedItemOnBlockEventSourceRegistered = true;
}

void unregisterPlayerAllayDroppedItemOnBlockEventSource() {
    gPublishedAllayDropTargets.clear();
    gPlayerAllayDroppedItemOnBlockEventSourceRegistered = false;
}

} // namespace advancements::event::entity
