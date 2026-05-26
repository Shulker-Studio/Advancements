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
- `minecraft:cured_zombie_villager`
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
- `minecraft:nether_travel`
- `minecraft:levitation`
- `minecraft:summoned_entity`
- `minecraft:brewed_potion`
- `minecraft:construct_beacon`
- `minecraft:effects_changed`
- `minecraft:enter_block`（当前窄实现：hook `EndGatewayBlockActor::teleportEntity(Actor&)`，仅 `{ "block": "minecraft:end_gateway" }` / `end/enter_end_gateway`）
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
| `brewed_potion` | done | 当前窄实现：基于 `ItemStackRequestActionHandler::_handleTransfer`，仅在从 `BrewingStandResultContainer` 成功转移且当前 screen type 为 `ContainerType::BrewingStand` 后触发；不检查输出物品类型，匹配 wiki/原版“从酿造台输出栏获取任意物品”的语义 |
| `changed_dimension` | done | 当前窄实现，支持 `conditions.from` / `conditions.to`；通知时机仍有 caveat |
| `channeled_lightning` | missing-trigger | |
| `construct_beacon` | done | 当前窄实现：hook `BeaconBlockActor::checkShape` 关键刷新函数，在当前 `mNumLevels` 从低层级提升为更高层级时触发；仅支持裸条件和 `conditions.level.min`，并按 wiki 语义派发给同维度、信标中心水平切比雪夫距离 10 格内、垂直向下 9 格到向上 5 格内的玩家；实测该关键函数约每 80 tick / 4 秒进入一次，避免直接监听每 tick 热路径；仍需 live-server QA 验证 Bedrock 层级刷新时机与 Java 激活语义严格对齐 |
| `consume_item` | done | 当前窄实现，仅 `conditions.item` |
| `crafter_recipe_crafted` | missing-trigger | |
| `cured_zombie_villager` | partial | 当前窄实现：仅支持无 `conditions` 的 `story/cure_zombie_villager`；基于玩家对僵尸村民成功使用金苹果时记录责任玩家，并在后续 transformation seam 上确认转化；不支持未核实的 `zombie` / `villager` 条件谓词 |
| `default_block_use` | missing-trigger | |
| `effects_changed` | done | 当前窄实现：hook `Actor::addEffect` 后仅对玩家派发，并按当前玩家已拥有的效果快照匹配 `conditions.effects` 中每个空对象 effect；已扩到当前 Bedrock/LL 可见 effect id 集合，并接受 `hero_of_the_village -> village_hero` alias；按当前项目约定，`glowing` / `dolphins_grace` 两个 Java-only 效果不纳入 Bedrock 测试集合 |
| `enchanted_item` | done | 当前窄实现：基于 `ItemStackRequestActionHandler::_handleTransfer`，仅在从 `CreatedOutputContainer` 成功转移且当前 screen type 为 `ContainerType::Enchantment` 后触发；不依赖打开附魔 UI |
| `enter_block` | done | 当前窄实现：hook `EndGatewayBlockActor::teleportEntity(Actor&)`，仅当原传送逻辑执行后玩家位置发生变化时派发；仅用于 `end/enter_end_gateway` 的 `{ "block": "minecraft:end_gateway" }` 形状 |
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
| `item_used_on_block` | done | 当前窄实现：仅支持 `nether/charge_respawn_anchor` 形状 `conditions.item = minecraft:glowstone` + `conditions.block = minecraft:respawn_anchor`；runtime hook `RespawnAnchorBlock::_bumpCharge`，只在玩家触发的正向充能把 `RespawnAnchorCharge` 从小于 4 提升到 4 时派发；保留 `Advancements debug: respawn_anchor_bump_charge ...` 日志供 live-server QA |
| `kill_mob_near_sculk_catalyst` | missing-trigger | |
| `killed_by_arrow` | missing-trigger | |
| `levitation` | done | 当前窄实现：基于 `Player::$tickWorld` 观察玩家 `MobEffect::LEVITATION()` 状态，仅支持已核 wiki 语义/本地 JSON 形状 `conditions.distance.y.min = 50.0`；效果开始时记录起始 Y，效果结束时用结束 Y 结算一次绝对垂直位移；仍需 live-server QA 验证潜影贝弹丸给予的 Bedrock levitation 状态与 Java 完成时机严格对齐 |
| `lightning_strike` | missing-trigger | |
| `location` | partial | 当前窄实现：仅支持已核原版 JSON 的 `conditions.player[0].predicate.location.structures` 结构进入条件，覆盖 bastion、fortress、end_city、stronghold、trial_chambers；按 location 语义每 20 tick 轮询，不支持 biome、y-position、维度或通用 location predicate |
| `nether_travel` | done | 当前窄实现：仅支持已核原版 `nether/fast_travel` 形状 `conditions.distance.horizontal.min`；runtime 在玩家从主世界进入下界前记录主世界水平起点，并在从下界返回主世界时按主世界水平距离触发；未泛化 `distance` 其他子键 |
| `placed_block` | missing-trigger | |
| `player_generates_container_loot` | done | 当前窄实现：基于 `Util::LootTableUtils::fillContainer`，仅支持玩家作为 loot context entity 生成的四个 bastion chest loot table 的 `conditions.loot_table` 精确匹配 |
| `player_hurt_entity` | done | 当前窄实现：基于 `ll::event::ActorHurtEvent`，仅支持 `damage.type.direct_entity.type = #minecraft:arrows` + `damage.type.tags` 含 `minecraft:is_projectile` 这一已核 condition surface |
| `player_interacted_with_entity` | missing-trigger | |
| `player_killed_entity` | done | 当前窄实现：保留 `conditions.entity`，并额外支持 `adventure/sniper_duel` 已核窄形状（骷髅 + 50m horizontal + projectile killing blow）与 `nether/return_to_sender` 已核窄形状（恶魂 + direct fireball + projectile killing blow）；不泛化其他 nested predicates |
| `player_sheared_equipment` | missing-trigger | |
| `recipe_crafted` | missing-trigger | |
| `recipe_unlocked` | missing-trigger | |
| `ride_entity_in_lava` | missing-trigger | |
| `shot_crossbow` | done | 当前窄实现：仅支持 `conditions.item.items = minecraft:crossbow`（`adventure/ol_betsy` 需求形状） |
| `slept_in_bed` | done | 当前窄实现，Hook `Player::$startSleepInBed` 且仅在 `BedSleepingResult::Ok` 后触发 |
| `slide_down_block` | missing-trigger | |
| `started_riding` | missing-trigger | |
| `summoned_entity` | done | 当前窄实现：仅支持已核原版 `nether/summon_wither` 与 `end/respawn_dragon` 形状；Wither 走 `SkullBlock::checkMobSpawn` 成功路径并对同维度 50 格切比雪夫距离内玩家派发，Ender Dragon 走 `EndDragonFight::tryRespawn()` 进入复活流程后的窄派发 |
| `spear_mobs` | missing-trigger | |
| `tame_animal` | missing-trigger | |
| `target_hit` | partial | 当前仅支持 `adventure/bullseye` 已核窄形状：`signal_strength = 15` + `projectile[0].condition = minecraft:entity_properties` + `projectile[0].entity = this` + `projectile[0].predicate.distance.horizontal.min = 30.0` |
| `thrown_item_picked_up_by_entity` | missing-trigger | |
| `thrown_item_picked_up_by_player` | missing-trigger | |
| `tick` | missing-trigger | 已不再用于现有 advancement 定义，应继续保持移除状态 |
| `used_ender_eye` | missing-trigger | Java 原版存在该 trigger，但当前项目已不再保留其独立运行时路径；`story/follow_ender_eye` 已回正为 `minecraft:location` + stronghold 结构条件 |
| `used_totem` | done | 当前窄实现，复用 item 条件；Hook `Player::$consumeTotem` 且仅在实际消耗图腾后触发 |
| `using_item` | missing-trigger | |
| `villager_trade` | done | 当前窄实现：基于 `ItemStackRequestActionHandler::_handleTransfer`，仅在从 `CreatedOutputContainer` 成功转移且当前 screen type 为 `ContainerType::Trade` 后触发；不依赖打开交易 UI；条件解析支持空条件 `adventure/trade` 与已核窄形状 `player[0].predicate.minecraft:location.position.y.min`（`adventure/trade_at_world_height`） |
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
| `story/cure_zombie_villager` | `cured_zombie_villager` | partial | 已发货窄实现；本地 row 使用裸 `minecraft:cured_zombie_villager` trigger。raw vanilla JSON 未能在本波次取得，依据本地 wiki 摘录和可见 LL headers 保持最小形状；runtime 当前在成功金苹果交互时记录责任玩家，并在 `TransformationComponent::maintainOldData(...)` 观察到 tracked 的 `zombie_villager(_v2) -> villager(_v2)` 转化时派发进度。当前主要 caveat 是未对原版 `conditions.zombie` / `conditions.villager` 做支持，且仍需 live server QA 继续证明该完成点与僵尸村民治疗完成严格对齐 |
| `story/follow_ender_eye` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:stronghold`；当前窄实现基于玩家所在 Stronghold 结构触发，按 location 语义每 20 tick 轮询，不再依赖 `used_ender_eye` |
| `story/enter_the_end` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension`；父节点已回正为 `story/follow_ender_eye` |

