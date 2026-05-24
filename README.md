# Advancements

一个面向 LeviLamina 的 Java 风格进度（Advancements）插件。

当前版本：`0.1.0`

## 项目状态

> 当前仍在开发中（WIP / Work in Progress）。

`0.1.0` 是首个可发布版本，已经具备基础可玩性，但距离完整覆盖 Java 原版进度系统仍有不少工作要做。

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
- `minecraft:used_totem`
- `minecraft:filled_bucket`
- `minecraft:fishing_rod_hooked`
- `minecraft:player_killed_entity`
- `minecraft:entity_killed_player`
- `minecraft:slept_in_bed`
- `minecraft:changed_dimension`
- `minecraft:location`（当前为窄实现，仅结构进入类）
- `minecraft:player_generates_container_loot`（当前为窄实现，仅 `nether/loot_bastion`）
- `minecraft:villager_trade`
- `minecraft:enchanted_item`
- `bedrock:player_destroy_block`

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
lip install github.com/Shulker-Studio/Advancements@0.1.0
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
│       ├── MyMod.cpp
│       └── MyMod.h
├── tooth.json
├── xmake.lua
└── README.md
```

各部分作用：

- `data/`：内置 advancement 定义
- `lang/`：本地化文本
- `src/mod/advancement/`：加载、存储、触发分发、GUI、通知
- `src/mod/commands/`：命令注册与命令执行逻辑
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
- 插件停用时会尝试 flush 脏数据

## 更新日志

完整版本更新记录见 [CHANGELOG.md](CHANGELOG.md)。
