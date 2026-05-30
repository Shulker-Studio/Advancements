#include "mod/trigger/triggers/ItemUsedOnBlockTrigger.h"

#include "mod/Entry.h"
#include "mod/trigger/RuntimeTriggerAdaptersInternal.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerInteractBlockEvent.h"

#include "mc/deps/core/string/HashedString.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/ItemStackBase.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/biome/Biome.h"
#include "mc/world/level/biome/registry/VanillaBiomeNames.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/VanillaStates.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/BlockActorType.h"
#include "mc/world/level/block/actor/JukeboxBlockActor.h"
#include "mc/world/level/block/actor/SignBlockActor.h"

#include <array>
#include <optional>
#include <algorithm>
#include <string_view>

namespace advancements {
namespace {

constexpr int RespawnAnchorChargeBeforeFull = 3;
constexpr int FullBeehiveHoneyLevel = 5;

constexpr auto GlowstoneItemId = "minecraft:glowstone";
constexpr auto RespawnAnchorBlockId = "minecraft:respawn_anchor";
constexpr auto GlowInkSacItemId = "minecraft:glow_ink_sac";
constexpr auto GlassBottleItemId = "minecraft:glass_bottle";
constexpr auto HoneycombItemId = "minecraft:honeycomb";
constexpr auto JukeboxBlockId = "minecraft:jukebox";
constexpr auto CampfireBlockId = "minecraft:campfire";
constexpr auto SoulCampfireBlockId = "minecraft:soul_campfire";
constexpr auto CopperBulbBlockId = "minecraft:copper_bulb";
constexpr auto ExposedCopperBulbBlockId = "minecraft:exposed_copper_bulb";
constexpr auto WeatheredCopperBulbBlockId = "minecraft:weathered_copper_bulb";
constexpr auto OxidizedCopperBulbBlockId = "minecraft:oxidized_copper_bulb";

constexpr std::array<std::string_view, 2> BeehiveBlockIds{
    "minecraft:bee_nest",
    "minecraft:beehive",
};

constexpr std::array<std::string_view, 7> AxeItemIds{
    "minecraft:wooden_axe",
    "minecraft:stone_axe",
    "minecraft:iron_axe",
    "minecraft:golden_axe",
    "minecraft:diamond_axe",
    "minecraft:netherite_axe",
    "minecraft:copper_axe",
};

constexpr std::array<std::string_view, 48> WaxableCopperBlockIds{
    "minecraft:copper_block",
    "minecraft:exposed_copper",
    "minecraft:weathered_copper",
    "minecraft:oxidized_copper",
    "minecraft:cut_copper",
    "minecraft:exposed_cut_copper",
    "minecraft:weathered_cut_copper",
    "minecraft:oxidized_cut_copper",
    "minecraft:cut_copper_slab",
    "minecraft:exposed_cut_copper_slab",
    "minecraft:weathered_cut_copper_slab",
    "minecraft:oxidized_cut_copper_slab",
    "minecraft:cut_copper_stairs",
    "minecraft:exposed_cut_copper_stairs",
    "minecraft:weathered_cut_copper_stairs",
    "minecraft:oxidized_cut_copper_stairs",
    "minecraft:chiseled_copper",
    "minecraft:exposed_chiseled_copper",
    "minecraft:weathered_chiseled_copper",
    "minecraft:oxidized_chiseled_copper",
    "minecraft:copper_door",
    "minecraft:exposed_copper_door",
    "minecraft:weathered_copper_door",
    "minecraft:oxidized_copper_door",
    "minecraft:copper_trapdoor",
    "minecraft:exposed_copper_trapdoor",
    "minecraft:weathered_copper_trapdoor",
    "minecraft:oxidized_copper_trapdoor",
    "minecraft:copper_bars",
    "minecraft:exposed_copper_bars",
    "minecraft:weathered_copper_bars",
    "minecraft:oxidized_copper_bars",
    "minecraft:copper_grate",
    "minecraft:exposed_copper_grate",
    "minecraft:weathered_copper_grate",
    "minecraft:oxidized_copper_grate",
    CopperBulbBlockId,
    ExposedCopperBulbBlockId,
    WeatheredCopperBulbBlockId,
    OxidizedCopperBulbBlockId,
    "minecraft:copper_chest",
    "minecraft:exposed_copper_chest",
    "minecraft:weathered_copper_chest",
    "minecraft:oxidized_copper_chest",
    "minecraft:copper_golem_statue",
    "minecraft:exposed_copper_golem_statue",
    "minecraft:weathered_copper_golem_statue",
    "minecraft:oxidized_copper_golem_statue",
};

constexpr std::array<std::string_view, 48> WaxedCopperBlockIds{
    "minecraft:waxed_copper_block",
    "minecraft:waxed_exposed_copper",
    "minecraft:waxed_weathered_copper",
    "minecraft:waxed_oxidized_copper",
    "minecraft:waxed_cut_copper",
    "minecraft:waxed_exposed_cut_copper",
    "minecraft:waxed_weathered_cut_copper",
    "minecraft:waxed_oxidized_cut_copper",
    "minecraft:waxed_cut_copper_slab",
    "minecraft:waxed_exposed_cut_copper_slab",
    "minecraft:waxed_weathered_cut_copper_slab",
    "minecraft:waxed_oxidized_cut_copper_slab",
    "minecraft:waxed_cut_copper_stairs",
    "minecraft:waxed_exposed_cut_copper_stairs",
    "minecraft:waxed_weathered_cut_copper_stairs",
    "minecraft:waxed_oxidized_cut_copper_stairs",
    "minecraft:waxed_chiseled_copper",
    "minecraft:waxed_exposed_chiseled_copper",
    "minecraft:waxed_weathered_chiseled_copper",
    "minecraft:waxed_oxidized_chiseled_copper",
    "minecraft:waxed_copper_door",
    "minecraft:waxed_exposed_copper_door",
    "minecraft:waxed_weathered_copper_door",
    "minecraft:waxed_oxidized_copper_door",
    "minecraft:waxed_copper_trapdoor",
    "minecraft:waxed_exposed_copper_trapdoor",
    "minecraft:waxed_weathered_copper_trapdoor",
    "minecraft:waxed_oxidized_copper_trapdoor",
    "minecraft:waxed_copper_bars",
    "minecraft:waxed_exposed_copper_bars",
    "minecraft:waxed_weathered_copper_bars",
    "minecraft:waxed_oxidized_copper_bars",
    "minecraft:waxed_copper_grate",
    "minecraft:waxed_exposed_copper_grate",
    "minecraft:waxed_weathered_copper_grate",
    "minecraft:waxed_oxidized_copper_grate",
    "minecraft:waxed_copper_bulb",
    "minecraft:waxed_exposed_copper_bulb",
    "minecraft:waxed_weathered_copper_bulb",
    "minecraft:waxed_oxidized_copper_bulb",
    "minecraft:waxed_copper_chest",
    "minecraft:waxed_exposed_copper_chest",
    "minecraft:waxed_weathered_copper_chest",
    "minecraft:waxed_oxidized_copper_chest",
    "minecraft:waxed_copper_golem_statue",
    "minecraft:waxed_exposed_copper_golem_statue",
    "minecraft:waxed_weathered_copper_golem_statue",
    "minecraft:waxed_oxidized_copper_golem_statue",
};

ll::event::ListenerPtr gItemUsedOnBlockListener;

bool contains(std::span<std::string_view const> ids, std::string_view id) {
    return std::ranges::find(ids, id) != ids.end();
}

void dispatchItemUsedOnBlock(Entry& mod, Player& player, std::string itemId, std::string blockId) {
    dispatchTrigger(
        mod,
        TriggerContext{
            player,
            "minecraft:item_used_on_block",
            ItemUsedOnBlockPayload{std::move(itemId), std::move(blockId)},
        }
    );
}

std::optional<int> blockIntState(Player const& player, BlockPos const& pos, auto const& state) {
    return player.getDimensionBlockSourceConst().getBlock(pos).getState<int>(state);
}

std::optional<bool> blockBoolState(Player const& player, BlockPos const& pos, auto const& state) {
    return player.getDimensionBlockSourceConst().getBlock(pos).getState<bool>(state);
}

bool hasCampfireSmokeForHive(Player const& player, BlockPos const& pos) {
    for (auto dy = 1; dy <= 2; ++dy) {
        for (auto dx = -1; dx <= 1; ++dx) {
            for (auto dz = -1; dz <= 1; ++dz) {
                auto const smokePos = BlockPos{pos.x + dx, pos.y - dy, pos.z + dz};
                auto const& block   = player.getDimensionBlockSourceConst().getBlock(smokePos);
                auto const blockId  = block.getTypeName();
                if (blockId != CampfireBlockId && blockId != SoulCampfireBlockId) {
                    continue;
                }

                auto const extinguished = block.getState<bool>(VanillaStates::Extinguished());
                if (!extinguished || !*extinguished) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::optional<int> respawnAnchorCharge(Player const& player, BlockPos const& pos) {
    return blockIntState(player, pos, VanillaStates::RespawnAnchorCharge());
}

bool isChargeRespawnAnchorUse(ll::event::player::PlayerInteractBlockEvent& event, std::string_view itemId, std::string_view blockId) {
    if (blockId != RespawnAnchorBlockId || itemId != GlowstoneItemId) {
        return false;
    }

    auto const charge = respawnAnchorCharge(event.self(), event.blockPos());
    return charge && *charge == RespawnAnchorChargeBeforeFull;
}

SignBlockActor* signBlockActorAt(Player& player, BlockPos const& pos) {
    auto* blockActor = player.getDimensionBlockSource().getBlockEntity(pos);
    if (blockActor == nullptr || (blockActor->mType != BlockActorType::Sign && blockActor->mType != BlockActorType::HangingSign)) {
        return nullptr;
    }
    return static_cast<SignBlockActor*>(blockActor);
}

bool signSideCanGlow(SignBlockActor const& blockActor, SignTextSide side) {
    auto const& text = side == SignTextSide::Front ? blockActor.mTextFront : blockActor.mTextBack;
    return text != nullptr && !text->mGlowing;
}

bool isSignGlowUse(ll::event::player::PlayerInteractBlockEvent& event, std::string_view itemId) {
    if (itemId != GlowInkSacItemId) {
        return false;
    }

    auto* blockActor = signBlockActorAt(event.self(), event.blockPos());
    if (blockActor == nullptr || blockActor->mIsWaxed) {
        return false;
    }

    auto const side = blockActor->getSideFacingPlayer(event.self());
    return signSideCanGlow(*blockActor, side);
}

bool isJukeboxMeadowUse(ll::event::player::PlayerInteractBlockEvent& event, std::string_view blockId) {
    if (blockId != JukeboxBlockId) {
        return false;
    }
    auto const* item = event.item().getItem();
    if (item == nullptr || !item->isMusicDisk()) {
        return false;
    }

    auto const* blockActor = event.self().getDimensionBlockSourceConst().getBlockEntity(event.blockPos());
    if (blockActor == nullptr || blockActor->mType != BlockActorType::Jukebox) {
        return false;
    }
    auto const* jukebox = static_cast<JukeboxBlockActor const*>(blockActor);
    if (!jukebox->mRecord->isNull()) {
        return false;
    }

    auto const& biome = event.self().getDimensionBlockSourceConst().getBiome(event.blockPos());
    return biome.mHash->getString() == VanillaBiomeNames::Meadow();
}

bool isSafeHoneyHarvestUse(ll::event::player::PlayerInteractBlockEvent& event, std::string_view itemId, std::string_view blockId) {
    if (itemId != GlassBottleItemId || !contains(BeehiveBlockIds, blockId)) {
        return false;
    }

    auto const honeyLevel = blockIntState(event.self(), event.blockPos(), VanillaStates::BeehiveHoneyLevel());
    return honeyLevel && *honeyLevel >= FullBeehiveHoneyLevel && hasCampfireSmokeForHive(event.self(), event.blockPos());
}

bool isWaxOnUse(std::string_view itemId, std::string_view blockId) {
    return itemId == HoneycombItemId && contains(WaxableCopperBlockIds, blockId);
}

bool isAxeItem(std::string_view itemId) {
    return contains(AxeItemIds, itemId);
}

bool isWaxOffUse(std::string_view itemId, std::string_view blockId) {
    return isAxeItem(itemId) && contains(WaxedCopperBlockIds, blockId);
}

bool isLightenUpUse(ll::event::player::PlayerInteractBlockEvent& event, std::string_view itemId, std::string_view blockId) {
    if (!isAxeItem(itemId)) {
        return false;
    }
    if (blockId != ExposedCopperBulbBlockId && blockId != WeatheredCopperBulbBlockId && blockId != OxidizedCopperBulbBlockId) {
        return false;
    }
    auto const lit = blockBoolState(event.self(), event.blockPos(), VanillaStates::Lit());
    return lit && *lit;
}

void handleItemUsedOnBlock(Entry& mod, ll::event::player::PlayerInteractBlockEvent& event) {
    if (event.isCancelled()) {
        return;
    }

    auto const* block = event.block().as_ptr();
    if (block == nullptr) {
        return;
    }
    auto const blockId = block->getTypeName();
    if (event.item().isNull()) {
        return;
    }
    auto const itemId = event.item().getTypeName();

    if (isChargeRespawnAnchorUse(event, itemId, blockId) || isSignGlowUse(event, itemId) || isJukeboxMeadowUse(event, blockId)
        || isSafeHoneyHarvestUse(event, itemId, blockId) || isWaxOnUse(itemId, blockId) || isWaxOffUse(itemId, blockId)
        || isLightenUpUse(event, itemId, blockId)) {
        dispatchItemUsedOnBlock(mod, event.self(), itemId, blockId);
    }
}

} // namespace

bool itemUsedOnBlockTriggerRegistered() { return gItemUsedOnBlockListener != nullptr; }

void registerItemUsedOnBlockTrigger(Entry& mod) {
    if (itemUsedOnBlockTriggerRegistered()) {
        return;
    }

    gItemUsedOnBlockListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerInteractBlockEvent>(
        [&mod](auto& event) { handleItemUsedOnBlock(mod, event); },
        ll::event::EventPriority::Lowest
    );
}

void unregisterItemUsedOnBlockTrigger() {
    if (gItemUsedOnBlockListener) {
        ll::event::EventBus::getInstance().removeListener(gItemUsedOnBlockListener);
        gItemUsedOnBlockListener.reset();
    }
}

} // namespace advancements
