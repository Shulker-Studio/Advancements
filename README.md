# Advancements

一个面向 LeviLamina 的 Java 风格进度（Advancements）插件。

当前版本：`0.1.2`

## 项目状态

> 当前仍在开发中（WIP / Work in Progress）。

`0.1.2` 在 `0.1.1` 的基础上继续扩展进度数据与窄实现 trigger，并将运行时触发路径进一步收敛到事件源 + trigger registry 架构。当前版本新增三叉戟、超越极限、下界合金盔甲、蛙明灯、拴住所有青蛙变种等进度，已经更适合继续分发测试与日常游玩使用，但距离完整覆盖 Java 原版进度系统仍有不少工作要做。

## 关于 GPT Vibe Coding

本插件和配套文档在开发过程中使用了 ChatGPT 辅助的 vibe coding 工作流。

这意味着：

- 需求拆分、实现草拟、文档整理中使用了 AI 辅助；
- 最终行为以当前仓库中的源代码、数据文件和本地验证结果为准；
- 发布前的改动都经过了本地编译和功能核对，而不是直接使用生成结果。

## 功能概览

当前版本已经包含：

- Java 风格 advancement 数据加载
- 中英文本地化（`en_US` / `zh_CN`）
- 进度 GUI
- 管理 / 测试命令
- 一批基础 runtime trigger 支持
- 进度完成 toast 与聊天广播

当前已经支持的主要 trigger 包括：

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
- `minecraft:target_hit`（当前为窄实现，主要覆盖 `adventure/bullseye`）
- `minecraft:slept_in_bed`
- `minecraft:changed_dimension`
- `minecraft:location`（当前为窄实现，主要覆盖结构进入类）
- `minecraft:player_generates_container_loot`（当前为窄实现，仅 `nether/loot_bastion`）
- `minecraft:nether_travel`
- `minecraft:summoned_entity`（当前为窄实现，仅 `nether/summon_wither` / `end/respawn_dragon`）
- `minecraft:levitation`（当前为窄实现，仅 `end/levitate` 的垂直位移条件）
- `minecraft:effects_changed`（当前为窄实现，仅 `nether/all_potions` 的 required effects 快照）
- `minecraft:brewed_potion`（当前为窄实现，仅从酿造台输出槽成功取物）
- `minecraft:construct_beacon`（当前为窄实现，仅 `nether/create_beacon` / `nether/create_full_beacon`）
- `minecraft:enter_block`（当前为窄实现，仅 `end/enter_end_gateway`）
- `minecraft:item_used_on_block`（当前为窄实现，仅 `nether/charge_respawn_anchor`）
- `minecraft:player_interacted_with_entity`（当前为窄实现，仅 `husbandry/leash_all_frog_variants`）
- `minecraft:villager_trade`
- `minecraft:enchanted_item`
- `minecraft:channeled_lightning`（当前为窄实现，仅 `adventure/very_very_frightening`）
- `minecraft:kill_mob_near_sculk_catalyst`（当前为窄实现，基于幽匿催发体消耗死亡经验路径）
- `minecraft:bee_nest_destroyed`（当前为窄实现，仅 `husbandry/silk_touch_nest`）
- `minecraft:bred_animals`（当前为窄实现，支持无条件与 `conditions.child` 的单一 `type` 谓词；仍依赖成功繁殖且可从 `mLoveCause` 解析到当前玩家）
- `minecraft:tame_animal`（当前为窄实现，仅驯服状态从未驯服变为已驯服）

## 安装

### 方式一：手动安装

1. 下载 release 里的 `Advancements-server-windows-x64.zip`
2. 解压后得到 `Advancements/` 目录
3. 将整个 `Advancements/` 目录放入服务器的 `plugins/` 目录下
4. 重启 LeviLamina 服务端

目标结构应类似：

```text
plugins/
└── Advancements/
    ├── Advancements.dll
    ├── manifest.json
    ├── data/
    └── lang/
```

### 方式二：使用 lip 安装

如果你已经通过 release + `tooth.json` 发布，可以使用：

```bash
lip install github.com/Shulker-Studio/Advancements@0.1.2
```

安装目标目录为：

```text
plugins/Advancements/
```

## 使用方法

### 玩家命令

打开进度 GUI：

```text
/advancement gui
```

### 管理员命令

重新加载进度定义：

```text
/advancement reload
```

授予 / 撤销进度：

```text
/advancement grant <target> everything
/advancement grant <target> only <advancement> [criterion]
/advancement grant <target> from <advancement>
/advancement grant <target> through <advancement>
/advancement grant <target> until <advancement>

/advancement revoke <target> everything
/advancement revoke <target> only <advancement> [criterion]
/advancement revoke <target> from <advancement>
/advancement revoke <target> through <advancement>
/advancement revoke <target> until <advancement>
```

说明：

- `everything`：所有已加载进度
- `only`：单个进度，或单个 criterion
- `from`：指定进度及其所有子进度
- `until`：从根到指定进度的整条父链
- `through`：`until + from`

## 插件目录结构

当前仓库的核心结构大致如下：

```text
Advancements/
├── data/
│   └── minecraft/advancements/
├── lang/
│   ├── en_US.json
│   └── zh_CN.json
├── src/
│   └── mod/
│       ├── advancement/
│       ├── commands/
│       ├── event/
│       ├── gui/
│       ├── predicate/
│       ├── trigger/
│       ├── Entry.cpp
│       └── Entry.h
├── tooth.json
├── xmake.lua
└── README.md
```

各部分作用：

- `data/`：内置 advancement 定义
- `lang/`：本地化文本
- `src/mod/advancement/`：加载、解析、进度存储、进度服务与通知
- `src/mod/commands/`：命令注册与命令执行逻辑
- `src/mod/event/`：Bedrock / LeviLamina runtime 事件源适配
- `src/mod/gui/`：进度 GUI 与 GUI 索引
- `src/mod/predicate/`：条件谓词解析与匹配辅助
- `src/mod/trigger/`：trigger registry、criteria 与触发器实现
- `tooth.json`：LIP / release 安装元数据
- `xmake.lua`：构建配置

## 数据保存位置

玩家进度保存在世界目录下：

```text
<worldDataDir>/advancements/<player-uuid>.json
```

例如：

```text
worlds/Bedrock level/ll-data/Advancements/advancements/xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx.json
```

也就是说：

- 数据按世界保存
- 数据按玩家 UUID 分文件保存
- 玩家退出和插件停用时会尝试 flush 脏数据

## 更新日志

完整版本更新记录见 [CHANGELOG.md](CHANGELOG.md)。
