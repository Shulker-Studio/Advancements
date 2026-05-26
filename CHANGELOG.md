# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.1]

### Added

- 补充并接入一批新的 advancement 数据、文案与窄实现触发，包括：
  - `story/cure_zombie_villager`
  - `story/deflect_arrow`
  - `adventure/shoot_arrow`
  - `adventure/bullseye`
  - `adventure/ol_betsy`
  - `adventure/sniper_duel`
  - `adventure/kill_all_mobs`
  - `adventure/trade_at_world_height`
  - `nether/brew_potion`
  - `nether/charge_respawn_anchor`
  - `nether/create_beacon`
  - `nether/create_full_beacon`
  - `nether/fast_travel`
  - `nether/all_potions`
  - `nether/all_effects`
  - `nether/return_to_sender`
  - `nether/summon_wither`
  - `end/enter_end_gateway`
  - `end/levitate`
  - `end/respawn_dragon`
- 新增 `ProgressLifecycle`，在玩家退出时保存进度数据，降低服务端停用前才统一 flush 的风险。

### Fixed

- 修复空条件 criterion 在 descriptor 路径下无法正确命中的问题，恢复部分 root、parent 与普通触发进度的正常授予。
- 修复 `minecraft:shot_crossbow` 触发载荷，恢复 `ol_betsy` 一类弩相关进度的正常触发。
- 修复 `minecraft:used_totem` 触发载荷，恢复 `totem_of_undying` / “超越生死” 的正常触发。
- 修复 `inventory_changed` 相关触发 ID 回归问题。
- 放宽并校正部分弹射物 / projectile 伤害来源判定，使 `deflect_arrow`、`sniper_duel`、`return_to_sender` 等相关进度更接近原版语义。

### Changed

- 触发器核心迁移到 descriptor / criteria 模型，整理为更明确的 `trigger/` 与 `criteria/` 结构，并完成剩余触发器族迁移。
- 整理源码命名与入口结构，将旧的 `MyMod` 命名收敛到 `Entry` 与 `advancements` 命名体系。
- 重构进度读写路径，补充只读视图接口，移除不必要的复制与 `const` 写语义，并拆分进度保存生命周期模块。
- 调整进度 GUI 一级分类顺序为：故事、下界、末地、冒险、农业。
- 调整进度 GUI 按钮文案为两行展示，一级菜单显示分类名与完成度，二级菜单显示进度标题与完成状态 / 完成度。
- 将中文环境下故事根分类标题从 `Minecraft` 调整为 `主线`。
- 更新并补充多批原版进度数据、翻译与状态文档，使 README / TODO / 本地 wiki 参考更接近当前实现状态。

## [0.1.0]

### Added

- 基础 advancement 数据加载与本地化。
- 进度 GUI。
- `advancement` 命令面（查看、GUI、grant、revoke）。
- 一批核心 runtime trigger 支持。
- 结构进入类进度支持：Stronghold、Bastion Remnant、Nether Fortress、End City、Trial Chambers。
- `nether/loot_bastion` 的战利品箱触发支持。
- 进度完成 toast 与聊天广播样式和颜色区分。