## Nether Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `nether/root` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension` |
| `nether/return_to_sender` | `player_killed_entity` / ghast fireball narrow slice | done | 已核原版 JSON：`minecraft:player_killed_entity` + `entity[0].predicate.type = minecraft:ghast` + `killing_blow.direct_entity.type = minecraft:fireball` + `killing_blow.tags` 含 `minecraft:is_projectile`；当前 runtime 复用 MobDieEvent/player attribution，并从 ActorDamageSource 记录 direct damager type；需 live-server QA 证明 Bedrock 反弹恶魂火球击杀时 direct damager 为 `minecraft:fireball` |
| `nether/find_bastion` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:bastion_remnant`；当前窄实现基于玩家所在结构触发 |
| `nether/obtain_ancient_debris` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/fast_travel` | `nether_travel` | done | 已核原版 JSON：`minecraft:nether_travel` + `conditions.distance.horizontal.min = 7000.0` + `rewards.experience = 100`；当前窄 runtime 使用下界进出切维 seam 记录/结算水平距离，仍需 live-server QA 验证跨 portal 往返链路时序 |
| `nether/find_fortress` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:fortress`；当前窄实现基于玩家所在结构触发 |
| `nether/uneasy_alliance` | complex entity transport / kill family | missing-trigger | |
| `nether/get_wither_skull` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/summon_wither` | `summoned_entity` | done | 已核原版 JSON：父级 `minecraft:nether/get_wither_skull`，`minecraft:summoned_entity` + `entity[0].predicate.type = minecraft:wither`；当前窄 runtime 基于 `SkullBlock::checkMobSpawn` 成功路径，对同维度 50 格切比雪夫距离内玩家派发 |
| `nether/obtain_blaze_rod` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/create_beacon` | `construct_beacon` | done | 已补数据并接入窄实现：裸 `minecraft:construct_beacon`，基于激活信标层级变为大于 0 时派发 |
| `nether/brew_potion` | `brewed_potion` | done | 已补数据，复用当前酿造台输出槽成功取物语义的 `brewed_potion` |
| `nether/all_potions` | `effects_changed` | done | 已按本地 wiki 语义补齐：父级 `minecraft:nether/brew_potion`，`minecraft:effects_changed` + 17 个 required effect 空条件，奖励 100 XP；runtime hook `Actor::addEffect` 后对玩家当前 effect 快照匹配 |
| `nether/create_full_beacon` | `construct_beacon` | done | 已补数据并接入窄实现：`minecraft:construct_beacon` + `conditions.level.min = 4`，复用信标层级提升派发；仍需 live-server QA 验证满级信标刷新时机 |
| `nether/all_effects` | `effects_changed` | done | 已补本地 JSON：父级 `minecraft:nether/all_potions`，按当前项目定义使用 Bedrock 可见效果全集，并显式排除 `glowing` / `dolphins_grace` 两个 Java-only 效果；奖励 1000 XP，hidden 保持 true |
| `nether/explore_nether` | `location` | missing-trigger | 已核原版 JSON：使用 biome location 条件并要求遍历多个 Nether biome；超出本轮 structure-entry 窄实现 |
| `nether/ride_strider` | ride / lava family | missing-trigger | |
| `nether/ride_strider_in_overworld_lava` | `ride_entity_in_lava` | missing-trigger | |
| `nether/distract_piglin` | interaction / inventory family | missing-trigger | |
| `nether/loot_bastion` | `player_generates_container_loot` | done | 已核原版 JSON：父级 `minecraft:nether/find_bastion`，四个 bastion chest loot table 条件，OR requirements；当前窄 runtime 仅覆盖这四个 loot table |
| `nether/use_lodestone` | location / use item family | missing-trigger | |
| `nether/obtain_crying_obsidian` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/charge_respawn_anchor` | `item_used_on_block` / respawn anchor charge narrow slice | done | 已补数据并接入窄实现：criterion `charge_respawn_anchor` 使用 `minecraft:item_used_on_block` + `item = minecraft:glowstone` + `block = minecraft:respawn_anchor`；runtime hook `RespawnAnchorBlock::_bumpCharge` 并在 `delta > 0`、`source != nullptr`、充能从 `< 4` 变为 `4` 时派发；debug 日志会打印 player、pos、delta、before、after，供 live-server QA 验证 seam |

