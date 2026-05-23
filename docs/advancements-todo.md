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
- `minecraft:player_killed_entity`
- `minecraft:entity_killed_player`
- `minecraft:changed_dimension`
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
| `enchanted_item` | missing-trigger | |
| `enter_block` | missing-trigger | |
| `entity_hurt_player` | missing-trigger | |
| `entity_killed_player` | done | 当前窄实现，支持 `conditions.entity` |
| `fall_after_explosion` | missing-trigger | |
| `fall_from_height` | missing-trigger | |
| `filled_bucket` | missing-trigger | |
| `fishing_rod_hooked` | missing-trigger | |
| `hero_of_the_village` | missing-trigger | |
| `impossible` | missing-trigger | 可后续纯数据实现 |
| `inventory_changed` | done | 当前窄实现，支持 `item` + `count` |
| `item_durability_changed` | missing-trigger | |
| `item_used_on_block` | missing-trigger | |
| `kill_mob_near_sculk_catalyst` | missing-trigger | |
| `killed_by_arrow` | missing-trigger | |
| `levitation` | missing-trigger | |
| `lightning_strike` | missing-trigger | |
| `location` | missing-trigger | |
| `nether_travel` | missing-trigger | |
| `placed_block` | missing-trigger | |
| `player_generates_container_loot` | missing-trigger | |
| `player_hurt_entity` | missing-trigger | |
| `player_interacted_with_entity` | missing-trigger | |
| `player_killed_entity` | done | 当前窄实现，支持 `conditions.entity` |
| `player_sheared_equipment` | missing-trigger | |
| `recipe_crafted` | missing-trigger | |
| `recipe_unlocked` | missing-trigger | |
| `ride_entity_in_lava` | missing-trigger | |
| `shot_crossbow` | missing-trigger | |
| `slept_in_bed` | missing-trigger | |
| `slide_down_block` | missing-trigger | |
| `started_riding` | missing-trigger | |
| `summoned_entity` | missing-trigger | |
| `spear_mobs` | missing-trigger | |
| `tame_animal` | missing-trigger | |
| `target_hit` | missing-trigger | |
| `thrown_item_picked_up_by_entity` | missing-trigger | |
| `thrown_item_picked_up_by_player` | missing-trigger | |
| `tick` | missing-trigger | 已不再用于现有 advancement 定义，应继续保持移除状态 |
| `used_ender_eye` | missing-trigger | |
| `used_totem` | missing-trigger | |
| `using_item` | missing-trigger | |
| `villager_trade` | missing-trigger | |
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
| `story/deflect_arrow` | `killed_by_arrow` | missing-trigger | |
| `story/enchant_item` | `enchanted_item` | missing-trigger | |
| `story/shiny_gear` | `inventory_changed` | done | |
| `story/enter_the_nether` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension` |
| `story/cure_zombie_villager` | `cured_zombie_villager` | missing-trigger | |
| `story/follow_ender_eye` | `used_ender_eye` | missing-trigger | |
| `story/enter_the_end` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension`；当前临时挂在 `story/enter_the_nether` 下，等 `follow_ender_eye` 补齐后回正父节点 |

