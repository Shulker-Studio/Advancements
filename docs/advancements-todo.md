# Advancements Full TODO Inventory

本文件用于把 Java wiki 上的 trigger 和 advancement 先完整抄下来，再对照当前项目的 runtime / data 覆盖情况打标记，方便随时查看整体进度。

数据精确度说明：

- 本文件优先参考 Java wiki 的进度列表与 trigger 列表。
- 对于已单独核过的原版 JSON（例如通过 `SPGoding/vanilla-datapack` 的 raw 文件），会在备注中明确写“已核原版 JSON”。
- 对于尚未逐条核 raw JSON 的部分，trigger family 仍可能是按 wiki 语义做的归类，足够用于总览和推进，但不代表已经做到逐文件 100% 精确映射。

状态标记：

- `done`：runtime 已支持，且已有对应 JSON 定义
- `partial`：runtime 或数据只做了近似 / 演示，还不算原版完全对齐
- `missing-trigger`：还没有对应 runtime 支持
- `missing-data`：runtime 已支持，但还没有对应 JSON 定义

## Current Runtime Trigger Support

当前项目已实现的 trigger：

- `minecraft:inventory_changed`
- `minecraft:consume_item`
- `minecraft:used_totem`
- `minecraft:filled_bucket`
- `minecraft:fishing_rod_hooked`
- `minecraft:player_killed_entity`
- `minecraft:entity_killed_player`
- `minecraft:player_hurt_entity`
- `minecraft:slept_in_bed`
- `minecraft:changed_dimension`
- `minecraft:villager_trade`
- `minecraft:enchanted_item`
- `minecraft:player_generates_container_loot`
- `bedrock:player_destroy_block`

当前项目已暂时移除/不应继续依赖的临时脚手架：

- `minecraft:tick` 仅作为早期简化 root 使用过，后续应移除并改回原版语义

## Full Java Trigger Inventory

根据 wiki `进度定义格式` 页面目录整理：

