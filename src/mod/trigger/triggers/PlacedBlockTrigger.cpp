#include "mod/trigger/triggers/PlacedBlockTrigger.h"

#include "mod/Entry.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerPlaceBlockEvent.h"

#include "mc/world/Direction.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/VanillaStates.h"
#include "mc/world/level/block/states/BuiltInBlockStates.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <utility>

namespace advancements {
namespace {

constexpr auto ChiseledBookshelfBlockId = "minecraft:chiseled_bookshelf";
constexpr auto ComparatorBlockId        = "minecraft:comparator";
constexpr auto PoweredComparatorBlockId = "minecraft:powered_comparator";
constexpr auto LegacyComparatorBlockId  = "minecraft:unpowered_comparator";

struct HorizontalOffset {
    int             dx;
    int             dz;
    Direction::Type comparatorFacing;
};

constexpr std::array<HorizontalOffset, 4> BookshelfToComparatorOffsets{
    HorizontalOffset{0, 1, Direction::Type::North},
    HorizontalOffset{0, -1, Direction::Type::South},
    HorizontalOffset{1, 0, Direction::Type::West},
    HorizontalOffset{-1, 0, Direction::Type::East},
};

ll::event::ListenerPtr gPlacedBlockListener;

bool isChiseledBookshelf(Block const& block) { return block.getTypeName() == ChiseledBookshelfBlockId; }

bool isComparator(Block const& block) {
    auto const& blockId = block.getTypeName();
    return blockId == ComparatorBlockId || blockId == PoweredComparatorBlockId || blockId == LegacyComparatorBlockId;
}

BlockPos offset(BlockPos const& pos, int dx, int dz) { return BlockPos{pos.x + dx, pos.y, pos.z + dz}; }

std::optional<Direction::Type> comparatorFacing(Block const& block) {
    if (auto const cardinal = block.getState<Direction::Type>(BuiltInBlockStates::CardinalDirection())) {
        return *cardinal;
    }
    if (auto const legacy = block.getState<int>(VanillaStates::Direction())) {
        switch (*legacy) {
        case 0:
            return Direction::Type::South;
        case 1:
            return Direction::Type::West;
        case 2:
            return Direction::Type::North;
        case 3:
            return Direction::Type::East;
        default:
            return std::nullopt;
        }
    }
    return std::nullopt;
}

bool bookshelfHasBackFacingComparator(BlockSource const& region, BlockPos const& bookshelfPos) {
    return std::ranges::any_of(BookshelfToComparatorOffsets, [&](auto const& candidate) {
        auto const& block = region.getBlock(offset(bookshelfPos, candidate.dx, candidate.dz));
        if (!isComparator(block)) {
            return false;
        }
        auto const facing = comparatorFacing(block);
        return facing && *facing == candidate.comparatorFacing;
    });
}

bool isReadPowerOfChiseledBookshelfPlacement(ll::event::player::PlayerPlacedBlockEvent& event) {
    auto const& region      = event.self().getDimensionBlockSourceConst();
    auto const& placedBlock = region.getBlock(event.pos());
    return isChiseledBookshelf(placedBlock) && bookshelfHasBackFacingComparator(region, event.pos());
}

std::string advancementBlockId(Block const& block) {
    return isComparator(block) ? std::string{ComparatorBlockId} : block.getTypeName();
}

void dispatchPlacedBlock(Entry& mod, Player& player, std::string blockId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:placed_block",
            BlockTriggerPayload{std::move(blockId)},
        }
    );
}

} // namespace

bool placedBlockTriggerRegistered() { return gPlacedBlockListener != nullptr; }

void registerPlacedBlockTrigger(Entry& mod) {
    if (placedBlockTriggerRegistered()) {
        return;
    }

    gPlacedBlockListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerPlacedBlockEvent>(
        [&mod](auto& event) {
            if (isReadPowerOfChiseledBookshelfPlacement(event)) {
                dispatchPlacedBlock(mod, event.self(), advancementBlockId(event.self().getDimensionBlockSourceConst().getBlock(event.pos())));
            }
        }
    );
}

void unregisterPlacedBlockTrigger() {
    if (gPlacedBlockListener) {
        ll::event::EventBus::getInstance().removeListener(gPlacedBlockListener);
        gPlacedBlockListener.reset();
    }
}

} // namespace advancements