## Nether Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `nether/root` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension` |
| `nether/return_to_sender` | projectile / ghast fireball family | missing-trigger | |
| `nether/find_bastion` | `location` / structure entry family | missing-trigger | |
| `nether/obtain_ancient_debris` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/fast_travel` | `nether_travel` | missing-trigger | |
| `nether/find_fortress` | `location` / structure entry family | missing-trigger | |
| `nether/uneasy_alliance` | complex entity transport / kill family | missing-trigger | |
| `nether/get_wither_skull` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/summon_wither` | `summoned_entity` | missing-trigger | |
| `nether/obtain_blaze_rod` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/create_beacon` | `construct_beacon` | missing-trigger | |
| `nether/brew_potion` | `brewed_potion` | missing-trigger | |
| `nether/all_potions` | `effects_changed` | missing-trigger | |
| `nether/create_full_beacon` | `construct_beacon` | missing-trigger | |
| `nether/all_effects` | `effects_changed` | missing-trigger | |
| `nether/explore_nether` | `location` | missing-trigger | |
| `nether/ride_strider` | ride / lava family | missing-trigger | |
| `nether/ride_strider_in_overworld_lava` | `ride_entity_in_lava` | missing-trigger | |
| `nether/distract_piglin` | interaction / inventory family | missing-trigger | |
| `nether/loot_bastion` | `player_generates_container_loot` | missing-trigger | |
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
| `end/find_end_city` | `location` | missing-trigger | |
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
| `adventure/shoot_arrow` | `target_hit` / arrow-hit family | missing-trigger | |
| `adventure/sniper_duel` | `killed_by_arrow` | missing-trigger | |
| `adventure/bullseye` | `target_hit` | missing-trigger | |
| `adventure/kill_all_mobs` | `player_killed_entity` | partial | 已补原版 ID，但当前仍受 runtime 支持怪物集合限制，未必达到原版完整可测范围 |
| `adventure/totem_of_undying` | `used_totem` | missing-trigger | |
| `adventure/ol_betsy` | `shot_crossbow` | missing-trigger | |
| `adventure/trade` | `villager_trade` | missing-trigger | |
| `adventure/trade_at_world_height` | `villager_trade` | missing-trigger | |
| `adventure/salvage_sherd` | other | missing-trigger | |
| `adventure/sleep_in_bed` | `slept_in_bed` | missing-trigger | |
| `adventure/adventuring_time` | `location` | missing-trigger | |
| `adventure/play_jukebox_in_meadows` | other | missing-trigger | |
| `adventure/fall_from_world_height` | `fall_from_height` | missing-trigger | |
| `adventure/trim_with_any_armor_pattern` | other | missing-trigger | |
| `adventure/read_power_of_chiseled_bookshelf` | other | missing-trigger | |
| `adventure/craft_decorated_pot_using_only_sherds` | other | missing-trigger | |
| `adventure/caves_and_cliffs` | `fall_from_height` / location family | missing-trigger | |
| `adventure/trim_with_all_exclusive_armor_patterns` | other | missing-trigger | |
| `adventure/who_needs_rockets` | other | missing-trigger | |
| `adventure/minecraft_trials_edition` | other | missing-trigger | |
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
| `husbandry/fishy_business` | `fishing_rod_hooked` | missing-trigger | 已删除旧 `inventory_changed` 近似 JSON；原版应按钓起的 item 匹配 |
| `husbandry/tactical_fishing` | `filled_bucket` | missing-trigger | 已删除旧 `inventory_changed` 近似 JSON；原版应按填充后的桶 item 匹配 |
| `husbandry/axolotl_in_a_bucket` | `inventory_changed` | missing-data | |
| `husbandry/kill_axolotl_target` | other | missing-trigger | |
| `husbandry/complete_catalogue` | `inventory_changed` / tame family | missing-trigger | |
| `husbandry/safely_harvest_honey` | other | missing-trigger | |
| `husbandry/wax_on` | other | missing-trigger | |
| `husbandry/wax_off` | other | missing-trigger | |
| `husbandry/tadpole_in_a_bucket` | `inventory_changed` | missing-data | |
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
- `husbandry/balanced_diet`
- `husbandry/obtain_netherite_hoe`

## Immediate Alignment Fixes

1. `adventure/kill_all_mobs` 用当前已支持敌对怪集合正式建成原版 ID，而不是 demo
2. `story/mine_stone` 继续评估是否要维持 `destroy_block` 近似，还是改成更贴近原版的获得语义
3. 实现 `fishing_rod_hooked` 后重新添加 `husbandry/fishy_business` 的原版 JSON
4. 实现 `filled_bucket` 后重新添加 `husbandry/tactical_fishing` 的原版 JSON
5. 基于 `inventory_changed` / `consume_item` 继续补其他 husbandry 原版条目（如 `obtain_sniffer_egg`）
6. 后续每完成一个 trigger，就回到本文件把对应 `missing-trigger` / `missing-data` 批量改状态