| Trigger | Status | Notes |
| --- | --- | --- |
| `allay_drop_item_on_block` | missing-trigger | |
| `any_block_use` | missing-trigger | |
| `avoid_vibration` | missing-trigger | |
| `bee_nest_destroyed` | missing-trigger | |
| `bred_animals` | missing-trigger | |
| `brewed_potion` | missing-trigger | |
| `changed_dimension` | done | 当前窄实现，支持 `conditions.from` / `conditions.to`；通知时机仍有 caveat |
| `channeled_lightning` | missing-trigger | |
| `construct_beacon` | missing-trigger | |
| `consume_item` | done | 当前窄实现，仅 `conditions.item` |
| `crafter_recipe_crafted` | missing-trigger | |
| `cured_zombie_villager` | missing-trigger | |
| `default_block_use` | missing-trigger | |
| `effects_changed` | missing-trigger | |
| `enchanted_item` | done | 当前窄实现：基于 `ItemStackRequestActionHandler::_handleTransfer`，仅在从 `CreatedOutputContainer` 成功转移且当前 screen type 为 `ContainerType::Enchantment` 后触发；不依赖打开附魔 UI |
| `enter_block` | missing-trigger | |
| `entity_hurt_player` | done | 当前仅支持 `story/deflect_arrow` 已核窄形状：`damage.blocked = true` + `damage.type.tags` 含 `minecraft:is_projectile`；runtime 使用 `Player::_blockUsingShield` 成功路径派发 |
| `entity_killed_player` | done | 当前窄实现，支持 `conditions.entity` |
| `fall_after_explosion` | missing-trigger | |
| `fall_from_height` | missing-trigger | |
| `filled_bucket` | done | 当前窄实现，基于 `BucketItem::$_useOn` 对可入桶实体（鱼 / 蝌蚪 / 美西螈）交互成功后的结果桶 item 匹配 |
| `fishing_rod_hooked` | done | 当前窄实现，基于 LSE `FishingHook::_pullCloser` 语义支持钓起的 item |
| `hero_of_the_village` | missing-trigger | |
| `impossible` | missing-trigger | 可后续纯数据实现 |
| `inventory_changed` | done | 当前窄实现，支持 `item` + `count` |
| `item_durability_changed` | missing-trigger | |
| `item_used_on_block` | missing-trigger | |
| `kill_mob_near_sculk_catalyst` | missing-trigger | |
| `killed_by_arrow` | missing-trigger | |
| `levitation` | missing-trigger | |
| `lightning_strike` | missing-trigger | |
| `location` | partial | 当前窄实现：仅支持已核原版 JSON 的 `conditions.player[0].predicate.location.structures` 结构进入条件，覆盖 bastion、fortress、end_city、stronghold、trial_chambers；按 location 语义每 20 tick 轮询，不支持 biome、y-position、维度或通用 location predicate |
| `nether_travel` | missing-trigger | |
| `placed_block` | missing-trigger | |
| `player_generates_container_loot` | done | 当前窄实现：基于 `Util::LootTableUtils::fillContainer`，仅支持玩家作为 loot context entity 生成的四个 bastion chest loot table 的 `conditions.loot_table` 精确匹配 |
| `player_hurt_entity` | done | 当前窄实现：基于 `ll::event::ActorHurtEvent`，仅支持 `damage.type.direct_entity.type = #minecraft:arrows` + `damage.type.tags` 含 `minecraft:is_projectile` 这一已核 condition surface |
| `player_interacted_with_entity` | missing-trigger | |
| `player_killed_entity` | done | 当前窄实现，支持 `conditions.entity` |
| `player_sheared_equipment` | missing-trigger | |
| `recipe_crafted` | missing-trigger | |
| `recipe_unlocked` | missing-trigger | |
| `ride_entity_in_lava` | missing-trigger | |
| `shot_crossbow` | done | 当前窄实现：仅支持 `conditions.item.items = minecraft:crossbow`（`adventure/ol_betsy` 需求形状） |
| `slept_in_bed` | done | 当前窄实现，Hook `Player::$startSleepInBed` 且仅在 `BedSleepingResult::Ok` 后触发 |
| `slide_down_block` | missing-trigger | |
| `started_riding` | missing-trigger | |
| `summoned_entity` | missing-trigger | |
| `spear_mobs` | missing-trigger | |
| `tame_animal` | missing-trigger | |
| `target_hit` | partial | 当前仅支持 `adventure/bullseye` 已核窄形状：`signal_strength = 15` + `projectile[0].condition = minecraft:entity_properties` + `projectile[0].entity = this` + `projectile[0].predicate.distance.horizontal.min = 30.0` |
| `thrown_item_picked_up_by_entity` | missing-trigger | |
| `thrown_item_picked_up_by_player` | missing-trigger | |
| `tick` | missing-trigger | 已不再用于现有 advancement 定义，应继续保持移除状态 |
| `used_ender_eye` | missing-trigger | Java 原版存在该 trigger，但当前项目已不再保留其独立运行时路径；`story/follow_ender_eye` 已回正为 `minecraft:location` + stronghold 结构条件 |
| `used_totem` | done | 当前窄实现，复用 item 条件；Hook `Player::$consumeTotem` 且仅在实际消耗图腾后触发 |
| `using_item` | missing-trigger | |
| `villager_trade` | done | 当前窄实现：基于 `ItemStackRequestActionHandler::_handleTransfer`，仅在从 `CreatedOutputContainer` 成功转移且当前 screen type 为 `ContainerType::Trade` 后触发；不依赖打开交易 UI |
| `voluntary_exile` | missing-trigger | |

