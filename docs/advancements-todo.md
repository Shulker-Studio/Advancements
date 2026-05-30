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
- `minecraft:entity_hurt_player`
- `minecraft:shot_crossbow`
- `minecraft:target_hit`（当前窄实现：仅 `adventure/bullseye` 已核形状）
- `minecraft:slept_in_bed`
- `minecraft:changed_dimension`
- `minecraft:location`（当前窄实现：结构进入类 location 条件）
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
- `minecraft:item_used_on_block`（当前窄实现：覆盖重生锚、磁石指南针、草甸唱片机、点亮铜灯刮削、蜂蜜采集、铜方块上蜡/脱蜡、告示牌发光等本地 partial slices）
- `minecraft:player_interacted_with_entity`（当前窄实现：仅 `husbandry/leash_all_frog_variants`）
- `minecraft:kill_mob_near_sculk_catalyst`（当前窄实现：基于幽匿催发体消耗死亡经验路径）
- `minecraft:bee_nest_destroyed`（当前窄实现：仅 `husbandry/silk_touch_nest` 已核形状）
- `minecraft:bred_animals`（当前窄实现：支持无条件与 `conditions.child` 的单一 `type` 谓词，仅成功繁殖且可从 `BreedableComponent::mLoveCause` 解析到当前玩家）
- `minecraft:tame_animal`（当前窄实现：支持无条件与猫/狼 `type_specific.variant`，仅 `TameableComponent::tame` 后 `ActorFlags::Tamed` 从 false 变 true）

当前项目已暂时移除/不应继续依赖的临时脚手架：

- `minecraft:tick` 仅作为早期简化 root 使用过，后续应移除并改回原版语义

## Full Java Trigger Inventory

根据 wiki `进度定义格式` 页面目录整理：

