# Phase 0 Projectile Wave Baseline

Verified against raw vanilla JSON from MCAsset Cloud (`1.21.5-pre2`) for the five rows in `.sisyphus/plans/advancements-projectile-trigger-wave.md` Phase 0.

| Vanilla ID | Parent | Criterion | Trigger ID | Exact condition keys | Requirements | Local data row | Local lang keys | Phase 0 notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `adventure/ol_betsy` | `minecraft:adventure/root` | `shot_crossbow` | `minecraft:shot_crossbow` | `item.items` | `[["shot_crossbow"]]` | missing | missing | Verified shape is narrow but not zero-condition: `item.items = "minecraft:crossbow"`. Current parser does not support `shot_crossbow` or nested item predicates. |
| `adventure/shoot_arrow` | `minecraft:adventure/kill_a_mob` | `shot_arrow` | `minecraft:player_hurt_entity` | `damage.type.direct_entity.type`, `damage.type.tags[]` | `[["shot_arrow"]]` | missing | missing | Not `target_hit`. Uses damage predicate shape with arrow tag/type checks; outside current narrow parser surface. |
| `adventure/bullseye` | `minecraft:adventure/shoot_arrow` | `bullseye` | `minecraft:target_hit` | `projectile[]`, `projectile[].condition`, `projectile[].entity`, `projectile[].predicate.distance.horizontal.min`, `signal_strength` | `[["bullseye"]]` | missing | missing | Requires `signal_strength = 15` plus projectile entity-predicate list with horizontal distance min `30.0`. |
| `story/deflect_arrow` | `minecraft:story/obtain_armor` | `deflected_projectile` | `minecraft:entity_hurt_player` | `damage.type.tags[]`, `damage.blocked` | `[["deflected_projectile"]]` | missing | missing | Not `killed_by_arrow`. Raw semantics are any blocked projectile hit, not arrow-only. |
| `adventure/sniper_duel` | `minecraft:adventure/shoot_arrow` | `killed_skeleton` | `minecraft:player_killed_entity` | `entity[]`, `entity[].condition`, `entity[].entity`, `entity[].predicate.type`, `entity[].predicate.distance.horizontal.min`, `killing_blow.tags[]` | `[["killed_skeleton"]]` | missing | missing | Not `killed_by_arrow`. Raw semantics are kill skeleton with projectile killing blow and horizontal distance min `50.0`. |

## Current local support delta

- Existing local todo rows already mark all five as `missing-trigger`.
- No local advancement JSON files exist yet for these five rows under `data/minecraft/advancements/...`.
- No local localization keys exist yet in `lang/en_US.json` or `lang/zh_CN.json` for these five rows.
- Current compiled trigger-condition support is limited to block, item, entity, dimension, location structure, and loot table surfaces in `src/mod/advancement/TriggerIndex.cpp`.
- Of the verified trigger IDs in this wave, only `minecraft:player_killed_entity` is currently wired at runtime in `src/mod/advancement/RuntimeTriggerAdapters.cpp`; the verified condition shape needed by `adventure/sniper_duel` is still unsupported.

## Phase 0 guardrails from verified raw semantics

- Defer any old `killed_by_arrow` assumptions for `story/deflect_arrow` and `adventure/sniper_duel`; raw vanilla does not use that trigger for either row.
- Defer any old `target_hit` assumption for `adventure/shoot_arrow`; raw vanilla uses `minecraft:player_hurt_entity`.
- Do not approximate nested predicate shapes silently: `damage`, `projectile[]`, `entity[]`, `killing_blow.tags[]`, and `item.items` all need explicit support decisions in later phases.