## Story Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `story/root` | `inventory_changed` | done | 已核原版 JSON：获得 `minecraft:crafting_table`；当前定义已回正 |
| `story/mine_stone` | `inventory_changed` | done | 已按原版核心语义回正：获得 `cobblestone / blackstone / cobbled_deepslate` 之一 |
| `story/upgrade_tools` | `inventory_changed` | done | |
| `story/smelt_iron` | `inventory_changed` | done | |
| `story/iron_tools` | `inventory_changed` | done | |
| `story/lava_bucket` | `inventory_changed` | done | |
| `story/obtain_armor` | `inventory_changed` | done | |
| `story/mine_diamond` | `inventory_changed` | done | |
| `story/form_obsidian` | `inventory_changed` | done | |
| `story/deflect_arrow` | `entity_hurt_player` | done | 已补数据并接入窄实现：仅支持已核 shape（`damage.blocked = true` + `damage.type.tags` 含 `minecraft:is_projectile`），保持非泛化 |
| `story/enchant_item` | `enchanted_item` | done | 已补数据，复用当前完成附魔结果转移语义的 `enchanted_item` |
| `story/shiny_gear` | `inventory_changed` | done | |
| `story/enter_the_nether` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension` |
| `story/cure_zombie_villager` | `cured_zombie_villager` | missing-trigger | |
| `story/follow_ender_eye` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:stronghold`；当前窄实现基于玩家所在 Stronghold 结构触发，按 location 语义每 20 tick 轮询，不再依赖 `used_ender_eye` |
| `story/enter_the_end` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension`；父节点已回正为 `story/follow_ender_eye` |

## Nether Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `nether/root` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension` |
| `nether/return_to_sender` | projectile / ghast fireball family | missing-trigger | |
| `nether/find_bastion` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:bastion_remnant`；当前窄实现基于玩家所在结构触发 |
| `nether/obtain_ancient_debris` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/fast_travel` | `nether_travel` | missing-trigger | |
| `nether/find_fortress` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:fortress`；当前窄实现基于玩家所在结构触发 |
| `nether/uneasy_alliance` | complex entity transport / kill family | missing-trigger | |
| `nether/get_wither_skull` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/summon_wither` | `summoned_entity` | missing-trigger | |
| `nether/obtain_blaze_rod` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/create_beacon` | `construct_beacon` | missing-trigger | |
| `nether/brew_potion` | `brewed_potion` | missing-trigger | |
| `nether/all_potions` | `effects_changed` | missing-trigger | |
| `nether/create_full_beacon` | `construct_beacon` | missing-trigger | |
| `nether/all_effects` | `effects_changed` | missing-trigger | |
| `nether/explore_nether` | `location` | missing-trigger | 已核原版 JSON：使用 biome location 条件并要求遍历多个 Nether biome；超出本轮 structure-entry 窄实现 |
| `nether/ride_strider` | ride / lava family | missing-trigger | |
| `nether/ride_strider_in_overworld_lava` | `ride_entity_in_lava` | missing-trigger | |
| `nether/distract_piglin` | interaction / inventory family | missing-trigger | |
| `nether/loot_bastion` | `player_generates_container_loot` | done | 已核原版 JSON：父级 `minecraft:nether/find_bastion`，四个 bastion chest loot table 条件，OR requirements；当前窄 runtime 仅覆盖这四个 loot table |
| `nether/use_lodestone` | location / use item family | missing-trigger | |
| `nether/obtain_crying_obsidian` | `inventory_changed` | missing-data | runtime 可承载，后续可直接补数据 |
| `nether/charge_respawn_anchor` | interaction / block use family | missing-trigger | |

当前总评：多数 `nether/*` 仍是 `missing-trigger`；纯“获得某物”型条目已有一批通过 `inventory_changed` 补齐，`obtain_crying_obsidian` 仍缺数据。

## End Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `end/root` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension` |
| `end/kill_dragon` | boss kill family | missing-trigger | |
| `end/dragon_egg` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `end/enter_end_gateway` | location / gateway family | missing-trigger | |
| `end/elytra` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `end/levitate` | `levitation` | missing-trigger | |
| `end/respawn_dragon` | summon / boss event family | missing-trigger | |
| `end/find_end_city` | `location` | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:end_city`；当前窄实现基于玩家所在结构触发 |
| `end/dragon_breath` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |

当前总评：`end/*` 仍主要卡在 `location / levitation / boss` 这些未实现 trigger；root 与少量物品获得型条目已具备基础数据。

## Adventure Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `adventure/root` | `player_killed_entity` OR `entity_killed_player` | done | 已按原版补齐 `killed_something` / `killed_by_something` 两个 criterion，并使用 OR requirement |
| `adventure/kill_a_mob` | `player_killed_entity` | partial | 已按原版 hostile 集合方向大幅扩展，但仍需继续核完整列表与当前 runtime 可测范围 |
| `adventure/voluntary_exile` | `voluntary_exile` | missing-trigger | |
| `adventure/spyglass_at_parrot` | other | missing-trigger | |
| `adventure/spyglass_at_ghast` | other | missing-trigger | |
| `adventure/spyglass_at_dragon` | other | missing-trigger | |
| `adventure/throw_trident` | other | missing-trigger | |
| `adventure/very_very_frightening` | other | missing-trigger | |
| `adventure/kill_mob_near_sculk_catalyst` | `kill_mob_near_sculk_catalyst` | missing-trigger | |
| `adventure/shoot_arrow` | `player_hurt_entity` | done | 已补数据并接入窄实现：仅支持 `damage.type.direct_entity.type = #minecraft:arrows` 与 `damage.type.tags` 含 `minecraft:is_projectile` 这组已核 surface；保持非泛化 |
| `adventure/sniper_duel` | `player_killed_entity` | missing-trigger | Phase 0 已核原版 JSON：`minecraft:player_killed_entity`，需 `entity` 谓词锁定 `minecraft:skeleton` 且水平距离 `>= 50`，并要求 `killing_blow.tags` 含 `minecraft:is_projectile`；不是 `killed_by_arrow` |
| `adventure/bullseye` | `target_hit` | done | 已落地本地 JSON + lang；runtime/条件解析仅支持该行已核窄形状（`signal_strength = 15` 与 projectile 水平距离 `>= 30`） |
| `adventure/kill_all_mobs` | `player_killed_entity` | partial | 已补原版 ID，但当前仍受 runtime 支持怪物集合限制，未必达到原版完整可测范围 |
| `adventure/totem_of_undying` | `used_totem` | done | 已补数据，复用现有 `used_totem` |
| `adventure/ol_betsy` | `shot_crossbow` | missing-trigger | |
| `adventure/trade` | `villager_trade` | done | 已补数据，复用当前完成交易语义的 `villager_trade` |
| `adventure/trade_at_world_height` | `villager_trade` | missing-trigger | |
| `adventure/salvage_sherd` | other | missing-trigger | |
| `adventure/sleep_in_bed` | `slept_in_bed` | done | 已补数据，复用现有 `slept_in_bed` |
| `adventure/adventuring_time` | `location` | missing-trigger | |
| `adventure/play_jukebox_in_meadows` | other | missing-trigger | |
| `adventure/fall_from_world_height` | `fall_from_height` | missing-trigger | |
| `adventure/trim_with_any_armor_pattern` | other | missing-trigger | |
| `adventure/read_power_of_chiseled_bookshelf` | other | missing-trigger | |
| `adventure/craft_decorated_pot_using_only_sherds` | other | missing-trigger | |
| `adventure/caves_and_cliffs` | `fall_from_height` / location family | missing-trigger | |
| `adventure/trim_with_all_exclusive_armor_patterns` | other | missing-trigger | |
| `adventure/who_needs_rockets` | other | missing-trigger | |
| `adventure/minecraft_trials_edition` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:trial_chambers`；当前窄实现基于玩家所在 Trial Chambers 结构触发，按 location 语义每 20 tick 轮询 |
| `adventure/lighten_up` | other | missing-trigger | |
| `adventure/blowback` | other | missing-trigger | |
| `adventure/under_lock_and_key` | other | missing-trigger | |
| `adventure/revaulting` | other | missing-trigger | |
| `adventure/overoverkill` | `player_hurt_entity` / damage family | missing-trigger | |
| `adventure/brush_armadillo` | other | missing-trigger | |
| `adventure/minecart_improvements` | other | missing-trigger | |
| `adventure/spyglass_at_ghast` | other | missing-trigger | duplicate name check from wiki if needed |

补充：当前已有近似定义：

- `adventure/root`
- `adventure/kill_a_mob`

## Husbandry Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `husbandry/root` | `consume_item` | done | 已核原版 JSON：`consume_item` 且 `conditions` 为空；当前定义已回正 |
| `husbandry/plant_seed` | other | missing-trigger | |
| `husbandry/breed_an_animal` | `bred_animals` | missing-trigger | |
| `husbandry/balanced_diet` | `consume_item` | partial | 已补原版 ID，但当前仅实现已支持 consumable 子集，不是原版完整食物集合 |
| `husbandry/obtain_netherite_hoe` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `husbandry/tame_an_animal` | `tame_animal` | missing-trigger | |
| `husbandry/fishy_business` | `fishing_rod_hooked` | done | 原版 JSON 已恢复，按钓起的四种鱼 item 匹配 |
| `husbandry/tactical_fishing` | `filled_bucket` | done | 原版 JSON 已恢复，按填充后的四种鱼桶 item 匹配 |
| `husbandry/axolotl_in_a_bucket` | `filled_bucket` | done | 已按填充后的美西螈桶 item 匹配 |
| `husbandry/kill_axolotl_target` | other | missing-trigger | |
| `husbandry/complete_catalogue` | `inventory_changed` / tame family | missing-trigger | |
| `husbandry/safely_harvest_honey` | other | missing-trigger | |
| `husbandry/wax_on` | other | missing-trigger | |
| `husbandry/wax_off` | other | missing-trigger | |
| `husbandry/tadpole_in_a_bucket` | `filled_bucket` | done | 已按填充后的蝌蚪桶 item 匹配 |
| `husbandry/leash_all_frog_variants` | other | missing-trigger | |
| `husbandry/froglights` | `inventory_changed` | missing-data | |
| `husbandry/silk_touch_nest` | `inventory_changed` | missing-data | |
| `husbandry/ride_a_boat_with_a_goat` | other | missing-trigger | |
| `husbandry/make_a_sign_glow` | other | missing-trigger | |
| `husbandry/allay_deliver_item_to_player` | other | missing-trigger | |
| `husbandry/allay_deliver_cake_to_note_block` | `allay_drop_item_on_block` | missing-trigger | |
| `husbandry/obtain_sniffer_egg` | `inventory_changed` | missing-data | |
| `husbandry/feed_snifflet` | other | missing-trigger | |
| `husbandry/plant_any_sniffer_seed` | other | missing-trigger | |
| `husbandry/remove_wolf_armor` | other | missing-trigger | |
| `husbandry/repair_wolf_armor` | other | missing-trigger | |

补充：当前已保留的 husbandry 定义：

- `husbandry/root`
- `husbandry/fishy_business`
- `husbandry/tactical_fishing`
- `husbandry/tadpole_in_a_bucket`
- `husbandry/axolotl_in_a_bucket`
- `husbandry/balanced_diet`
- `husbandry/obtain_netherite_hoe`

## Immediate Alignment Fixes

1. `adventure/kill_all_mobs` 用当前已支持敌对怪集合正式建成原版 ID，而不是 demo
2. `story/mine_stone` 继续评估是否要维持 `destroy_block` 近似，还是改成更贴近原版的获得语义
3. 基于 `inventory_changed` / `consume_item` 继续补其他 husbandry 原版条目（如 `obtain_sniffer_egg`）
4. 后续每完成一个 trigger，就回到本文件把对应 `missing-trigger` / `missing-data` 批量改状态