| Trigger | Status | Notes |
| --- | --- | --- |
| `allay_drop_item_on_block` | missing-trigger | |
| `any_block_use` | missing-trigger | |
| `avoid_vibration` | missing-trigger | |
| `bee_nest_destroyed` | partial | 已接入窄实现：`BeehiveBlock::playerWillDestroy` 只派发破坏事实；条件匹配支持 `block`、`item.enchantments` 中的 `minecraft:silk_touch`、`num_bees_inside.min`，暂未覆盖完整 Java 物品/玩家谓词；live-server QA 已验证 `minecraft:bee_nest` 且 `bees=3` 时可授予 `husbandry/silk_touch_nest` |
| `bred_animals` | partial | 当前窄实现：成功繁殖后按 `mLoveCause` 归因给玩家；无条件 `husbandry/breed_an_animal` 已通过 live-server QA，另支持 `conditions.child` 下单个 `entity_properties` / `predicate.type` 形状；不支持 `parent`/`partner`/玩家谓词、child 变种/NBT/tags 或完整 entity predicate parity；`child.type` 条件仍需 live-server QA |
| `brewed_potion` | done | 当前窄实现：基于 `ItemStackRequestActionHandler::_handleTransfer`，仅在从 `BrewingStandResultContainer` 成功转移且当前 screen type 为 `ContainerType::BrewingStand` 后触发；不检查输出物品类型，匹配 wiki/原版“从酿造台输出栏获取任意物品”的语义 |
| `changed_dimension` | done | 当前窄实现，支持 `conditions.from` / `conditions.to`；通知时机仍有 caveat |
| `channeled_lightning` | partial | 当前窄实现：基于玩家拥有的弹射物进入 `ProjectileComponent::_handleLightningOnHit` 后派发事件；trigger 层仅接受 `minecraft:thrown_trident` + `mChanneling` + 雷暴天气快照 + 命中位置可见天空 + 被击中实体为 villager/villager_v2 的 `victims` 单实体形状，用于 `adventure/very_very_frightening`；不泛化 lightning 实体谓词、多 victims 或非村民目标 |
| `construct_beacon` | done | 当前窄实现：hook `BeaconBlockActor::checkShape` 关键刷新函数，在当前 `mNumLevels` 从低层级提升为更高层级时触发；仅支持裸条件和 `conditions.level.min`，并按 wiki 语义派发给同维度、信标中心水平切比雪夫距离 10 格内、垂直向下 9 格到向上 5 格内的玩家；实测该关键函数约每 80 tick / 4 秒进入一次，避免直接监听每 tick 热路径；仍需 live-server QA 验证 Bedrock 层级刷新时机与 Java 激活语义严格对齐 |
| `consume_item` | done | 当前窄实现，仅 `conditions.item` |
| `crafter_recipe_crafted` | missing-trigger | |
| `cured_zombie_villager` | partial | 当前窄实现：仅支持无 `conditions` 的 `story/cure_zombie_villager`；基于玩家对僵尸村民成功使用金苹果时记录责任玩家，并在后续 transformation seam 上确认转化；不支持未核实的 `zombie` / `villager` 条件谓词 |
| `default_block_use` | missing-trigger | |
| `effects_changed` | done | 当前窄实现：hook `Actor::addEffect` 后仅对玩家派发，并按当前玩家已拥有的效果快照匹配 `conditions.effects` 中每个空对象 effect；已扩到当前 Bedrock/LL 可见 effect id 集合，并接受 `hero_of_the_village -> village_hero` alias；按当前项目约定，`glowing` / `dolphins_grace` 两个 Java-only 效果不纳入 Bedrock 测试集合 |
| `enchanted_item` | done | 当前窄实现：基于 `CraftHandlerEnchant::_handleEnchant` 成功路径触发，避开通用 UI 转移事件；事件携带附魔物品 id 与等级消耗 |
| `enter_block` | done | 当前窄实现：hook `EndGatewayBlockActor::teleportEntity(Actor&)`，仅当原传送逻辑执行后玩家位置发生变化时派发；仅用于 `end/enter_end_gateway` 的 `{ "block": "minecraft:end_gateway" }` 形状 |
| `entity_hurt_player` | done | 当前仅支持 `story/deflect_arrow` 已核窄形状：`damage.blocked = true` + `damage.type.tags` 含 `minecraft:is_projectile`；runtime 使用 `Player::_blockUsingShield` 成功路径派发 |
| `entity_killed_player` | done | 当前窄实现，支持 `conditions.entity` |
| `fall_after_explosion` | missing-trigger | |
| `fall_from_height` | missing-trigger | |
| `filled_bucket` | done | 当前窄实现，基于 `BucketItem::$_useOn` 对可入桶实体（鱼 / 蝌蚪 / 美西螈）交互成功后的结果桶 item 匹配 |
| `fishing_rod_hooked` | done | 当前窄实现，基于 LSE `FishingHook::_pullCloser` 语义支持钓起的 item |
| `hero_of_the_village` | missing-trigger | |
| `impossible` | missing-trigger | 可后续纯数据实现 |
| `inventory_changed` | partial | 当前实现支持 `item` + `count`，并新增窄形状 `required_items` 供 `husbandry/froglights` 检查“当前物品栏同时拥有多种指定物品”；不泛化槽位、NBT 或更复杂 inventory predicate |
| `item_durability_changed` | missing-trigger | |
| `item_used_on_block` | partial | 当前窄实现覆盖 `nether/charge_respawn_anchor`、`nether/use_lodestone`、`adventure/play_jukebox_in_meadows`、`adventure/lighten_up`、`husbandry/safely_harvest_honey`、`husbandry/wax_on`、`husbandry/wax_off`、`husbandry/make_a_sign_glow` 的本地 shape；trigger 直接监听 LeviLamina `ll::event::player::PlayerInteractBlockEvent`，使用 `EventPriority::Lowest` 让更早的取消逻辑先运行；由于该 LL 事件在 `GameMode::$useItemOn` 原始逻辑前发布，runtime 只在每个 advancement 的窄分支中派发，不做泛化 block-use 派发。 |
| `kill_mob_near_sculk_catalyst` | done | 当前窄实现：hook `SculkCatalystBlockActor::_tryConsumeOnDeathExperience(Level&, Actor&)`，用 `Actor::getLastHurtByPlayer()` 解析 `mLastHurtByPlayerId` 玩家归因，并通过同一调用内的 `SculkSpreader::addCursors(charge > 0)` 与 XP drop 被关闭确认幽匿催发体已消费死亡经验；使用无条件 descriptor，对应原版无 conditions JSON；live-server QA 已验证玩家在幽匿催发体附近击杀 skeleton 可完成，命令击杀未误触发 |
| `killed_by_arrow` | missing-trigger | |
| `levitation` | done | 当前窄实现：基于 `Player::$tickWorld` 观察玩家 `MobEffect::LEVITATION()` 状态，仅支持已核 wiki 语义/本地 JSON 形状 `conditions.distance.y.min = 50.0`；效果开始时记录起始 Y，效果结束时用结束 Y 结算一次绝对垂直位移；仍需 live-server QA 验证潜影贝弹丸给予的 Bedrock levitation 状态与 Java 完成时机严格对齐 |
| `lightning_strike` | missing-trigger | |
| `location` | partial | 当前窄实现：仅支持已核原版 JSON 的 `conditions.player[0].predicate.location.structures` 结构进入条件，覆盖 bastion、fortress、end_city、stronghold、trial_chambers；按 location 语义每 20 tick 轮询，不支持 biome、y-position、维度或通用 location predicate |
| `nether_travel` | done | 当前窄实现：仅支持已核原版 `nether/fast_travel` 形状 `conditions.distance.horizontal.min`；runtime 在玩家从主世界进入下界前记录主世界水平起点，并在从下界返回主世界时按主世界水平距离触发；未泛化 `distance` 其他子键 |
| `placed_block` | missing-trigger | |
| `player_generates_container_loot` | partial | 当前窄实现：基于 `Util::LootTableUtils::fillContainer`，支持玩家作为 loot context entity 生成的四个 bastion chest loot table；另通过 `BrushItem::$_useOn` 调用栈中的当前玩家归因支持 `adventure/salvage_sherd` 的 6 个 archaeology brushable loot table，并要求本次生成物品为 `*_pottery_sherd`；不泛化其他 loot table context、掉落完成时机或完整 Java loot predicate |
| `player_hurt_entity` | done | 当前窄实现：基于 `ll::event::ActorHurtEvent`，仅支持 `damage.type.direct_entity.type = #minecraft:arrows` + `damage.type.tags` 含 `minecraft:is_projectile` 这一已核 condition surface |
| `player_interacted_with_entity` | partial | 当前窄实现：仅支持 `husbandry/leash_all_frog_variants` 已核 shape（玩家成功使用 `minecraft:lead` 与 `minecraft:frog` 交互，并按 `Actor::getVariant()` 的 Bedrock 取值 `0=temperate` / `1=cold` / `2=warm` 匹配三种青蛙变种）；不泛化其他 entity/item/type_specific 条件 |
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
| `tame_animal` | partial | 当前窄实现：hook `TameableComponent::tame(Actor&, Player&)`，仅当 `ActorFlags::Tamed` 从 false 变 true 时派发；支持无 `conditions` 的 `husbandry/tame_an_animal`、猫 `type_specific.variant`（用于 `husbandry/complete_catalogue`）与狼 `type_specific.variant`（用于 `husbandry/whole_pack`），不支持 Java player 谓词或完整 entity predicate parity；仍需 live-server QA |
| `target_hit` | partial | 当前仅支持 `adventure/bullseye` 已核窄形状：`signal_strength = 15` + `projectile[0].condition = minecraft:entity_properties` + `projectile[0].entity = this` + `projectile[0].predicate.distance.horizontal.min = 30.0` |
| `thrown_item_picked_up_by_entity` | missing-trigger | |
| `thrown_item_picked_up_by_player` | missing-trigger | |
| `tick` | missing-trigger | 已不再用于现有 advancement 定义，应继续保持移除状态 |
| `used_ender_eye` | missing-trigger | Java 原版存在该 trigger，但当前项目已不再保留其独立运行时路径；`story/follow_ender_eye` 已回正为 `minecraft:location` + stronghold 结构条件 |
| `used_totem` | done | 当前窄实现，复用 item 条件；Hook `Player::$consumeTotem` 且仅在实际消耗图腾后触发 |
| `using_item` | missing-trigger | |
| `villager_trade` | done | 当前窄实现：基于 `ItemStackRequestActionHandler::_handleTransfer`，仅在从 `CreatedOutputContainer` 成功转移且当前 screen type 为 `ContainerType::Trade` 后触发；不依赖打开交易 UI；条件解析支持空条件 `adventure/trade` 与已核窄形状 `player[0].predicate.minecraft:location.position.y.min`（`adventure/trade_at_world_height`） |
| `voluntary_exile` | done | 已按当前 vanilla JSON 形状接入窄实现：复用 `minecraft:player_killed_entity`，支持 `entity[0].predicate.type = #minecraft:raiders` 且 `equipment.head.items = minecraft:white_banner` 这组条件形状；runtime 基于 `MobDieEvent` / `Actor::getLastHurtByPlayer()` 归因，并用 Bedrock `ActorFlags::IsIllagerCaptain` 判断袭击队长。live-server 正向 QA 已验证队长 pillager 死亡时 `illager_captain=true` 并完成 `adventure/voluntary_exile`；普通 pillager 负例 QA 已验证 `illager_captain=false` 且不授予该进度 |

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
| `nether/netherite_armor` | `inventory_changed` | done | 已补数据，复用 `inventory_changed` 的窄 `required_items` 形状；父级 `minecraft:nether/obtain_ancient_debris`，仅在当前物品栏同时拥有四件下界合金盔甲时完成，奖励 100 XP |
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
| `nether/use_lodestone` | `item_used_on_block` / 磁石指南针窄切片 | done | 已补本地 JSON + lang；criterion `use_lodestone` 使用 `minecraft:item_used_on_block` + `item = minecraft:compass` + `block = minecraft:lodestone`；复用现有 `ItemUsedOnBlockTrigger` 白名单路径，live-server QA 已验证对磁石使用指南针可完成；1.21.4 raw JSON 位于 Nether，1.21.5 wiki 已移动到 Adventure。 |
| `nether/obtain_crying_obsidian` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `nether/charge_respawn_anchor` | `item_used_on_block` / 重生锚直接交互窄切片 | done | 数据已补齐且保持窄实现：criterion `charge_respawn_anchor` 使用 `minecraft:item_used_on_block` + `item = minecraft:glowstone` + `block = minecraft:respawn_anchor`；runtime 现在由 `ItemUsedOnBlockTrigger` 直接监听 LeviLamina `PlayerInteractBlockEvent` / `GameMode::$useItemOn` seam，并以 Lowest 优先级运行；pre-origin caveat：当前在使用前 `RespawnAnchorCharge == 3` 时派发，该条件应对应一次被接受的荧石使用并充至满格。 |

