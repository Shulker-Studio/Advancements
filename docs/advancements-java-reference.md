# Java版进度与触发器对齐参考

本文件用于记录本项目后续实现 advancement / trigger 时，应优先对齐的 Java 版语义、结构和命名约定。

## 主要资料来源

- `https://zh.minecraft.wiki/w/进度`
- `https://zh.minecraft.wiki/w/进度定义格式`
- `https://zh.minecraft.wiki/w/注册表`
- `https://zh.minecraft.wiki/w/Tutorial:进度指南/冒险`
- `https://zh.minecraft.wiki/w/Java版1.19`
- `https://zh.minecraft.wiki/w/Java版1.20.5/开发版本`

说明：

- `进度` 页用于看原版进度系统总览、五个标签、原版条目与命名空间 ID。
- `进度定义格式` 是最关键页面，后续实现优先对齐它描述的 JSON 结构、准则与触发器语义。
- `注册表` 页用于确认 `ADVANCEMENT` / `TRIGGER_TYPE` 的数据包注册表语义。
- `教程:进度指南/冒险` 用于补原版具体进度树、分组方式与命名/本地化约定。
- 版本页用于补 trigger 增删与字段演进历史。

## Java版进度系统总览

根据 `进度` 页：

- Java 版进度是分标签的多棵树结构。
- 当前原版共有 5 个标签：
  - `story` / Minecraft
  - `nether`
  - `end`
  - `adventure`
  - `husbandry`
- 页面列出的当前总数为 125：
  - Minecraft 16
  - 下界 23
  - 末地 9
  - 冒险 47
  - 农牧业 30

对项目的直接启发：

- 后续不要只把 advancement 当成“成就条目集合”，而应当保留“标签树 + 父子关系 + 展示层级”的思路。
- 我们的数据模型与运行时都应允许：
  - root advancement
  - parent linkage
  - 不同标签/树
  - task / goal / challenge 框架差异

## Java版 JSON 结构基线

后续实现优先对齐 `进度定义格式` 中描述的这些核心字段：

- `parent`
- `display`
- `criteria`
- `requirements`
- `rewards`

### `display` 相关

Java 版 `display` 是展示语义的重要组成部分，至少应按下面思路对齐：

- `title`
- `description`
- `icon`
- `frame`
- `show_toast`
- `announce_to_chat`
- `hidden`

根进度还可带背景图等标签页展示信息。

### `criteria` 与 `requirements`

Java 版不是“一个进度只能有一个触发器”。

- `criteria` 是具名准则集合。
- 每个 criterion 绑定一个 trigger 和对应条件。
- `requirements` 决定这些 criterion 的组合关系。

应按 Java 语义理解 `requirements`：

- 外层列表中的每一项都必须满足。
- 每个子列表中满足任意一个 criterion 即可。
- 因而可以表达：
  - 全部都要完成
  - 多选一
  - 多组并列条件

这意味着后续项目实现不能长期停留在“单 criterion = 单 advancement 完成”的简化模型上。

### `rewards`

Java 版 `rewards` 至少包含：

- `experience`
- `loot`
- `recipes`
- `function`

当前项目阶段虽然暂不做 rewards，但后续字段设计应保留兼容空间，不要把结构写死成“只有经验或没有奖励”。

## Java版触发器基线

`进度定义格式` 页给出了完整的可用准则触发器列表。当前检索确认至少包含以下代表性 trigger：

- `minecraft:tick`
- `minecraft:inventory_changed`
- `minecraft:consume_item`
- `minecraft:placed_block`
- `minecraft:item_used_on_block`
- `minecraft:player_killed_entity`
- `minecraft:entity_killed_player`
- `minecraft:player_hurt_entity`
- `minecraft:target_hit`
- `minecraft:villager_trade`
- `minecraft:recipe_crafted`
- `minecraft:recipe_unlocked`
- `minecraft:impossible`
- `minecraft:location`
- `minecraft:nether_travel`
- `minecraft:changed_dimension`
- `minecraft:enter_block`
- `minecraft:used_totem`
- `minecraft:used_ender_eye`
- `minecraft:killed_by_arrow`
- `minecraft:shot_crossbow`
- `minecraft:spear_mobs`
- `minecraft:tame_animal`
- `minecraft:bred_animals`
- `minecraft:hero_of_the_village`
- `minecraft:voluntary_exile`
- `minecraft:allay_drop_item_on_block`
- `minecraft:avoid_vibration`
- `minecraft:kill_mob_near_sculk_catalyst`
- `minecraft:thrown_item_picked_up_by_player`
- `minecraft:crafter_recipe_crafted`
- `minecraft:fall_after_explosion`
- `minecraft:any_block_use`
- `minecraft:default_block_use`

说明：完整字段说明仍应以后续实现时逐项回查 `进度定义格式` 页面对应小节为准。

## 代表性原版进度与触发器映射

以下映射用于指导我们优先实现哪些 trigger，以及这些 trigger 在原版中的定位。

### 1. 根进度

Java 原版根进度普遍依赖极轻量的“进入游戏后自然成立”的 trigger。

- `story/root` -> `minecraft:tick`

这也是当前本项目本地样例已采用的结构。

### 2. 方块破坏类

原版故事线中的“石器时代”本质上是“挖到石头后解锁”的进度。

- Java 原版对应的是“获得/挖掘相关语义”
- 当前项目样例为：
  - `story/mine_stone`
  - trigger: `bedrock:player_destroy_block`
  - condition: `{ "block": "minecraft:stone" }`