当前总评：多数 `nether/*` 仍是 `missing-trigger`；纯“获得某物”型条目已有一批通过 `inventory_changed` 补齐，包含 `obtain_crying_obsidian`；`return_to_sender` 已作为 `player_killed_entity` 的恶魂火球窄切片补齐；`fast_travel` 已作为 `nether_travel` 窄切片补齐；`summon_wither` 已作为 `summoned_entity` 凋灵窄切片补齐；`charge_respawn_anchor` 已作为 `item_used_on_block` / `_bumpCharge` 满充能窄切片补齐；`all_potions` 已作为 `effects_changed` 的 17 效果快照窄切片补齐；`all_effects` 已按当前项目定义作为“排除 `glowing` / `dolphins_grace` 的 Bedrock 子集”补齐。

## End Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `end/root` | `changed_dimension` | done | 已补数据，复用现有 `changed_dimension` |
| `end/kill_dragon` | `player_killed_entity` | done | 已核原版 JSON：父级 `minecraft:end/root`，`minecraft:player_killed_entity` + `entity = minecraft:ender_dragon`；当前窄实现复用现有 `MobDieEvent` / `player_killed_entity` 路径 |
| `end/dragon_egg` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `end/enter_end_gateway` | `enter_block` / end gateway narrow slice | done | 已核原版 JSON：父级 `minecraft:end/kill_dragon`，`minecraft:enter_block` + `conditions.block = minecraft:end_gateway`；当前窄 runtime 基于 `EndGatewayBlockActor::teleportEntity(Actor&)` 专用传送 seam，并在原传送逻辑后确认玩家位置变化再派发，不再走玩家 tick 轮询；live-server QA 已确认玩家使用折跃门会触发并授予该进度 |
| `end/elytra` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `end/levitate` | `levitation` | done | 已补本地 JSON：父级 `minecraft:end/find_end_city`，`minecraft:levitation` + `conditions.distance.y.min = 50.0`，奖励 50 XP；runtime 为当前窄 `levitation` 开始/结束位置结算切片 |
| `end/respawn_dragon` | `summoned_entity` / EndDragonFight respawn seam | done | 已核原版 JSON：父级 `minecraft:end/kill_dragon`，运行时按 `minecraft:summoned_entity` + `entity[0].predicate.type = minecraft:ender_dragon` 窄派发；当前窄实现基于 `EndDragonFight::tryRespawn()` 进入复活流程后的状态变化 |
| `end/find_end_city` | `location` | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:end_city`；当前窄实现基于玩家所在结构触发 |
| `end/dragon_breath` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |

当前总评：`end/*` 里 `kill_dragon`、`enter_end_gateway`、`respawn_dragon` 与 `levitate` 已补齐窄实现；后续 End 剩余风险主要是 live-server QA 与更宽泛 trigger 语义确认。

## Adventure Vanilla Inventory

| Vanilla ID | Main trigger family | Status | Notes |
| --- | --- | --- | --- |
| `adventure/root` | `player_killed_entity` OR `entity_killed_player` | done | 已按原版补齐 `killed_something` / `killed_by_something` 两个 criterion，并使用 OR requirement |
| `adventure/kill_a_mob` | `player_killed_entity` | done | 已补齐当前本地 hostile 集合窄切片，复用现有 `player_killed_entity` 简单实体匹配；本波次补入 `bogged`、`breeze`、`creaking`、`evoker`、`wither`、`zoglin` 六项，并保持不泛化其他 nested predicates |
| `adventure/voluntary_exile` | `voluntary_exile` | missing-trigger | |
| `adventure/spyglass_at_parrot` | other | missing-trigger | |
| `adventure/spyglass_at_ghast` | other | missing-trigger | |
| `adventure/spyglass_at_dragon` | other | missing-trigger | |
| `adventure/throw_trident` | other | missing-trigger | |
| `adventure/very_very_frightening` | other | missing-trigger | |
| `adventure/kill_mob_near_sculk_catalyst` | `kill_mob_near_sculk_catalyst` | missing-trigger | |
| `adventure/shoot_arrow` | `player_hurt_entity` | done | 已补数据并接入窄实现：仅支持 `damage.type.direct_entity.type = #minecraft:arrows` 与 `damage.type.tags` 含 `minecraft:is_projectile` 这组已核 surface；保持非泛化 |
| `adventure/sniper_duel` | `player_killed_entity` | done | 已补本地 JSON + lang，并接入已核窄实现：仅支持 skeleton + 水平距离 `>= 50.0` + projectile killing_blow tags；保持非泛化 |
| `adventure/bullseye` | `target_hit` | done | 已落地本地 JSON + lang；runtime/条件解析仅支持该行已核窄形状（`signal_strength = 15` 与 projectile 水平距离 `>= 30`） |
| `adventure/kill_all_mobs` | `player_killed_entity` | done | 已补齐当前本地 hostile 集合窄切片，继续复用现有 `player_killed_entity` 简单实体匹配；本波次补入 `bogged`、`breeze`、`creaking`、`evoker`、`wither`、`zoglin` 六项，并与本地历史已跑通清单对齐 |
| `adventure/totem_of_undying` | `used_totem` | done | 已补数据，复用现有 `used_totem` |
| `adventure/ol_betsy` | `shot_crossbow` | missing-trigger | |
| `adventure/trade` | `villager_trade` | done | 已补数据，复用当前完成交易语义的 `villager_trade` |
| `adventure/trade_at_world_height` | `villager_trade` | done | 已按原版 raw JSON 形状补数据：父级 `minecraft:adventure/trade`，`minecraft:villager_trade` + `player[0].predicate.location.position.y.min = 319.0`；当前窄实现按玩家交易完成时脚部位置 `Y >= 319` 匹配 |
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
| `husbandry/obtain_sniffer_egg` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed`；按原版保持 hidden |
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
- `husbandry/obtain_sniffer_egg`
- `husbandry/obtain_netherite_hoe`

## Immediate Alignment Fixes

1. `adventure/kill_all_mobs` 用当前已支持敌对怪集合正式建成原版 ID，而不是 demo
2. `story/mine_stone` 继续评估是否要维持 `destroy_block` 近似，还是改成更贴近原版的获得语义
3. 基于 `inventory_changed` / `consume_item` 继续补其他 husbandry 原版条目（如 `froglights`，在父级与语义明确后）
4. 后续每完成一个 trigger，就回到本文件把对应 `missing-trigger` / `missing-data` 批量改状态