当前总评：多数 `nether/*` 仍是 `missing-trigger`；纯“获得某物”型条目已有一批通过 `inventory_changed` 补齐，包含 `obtain_crying_obsidian`；`return_to_sender` 已作为 `player_killed_entity` 的恶魂火球窄切片补齐；`fast_travel` 已作为 `nether_travel` 窄切片补齐；`summon_wither` 已作为 `summoned_entity` 凋灵窄切片补齐；`charge_respawn_anchor` 已作为 `item_used_on_block` / `_bumpCharge` 满充能窄切片补齐；`use_lodestone` 已作为 `item_used_on_block` 的 compass + lodestone 窄切片补齐并通过 live-server QA；`all_potions` 已作为 `effects_changed` 的 17 效果快照窄切片补齐；`all_effects` 已按当前项目定义作为“排除 `glowing` / `dolphins_grace` 的 Bedrock 子集”补齐。

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
| `adventure/voluntary_exile` | `player_killed_entity` / raid captain narrow slice | done | 已补数据并接入窄实现：当前 raw vanilla JSON 使用 `minecraft:player_killed_entity`，条件为 `#minecraft:raiders` + 头部 `minecraft:white_banner`；本地 runtime 将该条件映射到被杀实体属于当前支持的 raider 类型且 `ActorFlags::IsIllagerCaptain` 为真。live-server QA 已通过：队长 pillager 日志显示 `illager_captain=true` 后授予 `minecraft:adventure/voluntary_exile`；普通 pillager 日志显示 `illager_captain=false` 且仅授予 kill/root 相关进度，不授予 `voluntary_exile` |
| `adventure/spyglass_at_parrot` | other | missing-trigger | |
| `adventure/spyglass_at_ghast` | other | missing-trigger | |
| `adventure/spyglass_at_dragon` | other | missing-trigger | |
| `adventure/throw_trident` | `player_hurt_entity` | done | 已补本地 JSON + lang，并接入窄实现：按已核本地运行时形状复用 `minecraft:player_hurt_entity`，仅支持 `damage.type.direct_entity.type = minecraft:thrown_trident` 与 `damage.type.tags` 含 `minecraft:is_projectile` 这组条件，不泛化其他投射物或近战三叉戟伤害形状 |
| `adventure/very_very_frightening` | `channeled_lightning` | done | 已补数据并接入当前窄实现：仅支持 `victims[0].predicate.type = minecraft:villager`，runtime 要求玩家拥有的引雷三叉戟在雷暴且命中位置可见天空时实际进入 lightning-on-hit 处理；live-server 正向 QA 已通过 |
| `adventure/kill_mob_near_sculk_catalyst` | `kill_mob_near_sculk_catalyst` | done | 已按原版 1.21.3 无 conditions 形状补数据并接入当前催发体 XP 消耗窄实现；live-server QA 已验证玩家在幽匿催发体附近击杀 skeleton 可完成，命令击杀未误触发 |
| `adventure/shoot_arrow` | `player_hurt_entity` | done | 已补数据并接入窄实现：仅支持 `damage.type.direct_entity.type = #minecraft:arrows` 与 `damage.type.tags` 含 `minecraft:is_projectile` 这组已核 surface；保持非泛化 |
| `adventure/sniper_duel` | `player_killed_entity` | done | 已补本地 JSON + lang，并接入已核窄实现：仅支持 skeleton + 水平距离 `>= 50.0` + projectile killing_blow tags；保持非泛化 |
| `adventure/bullseye` | `target_hit` | done | 已落地本地 JSON + lang；runtime/条件解析仅支持该行已核窄形状（`signal_strength = 15` 与 projectile 水平距离 `>= 30`） |
| `adventure/kill_all_mobs` | `player_killed_entity` | done | 已补齐当前本地 hostile 集合窄切片，继续复用现有 `player_killed_entity` 简单实体匹配；本波次补入 `bogged`、`breeze`、`creaking`、`evoker`、`wither`、`zoglin` 六项，并与本地历史已跑通清单对齐 |
| `adventure/totem_of_undying` | `used_totem` | done | 已补数据，复用现有 `used_totem` |
| `adventure/ol_betsy` | `shot_crossbow` | done | 已补数据并接入窄实现：仅支持 `conditions.item.items = minecraft:crossbow` 这组已核 shape，复用当前 `shot_crossbow` 窄触发 |
| `adventure/trade` | `villager_trade` | done | 已补数据，复用当前完成交易语义的 `villager_trade` |
| `adventure/trade_at_world_height` | `villager_trade` | done | 已按原版 raw JSON 形状补数据：父级 `minecraft:adventure/trade`，`minecraft:villager_trade` + `player[0].predicate.location.position.y.min = 319.0`；当前窄实现按玩家交易完成时脚部位置 `Y >= 319` 匹配 |
| `adventure/salvage_sherd` | `player_generates_container_loot` | partial | 已补本地 JSON + lang；runtime 在 `BrushItem::$_useOn` 调用栈内捕获 `Util::LootTableUtils::fillContainer` 的 brushable archaeology loot，按当前玩家归因，并在 generated item 为 `*_pottery_sherd` 时匹配对应原版 `minecraft:archaeology/*` criterion；不再额外依赖 `inventory_changed`，触发时机为内部 brushable loot 生成而非最终掉落/拾取；live-server QA 已验证空 brushable block 与非陶片考古产物不会完成，刷出 `minecraft:archer_pottery_sherd` 时在后续背包变更前完成 |
| `adventure/sleep_in_bed` | `slept_in_bed` | done | 已补数据，复用现有 `slept_in_bed` |
| `adventure/adventuring_time` | `location` | missing-trigger | |
| `adventure/play_jukebox_in_meadows` | `item_used_on_block` | partial | 已补本地 JSON + lang；runtime 在玩家用唱片对空唱片机交互且唱片机位于 meadow biome 时派发，支持本地列出的 music disc ID；pre-origin caveat：仍需 live-server QA 验证唱片机成功插入/播放时机与所有 Bedrock 唱片 ID。 |
| `adventure/fall_from_world_height` | `fall_from_height` | missing-trigger | |
| `adventure/trim_with_any_armor_pattern` | other | missing-trigger | |
| `adventure/read_power_of_chiseled_bookshelf` | other | missing-trigger | |
| `adventure/craft_decorated_pot_using_only_sherds` | other | missing-trigger | |
| `adventure/caves_and_cliffs` | `fall_from_height` / location family | missing-trigger | |
| `adventure/trim_with_all_exclusive_armor_patterns` | other | missing-trigger | |
| `adventure/who_needs_rockets` | other | missing-trigger | |
| `adventure/minecraft_trials_edition` | `location` / structure entry family | done | 已核原版 JSON：`minecraft:location` + `player[0].predicate.location.structures = minecraft:trial_chambers`；当前窄实现基于玩家所在 Trial Chambers 结构触发，按 location 语义每 20 tick 轮询 |
| `adventure/lighten_up` | `item_used_on_block` | partial | 已补本地 JSON + lang；runtime 支持任意列出的斧对点亮的 exposed/weathered/oxidized copper bulb 使用；不覆盖完整 Java location predicate，涂蜡铜灯脱蜡相关 MC-269636 行为仍需 live-server QA。 |
| `adventure/blowback` | other | missing-trigger | |
| `adventure/under_lock_and_key` | other | missing-trigger | |
| `adventure/revaulting` | other | missing-trigger | |
| `adventure/overoverkill` | `player_hurt_entity` / damage family | done | 已补本地 JSON + lang，并接入窄实现：仅支持父级 `minecraft:adventure/minecraft_trials_edition`、`minecraft:player_hurt_entity` 下 `damage.dealt.min = 100.0` 与 `damage.type.tags = [{ id: minecraft:mace_smash, expected: true }]`、`direct_entity.equipment.mainhand.items = minecraft:mace` 这组已核本地 shape；runtime 以玩家当前主手为 `minecraft:mace` 且本次 `ActorHurtEvent.damage() >= 100.0` 的单击伤害作为最小匹配面，不泛化其他 damage predicate 形状 |
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
| `husbandry/breed_an_animal` | `bred_animals` | done | 已补本地 JSON + lang；无条件 criteria 已通过 live-server QA 并可授予 `lwenk`。Trigger family 另支持窄 `conditions.child.predicate.type`，但该条件形状及 parent/partner/player predicates、child 变种/NBT/tags/full Java parity 仍未覆盖 |
| `husbandry/bred_all_animals` | `bred_animals` | done | 已按原版 JSON 补齐 25 个 `conditions.child.predicate.type` criterion，复用当前 `bred_animals` child-type 窄实现；仍需 live-server QA 验证各 Bedrock 子代 type id 与 Java criterion 列表一致 |
| `husbandry/balanced_diet` | `consume_item` | partial | 已补原版 ID，但当前仅实现已支持 consumable 子集，不是原版完整食物集合 |
| `husbandry/obtain_netherite_hoe` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed` |
| `husbandry/tame_an_animal` | `tame_animal` | done | 已补本地 JSON + lang，并接入窄实现：仅在 `TameableComponent::tame` 使目标 `ActorFlags::Tamed` 从 false 变 true 时授予；当前 trigger family 另支持猫/狼 variant 条件，不支持 Java player 谓词或完整 entity predicate parity；仍需 live-server QA |
| `husbandry/fishy_business` | `fishing_rod_hooked` | done | 原版 JSON 已恢复，按钓起的四种鱼 item 匹配 |
| `husbandry/tactical_fishing` | `filled_bucket` | done | 原版 JSON 已恢复，按填充后的四种鱼桶 item 匹配 |
| `husbandry/axolotl_in_a_bucket` | `filled_bucket` | done | 已按填充后的美西螈桶 item 匹配 |
| `husbandry/kill_axolotl_target` | other | missing-trigger | |
| `husbandry/complete_catalogue` | `tame_animal` | done | 已按原版 JSON 补齐 11 个猫 `type_specific.variant` criterion；当前 Bedrock 映射基于 `Actor::getVariant()` 的猫 variant 值，仍需 live-server QA |
| `husbandry/safely_harvest_honey` | `item_used_on_block` | partial | 已补本地 JSON + lang；runtime 为确认玻璃瓶本次能实际采到蜂蜜，要求 bee_nest/beehive 使用前 `honey_level >= 5`，并在 hive/nest 下方 3x3、向下 1-2 格范围内扫描 campfire/soul_campfire；若营火暴露 `Extinguished` state 则排除已熄灭营火，若该 state 不存在则按扫描到营火处理。该 smoke-safety 仍是窄形状，不覆盖完整 Java `location.smokey=true` 搜索范围，需 live-server QA 后再考虑 done。 |
| `husbandry/wax_on` | `item_used_on_block` | partial | 已补本地 JSON + lang；runtime 支持 honeycomb 对本地列出的可上蜡铜方块使用；依赖 pre-origin block ID 白名单，需 live-server QA 验证 Bedrock 方块 ID 和成功上蜡时机。 |
| `husbandry/wax_off` | `item_used_on_block` | partial | 已补本地 JSON + lang；runtime 支持任意列出的斧对本地列出的已涂蜡铜方块使用；需 live-server QA 验证 Bedrock 方块 ID 和成功脱蜡时机。 |
| `husbandry/tadpole_in_a_bucket` | `filled_bucket` | done | 已按填充后的蝌蚪桶 item 匹配 |
| `husbandry/leash_all_frog_variants` | `player_interacted_with_entity` | done | 已补本地 JSON + lang，并接入窄实现：仅支持玩家成功用 `minecraft:lead` 拴住 `minecraft:frog`，且按当前 Bedrock `variant` 值映射 `minecraft:temperate`/`minecraft:cold`/`minecraft:warm` 三种 criterion；青蛙不需要同时被拴住 |
| `husbandry/froglights` | `inventory_changed` | done | 已补数据，复用 `inventory_changed` 的窄 `required_items` 形状；父级 `minecraft:husbandry/leash_all_frog_variants`，仅在当前物品栏同时拥有 `pearlescent_froglight`、`verdant_froglight`、`ochre_froglight` 时完成 |
| `husbandry/silk_touch_nest` | `bee_nest_destroyed` | done | 已补数据并接入窄实现：事件源只派发蜂巢/蜂箱破坏事实，trigger 层从玩家当前手持物品匹配精准采集条件，并要求 `minecraft:bee_nest`、`num_bees_inside.min = 3`；live-server QA 已验证空蜂巢 `bees=0` 不完成、含 3 只蜜蜂的蜂巢 `bees=3` 可授予进度 |
| `husbandry/ride_a_boat_with_a_goat` | other | missing-trigger | |
| `husbandry/make_a_sign_glow` | `item_used_on_block` | partial | 已补本地 JSON + lang；runtime 不再调用 `SignBlock::_getInteractResult`，改为只读检查 glow ink sac、告示牌/悬挂告示牌 block actor、未打蜡、玩家朝向侧文本存在且尚未发光；数据条件已包含实服观测到的 legacy `minecraft:standing_sign` / `minecraft:wall_sign` 以及本地木种 sign/hanging sign ID，仍需 live-server QA 覆盖更多 sign block ID。 |
| `husbandry/allay_deliver_item_to_player` | other | missing-trigger | |
| `husbandry/allay_deliver_cake_to_note_block` | `allay_drop_item_on_block` | missing-trigger | |
| `husbandry/obtain_sniffer_egg` | `inventory_changed` | done | 已补数据，复用现有 `inventory_changed`；按原版保持 hidden |
| `husbandry/feed_snifflet` | other | missing-trigger | |
| `husbandry/plant_any_sniffer_seed` | other | missing-trigger | |
| `husbandry/remove_wolf_armor` | other | missing-trigger | |
| `husbandry/repair_wolf_armor` | other | missing-trigger | |
| `husbandry/whole_pack` | `tame_animal` | done | 已按原版 JSON 补齐 9 个狼 `type_specific.variant` criterion；Bedrock 数字 ID 映射采用已核实的 `0=pale`、`1=ashen`、`2=black`、`3=chestnut`、`4=rusty`、`5=snowy`、`6=spotted`、`7=striped`、`8=woods`，仍需 live-server QA 覆盖更多狼外观 |

补充：当前已保留的 husbandry 定义：

- `husbandry/root`
- `husbandry/fishy_business`
- `husbandry/tactical_fishing`
- `husbandry/tadpole_in_a_bucket`
- `husbandry/leash_all_frog_variants`
- `husbandry/froglights`
- `husbandry/axolotl_in_a_bucket`
- `husbandry/balanced_diet`
- `husbandry/obtain_sniffer_egg`
- `husbandry/obtain_netherite_hoe`

## Current Alignment Notes

1. `story/mine_stone` 已按获得 `cobblestone / blackstone / cobbled_deepslate` 的原版核心语义回正，不再作为 `destroy_block` 近似项排队。
2. `adventure/very_very_frightening` 与 `adventure/kill_mob_near_sculk_catalyst` 已补数据并接入窄 runtime slice；后续不要再把它们列入 seam research 队列，除非有新的 Bedrock 行为证据需要修正。
3. `husbandry/silk_touch_nest`、`husbandry/breed_an_animal`、`husbandry/bred_all_animals`、`husbandry/tame_an_animal`、`husbandry/complete_catalogue`、`husbandry/whole_pack` 已有数据与窄实现；对应 trigger family 仍按上方备注保留 Java parity caveat。
4. `husbandry/balanced_diet` 当前仍保持 `partial`：本地已有原版 ID 数据，但食物集合与 Bedrock consume runtime 覆盖仍需逐项核对后才能标为 `done`。
5. 后续每完成一个 trigger，就回到本文件把对应 `missing-trigger` / `missing-data` / `partial` 状态和 caveat 同步更新。