这说明在基岩/LeviLamina 环境里，我们可以接受“以方块破坏事件去近似 Java 行为”，但命名、显示字段和条件结构尽量继续贴近 Java 风格。

### 3. 战斗类

从 `教程:进度指南/冒险` 与 `进度` 页可提炼出一批高优先级战斗 trigger：

- `player_killed_entity`
  - 例：怪物猎人、资深怪物猎人一类
- `player_hurt_entity`
  - 例：部分命中/伤害类条件
- `killed_by_arrow`
  - 例：劲弩手、狙击/穿透相关
- `target_hit`
  - 例：正中靶心
- `spear_mobs`
  - 例：生物串串香

### 4. 交易 / 合成 / 物品使用类

- `villager_trade`
  - 例：成交！、星际商人
- `recipe_crafted`
  - 用于常规合成类进度
- `crafter_recipe_crafted`
  - 用于合成器相关新进度
- `consume_item`
  - 用于消耗食物/药水等
- `inventory_changed`
  - 用于“获得某物”这类最常见的 story / husbandry advancement

### 5. 探索 / 移动 / 维度类

- `location`
- `changed_dimension`
- `nether_travel`
- `used_ender_eye`
- `enter_block`

这类 trigger 很多是 story / nether / end 的骨架。

## 版本演进中值得注意的点

根据 `Java版1.19` 与 `Java版1.20.5/开发版本`：

- 1.19 新增过一批与深暗之域/悦灵相关的 trigger：
  - `kill_mob_near_sculk_catalyst`
  - `allay_drop_item_on_block`
  - `avoid_vibration`
  - `thrown_item_picked_up_by_player`
- 1.20.5 / 1.21 开发周期新增：
  - `crafter_recipe_crafted`
  - `fall_after_explosion`
  - `any_block_use`
  - `default_block_use`
- 1.19 页面还提到：
  - 某些 trigger 旧有的 `location` 字段被移除，因为它与 `player.location` 重复。

对项目的启发：

- 后续设计 trigger 条件时，应注意 Java trigger 字段并非静态不变。
- 若未来补更多 trigger，应优先先查 wiki 对应 trigger 小节，而不是凭直觉设计字段名。

## 本项目当前已对齐与未对齐项

### 已对齐

当前本地样例已具备以下 Java 风格字段：

- `parent`
- `display.title`
- `display.description`
- `display.icon`
- `display.frame`
- `display.show_toast`
- `display.announce_to_chat`
- `display.hidden`
- `criteria`
- `requirements`
- `rewards`

并且：

- `story/root.json` 已使用 `minecraft:tick`
- `story/mine_stone.json` 已使用 block 条件化的 destroy-block trigger

### 仍未完全对齐

当前运行时仍只是 MVP，和 Java 完整语义相比还有明显差距：

- `requirements` 组合语义尚未完整实现
- 多 criterion advancement 的完整达成逻辑尚未完整实现
- `rewards.function / loot / recipes / experience` 尚未接入运行时
- 原版大多数 trigger 还没有底层事件源与标准层适配
- 原版“获得物品”“交易”“击杀实体”“命中目标”“维度切换”等主干 trigger 尚未系统实现

## 后续实现优先级建议

为了“尽可能按照 Java 的来”，建议优先级如下：

1. 保持标准层继续围绕 `TriggerDispatcher / TriggerIndex`，不要直接把 LL 事件语义泄露到 advancement 层。
2. 先补 Java 原版最核心、覆盖面最高的 trigger：
   - `minecraft:tick`
   - `inventory_changed`
   - `player_killed_entity`
   - `villager_trade`
   - `consume_item`
   - `placed_block` / `item_used_on_block`
   - `changed_dimension`
3. 在 trigger 逐步增加前，先把 `requirements` 语义补到足够接近 Java。
4. 所有新增 trigger 的命名、条件字段、典型样例都优先对齐 `进度定义格式` 页面，而不是自创。
5. 若基岩 / LL 环境无法精确复刻 Java 行为，允许在“底层事件源不同”的前提下保留 Java 风格的 advancement 定义与条件结构。

## 本项目执行原则

后续实现时遵循以下原则：

1. **定义层尽量像 Java**
   - advancement JSON 字段名
   - trigger 命名
   - condition 结构
   - parent / display / rewards / requirements 语义

2. **底层事件源按 LeviLamina 实际能力适配**
   - 优先官方 LL 事件
   - 无官方事件时再评估 hook
   - 但 LL 事件 / hook 只作为底层 source，不直接污染标准层

3. **玩家身份继续使用 `mce::UUID`**

4. **当前阶段不为了追求“全覆盖”牺牲结构正确性**
   - 宁可少做几个 trigger
   - 也不要把 Java 结构做歪

## 与当前样例的直接对应

当前仓库样例可视为以下 Java 对齐路径的起点：

- `story/root.json`
  - Java 对齐目标：根 advancement + `minecraft:tick`
- `story/mine_stone.json`
  - Java 对齐目标：故事线中的早期资源获取/方块交互类 advancement
  - 当前基岩侧近似实现：`bedrock:player_destroy_block` + `block` 条件

后续新增示例建议继续按 Java 原版标签推进：

- `story/*`：先补故事线骨架
- `adventure/*`：再补击杀、命中、交易
- `husbandry/*`：再补消耗、繁殖、驯服

这样更容易保持整个项目的数据与运行时都沿着 Java 原版语义生长。
