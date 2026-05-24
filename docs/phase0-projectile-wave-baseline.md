# Phase 0 Projectile Wave Baseline

Verified against raw vanilla JSON from MCAsset Cloud (`1.21.5-pre2`) for the five rows in `.sisyphus/plans/advancements-projectile-trigger-wave.md` Phase 0.

| Vanilla ID | Parent | Criterion | Trigger ID | Exact condition keys | Requirements | Local data row | Local lang keys | Phase 0 notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `adventure/ol_betsy` | `minecraft:adventure/root` | `shot_crossbow` | `minecraft:shot_crossbow` | `item.items` | `[["shot_crossbow"]]` | missing | missing | Verified shape is narrow but not zero-condition: `item.items = "minecraft:crossbow"`. Current parser does not support `shot_crossbow` or nested item predicates. |
| `adventure/shoot_arrow` | `minecraft:adventure/kill_a_mob` | `shot_arrow` | `minecraft:player_hurt_entity` | `damage.type.direct_entity.type`, `damage.type.tags[]` | `[["shot_arrow"]]` | done | done | Not `target_hit`. 已落地窄实现：仅支持 `#minecraft:arrows` + `minecraft:is_projectile` 这组已核 surface。 |
| `adventure/bullseye` | `minecraft:adventure/shoot_arrow` | `bullseye` | `minecraft:target_hit` | `projectile[]`, `projectile[].condition`, `projectile[].entity`, `projectile[].predicate.distance.horizontal.min`, `signal_strength` | `[["bullseye"]]` | done | done | 已落地窄实现：仅支持已核 shape（`signal_strength = 15` + projectile 水平距离 `>= 30`）。 |
| `story/deflect_arrow` | `minecraft:story/obtain_armor` | `deflected_projectile` | `minecraft:entity_hurt_player` | `damage.type.tags[]`, `damage.blocked` | `[["deflected_projectile"]]` | missing | missing | Not `killed_by_arrow`. Raw semantics are any blocked projectile hit, not arrow-only. |
| `adventure/sniper_duel` | `minecraft:adventure/shoot_arrow` | `killed_skeleton` | `minecraft:player_killed_entity` | `entity[]`, `entity[].condition`, `entity[].entity`, `entity[].predicate.type`, `entity[].predicate.distance.horizontal.min`, `killing_blow.tags[]` | `[["killed_skeleton"]]` | missing | missing | Not `killed_by_arrow`. Raw semantics are kill skeleton with projectile killing blow and horizontal distance min `50.0`. |

## Current local support delta

- `adventure/ol_betsy` is now shipped locally with JSON and both language keys.
- `adventure/shoot_arrow` is now shipped locally with JSON and both language keys.
- `story/deflect_arrow` and `adventure/sniper_duel` still have no local advancement JSON rows.
- Current compiled trigger-condition support now includes the shipped narrow surfaces for:
  - `minecraft:shot_crossbow` via `item.items = minecraft:crossbow`
  - `minecraft:player_hurt_entity` via `#minecraft:arrows` + `minecraft:is_projectile`
- Of the verified trigger IDs in this wave, `minecraft:shot_crossbow`, `minecraft:player_hurt_entity`, `minecraft:player_killed_entity`, and the bullseye-only slice of `minecraft:target_hit` are now wired at runtime; `minecraft:entity_hurt_player` remains unsupported.

## Phase 0 guardrails from verified raw semantics

- Defer any old `killed_by_arrow` assumptions for `story/deflect_arrow` and `adventure/sniper_duel`; raw vanilla does not use that trigger for either row.
- Defer any old `target_hit` assumption for `adventure/shoot_arrow`; raw vanilla uses `minecraft:player_hurt_entity`.
- Do not approximate nested predicate shapes silently: `damage`, `projectile[]`, `entity[]`, `killing_blow.tags[]`, and `item.items` all need explicit support decisions in later phases.
