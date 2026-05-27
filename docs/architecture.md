# Advancements Architecture

This document keeps both the current architecture and the proposed target architecture for future refactor and optimization waves.

## Current Architecture

The current implementation is reload-centered. Advancement JSON is parsed on reload, indexes are rebuilt, runtime hooks/events create `TriggerContext`, and `TriggerDispatcher` grants progress through `ProgressService`.

```mermaid
flowchart TD
    subgraph Data["Advancement Data"]
        Json["data/**/advancements/*.json"]
        Lang["lang/*.json"]
    end

    subgraph Reload["Reload Phase"]
        Loader["AdvancementLoader"]
        Parser["AdvancementParser"]
        Definitions["LoadResult / AdvancementDefinition"]
        GuiIndex["AdvancementGuiIndex"]
        CommandIndex["AdvancementCommandIndex"]
        TriggerIndex["TriggerIndex"]
    end

    subgraph Criteria["Criteria Layer"]
        Registry["TriggerCriteriaRegistry"]
        Descriptor["TriggerDescriptor"]
        Compile["compile conditions"]
        Match["match TriggerContext"]
    end

    subgraph Runtime["Runtime Event Sources"]
        InventoryRuntime["InventoryRuntime"]
        CombatRuntime["CombatRuntime"]
        WorldRuntime["WorldRuntime"]
        ProjectileRuntime["ProjectileRuntime"]
        LootRuntime["LootRuntime"]
        EffectRuntime["EffectRuntime"]
    end

    subgraph Dispatch["Dispatch Pipeline"]
        Context["TriggerContext"]
        Adapter["dispatchTrigger"]
        Dispatcher["TriggerDispatcher"]
    end

    subgraph Progress["Progress Layer"]
        Service["ProgressService"]
        Store["ProgressStore"]
        PlayerJson["world/advancements/<uuid>.json"]
    end

    subgraph Presentation["Read / Presentation"]
        Gui["AdvancementGui"]
        Commands["AdvancementsCommand"]
        Notifier["AdvancementNotifier"]
    end

    Json --> Loader
    Lang --> Loader
    Loader --> Parser
    Parser --> Definitions
    Definitions --> GuiIndex
    Definitions --> CommandIndex
    Definitions --> TriggerIndex

    Registry --> Descriptor
    Descriptor --> Compile
    Compile --> TriggerIndex

    InventoryRuntime --> Context
    CombatRuntime --> Context
    WorldRuntime --> Context
    ProjectileRuntime --> Context
    LootRuntime --> Context
    EffectRuntime --> Context

    Context --> Adapter
    Adapter --> Dispatcher
    TriggerIndex --> Dispatcher
    Dispatcher --> Match
    Match --> Service

    Service --> Store
    Store --> PlayerJson
    PlayerJson --> Service

    GuiIndex --> Gui
    CommandIndex --> Commands
    Service --> Gui
    Service --> Commands
    Service --> Notifier
```

### Current Runtime Trigger Flow

```mermaid
sequenceDiagram
    participant Runtime as Runtime Hook/Event
    participant Adapter as dispatchTrigger()
    participant Index as TriggerIndex
    participant Dispatcher as TriggerDispatcher
    participant Criteria as TriggerDescriptor.match()
    participant Progress as ProgressService
    participant Store as ProgressStore
    participant Notifier as AdvancementNotifier

    Runtime->>Adapter: create TriggerContext
    Adapter->>Index: find(triggerId)
    Index-->>Adapter: CriterionBinding[]
    Adapter->>Dispatcher: dispatch(context)
    Dispatcher->>Criteria: match(compiled condition, context)

    alt matched
        Dispatcher->>Progress: grantCriterion(player, advancement, criterion)
        Progress->>Store: mutate cached PlayerProgress
        Store-->>Progress: changed / done / becameDone

        alt becameDone
            Dispatcher->>Notifier: notifyAdvancementCompleted()
        end
    else not matched
        Dispatcher-->>Runtime: no-op
    end
```

### Current Pain Points

- `trigger/runtime/*` mixes hook registration, LL event consumption, game-fact extraction, trigger-specific state, and `dispatchTrigger` calls.
- `trigger/criteria/*` mixes trigger condition parsing with reusable vanilla/wiki predicate parsing.
- `TriggerIndex.h` owns a growing `TriggerPayload` / `TriggerCondition` variant that becomes harder to extend as more triggers are added.
- Common predicate shapes such as player, entity, item, block, location, distance, and damage are repeated across trigger-specific criteria files.

## Target Architecture

The target architecture separates game event sourcing, reusable predicate parsing/matching, and per-trigger modules.

```mermaid
flowchart TD
    subgraph Game["Bedrock / LeviLamina"]
        BedrockHooks["Bedrock hooks"]
        NativeEvents["LL native events"]
    end

    subgraph EventLayer["src/mod/event"]
        PlayerEvents["player events"]
        EntityEvents["entity events"]
        ItemEvents["item events"]
        BlockEvents["block events"]
        WorldEvents["world events"]
    end

    subgraph PredicateLayer["src/mod/predicate"]
        CommonPredicate["Common helpers"]
        PlayerPredicate["PlayerPredicate"]
        EntityPredicate["EntityPredicate"]
        ItemPredicate["ItemPredicate"]
        BlockPredicate["BlockPredicate"]
        LocationPredicate["LocationPredicate"]
        DamagePredicate["DamagePredicate"]
        DistancePredicate["DistancePredicate"]
        EffectPredicate["EffectPredicate"]
    end

    subgraph TriggerLayer["src/mod/trigger"]
        TriggerRegistry["TriggerRegistry"]
        TriggerIndexNew["TriggerIndex"]
        TriggerDispatcherNew["TriggerDispatcher"]

        subgraph TriggerModules["src/mod/trigger/triggers"]
            LocationTrigger["LocationTrigger"]
            InventoryChangedTrigger["InventoryChangedTrigger"]
            PlayerKilledEntityTrigger["PlayerKilledEntityTrigger"]
            PlayerHurtEntityTrigger["PlayerHurtEntityTrigger"]
            ConstructBeaconTrigger["ConstructBeaconTrigger"]
            CuredZombieVillagerTrigger["CuredZombieVillagerTrigger"]
        end
    end

    subgraph AdvancementLayer["Advancement / Progress"]
        LoaderNew["AdvancementLoader / Parser"]
        DefinitionsNew["AdvancementDefinition"]
        ProgressServiceNew["ProgressService"]
        ProgressStoreNew["ProgressStore"]
        NotifierNew["AdvancementNotifier"]
    end

    BedrockHooks --> EventLayer
    NativeEvents --> EventLayer

    PlayerEvents --> TriggerModules
    EntityEvents --> TriggerModules
    ItemEvents --> TriggerModules
    BlockEvents --> TriggerModules
    WorldEvents --> TriggerModules

    LoaderNew --> DefinitionsNew
    DefinitionsNew --> TriggerIndexNew
    TriggerRegistry --> TriggerIndexNew

    TriggerModules --> TriggerRegistry
    TriggerModules --> PredicateLayer
    PredicateLayer --> TriggerModules

    TriggerModules --> TriggerDispatcherNew
    TriggerIndexNew --> TriggerDispatcherNew
    TriggerDispatcherNew --> ProgressServiceNew
    ProgressServiceNew --> ProgressStoreNew
    ProgressServiceNew --> NotifierNew
```

### Target Runtime Flow

```mermaid
sequenceDiagram
    participant Bedrock as Bedrock / LL Signal
    participant EventSource as src/mod/event source
    participant EventBus as Project Event Bus
    participant Trigger as Per-Trigger Module
    participant Predicate as Predicate Module
    participant Dispatcher as TriggerDispatcher
    participant Progress as ProgressService

    Bedrock->>EventSource: hook callback or native LL event
    EventSource->>EventBus: publish typed game event
    EventBus->>Trigger: trigger module listener receives event
    Trigger->>Trigger: build trigger-specific fact context
    Trigger->>Predicate: match reusable predicates

    alt predicate matched
        Trigger->>Dispatcher: dispatch trigger hit
        Dispatcher->>Progress: grantCriterion(...)
    else predicate not matched
        Trigger-->>EventBus: no-op
    end
```

## Target Directory Shape

```text
src/mod/
  event/
    player/
    entity/
    item/
    block/
    world/

  predicate/
    Common.*
    EntityPredicate.*
    PlayerPredicate.*
    ItemPredicate.*
    BlockPredicate.*
    LocationPredicate.*
    DamagePredicate.*
    DistancePredicate.*
    EffectPredicate.*

  trigger/
    TriggerRegistry.*
    TriggerDispatcher.*
    TriggerIndex.*
    TriggerModule.*
    triggers/
      InventoryChangedTrigger.*
      ConsumeItemTrigger.*
      LocationTrigger.*
      LevitationTrigger.*
      ChangedDimensionTrigger.*
      PlayerKilledEntityTrigger.*
      PlayerHurtEntityTrigger.*
      ConstructBeaconTrigger.*
      CuredZombieVillagerTrigger.*
```

## Layer Boundaries

### Event Layer

Owns:

- Existing LL event subscriptions.
- Bedrock hooks when LL has no native event.
- Typed game events grouped by player, entity, item, block, and world.
- Stable payload extraction from raw game objects.

Must not own:

- Advancement IDs.
- Criterion IDs.
- Trigger IDs.
- JSON condition parsing.
- Predicate matching.
- Progress mutation or persistence.

### Predicate Layer

Owns:

- Wiki/vanilla-style predicate parsing.
- Reusable predicate matching for player, entity, item, block, location, damage, distance, and effects.
- Small predicate context objects assembled by trigger modules from events.

Must not own:

- Event registration.
- Trigger registration.
- Advancement progress mutation.
- Trigger-specific dispatch timing.

### Trigger Layer

Owns:

- Trigger IDs.
- Per-trigger condition parsing.
- Per-trigger event subscriptions.
- Per-trigger state such as location polling cadence or levitation start positions.
- Mapping event payloads into predicate contexts.
- Calling `dispatchTrigger` or the future dispatcher entry point when a trigger is satisfied.

Must not own:

- Bedrock hook mechanics.
- Raw LL event adaptation.
- Duplicated player/item/block/location predicate parsing.
- Progress persistence.

## Predicate Reuse Model

```mermaid
flowchart LR
    Conditions["criteria conditions JSON"] --> TriggerParse["Trigger parse"]
    TriggerParse --> PlayerPred["PlayerPredicate"]
    TriggerParse --> EntityPred["EntityPredicate"]
    TriggerParse --> ItemPred["ItemPredicate"]
    TriggerParse --> BlockPred["BlockPredicate"]
    TriggerParse --> LocationPred["LocationPredicate"]
    TriggerParse --> DamagePred["DamagePredicate"]

    PlayerPred --> EntityPred
    EntityPred --> ItemPred
    EntityPred --> LocationPred
    EntityPred --> DistancePred["DistancePredicate"]
    DamagePred --> EntityPred

    EventFacts["event facts"] --> PredicateContext["predicate context"]
    PredicateContext --> PlayerPred
    PredicateContext --> EntityPred
    PredicateContext --> ItemPred
    PredicateContext --> BlockPred
    PredicateContext --> LocationPred
    PredicateContext --> DamagePred
```

Examples:

- `minecraft:location` parses `conditions.player` through `PlayerPredicate`, which can reuse `EntityPredicate` and `LocationPredicate`.
- `minecraft:target_hit` parses `conditions.projectile` through `EntityPredicate`, which can reuse `DistancePredicate`.
- `minecraft:player_hurt_entity` parses `conditions.damage` through `DamagePredicate`, which can reuse `EntityPredicate` for `direct_entity`.
- `minecraft:villager_trade` parses player location constraints through `PlayerPredicate` and `LocationPredicate`.

## Migration Map

```mermaid
flowchart TD
    subgraph Current["Current files"]
        RuntimeOld["trigger/runtime/*.cpp"]
        CriteriaOld["trigger/criteria/*.cpp"]
        VariantOld["TriggerIndex.h variant"]
    end

    subgraph Target["Target files"]
        EventNew["event/<domain>/*Event.*"]
        PredicateNew["predicate/*Predicate.*"]
        TriggerNew["trigger/triggers/*Trigger.*"]
        RegistryNew["TriggerRegistry / TriggerIndex"]
    end

    RuntimeOld --> EventNew
    RuntimeOld --> TriggerNew
    CriteriaOld --> PredicateNew
    CriteriaOld --> TriggerNew
    VariantOld --> RegistryNew
```

Recommended first wave:

1. Add `event/player/PlayerTickEvent.*` as the event-source seam for `Player::$tickWorld`.
2. Add minimal `predicate/PlayerPredicate.*` and `predicate/LocationPredicate.*` for the currently supported `player[0].predicate.location.structures` shape.
3. Move `minecraft:location` into `trigger/triggers/LocationTrigger.*` as the first per-trigger module.
4. Keep `TriggerDispatcher` and `ProgressService` behavior unchanged.
5. Verify the same trigger IDs, same player attribution, same 20-tick cadence, and same advancement completion behavior before migrating another trigger.

## Stable Rules

- Event layer describes game facts only.
- Predicate layer parses and matches reusable wiki/vanilla predicates only.
- Trigger modules listen to events, compose predicates, and dispatch trigger hits.
- Dispatcher and progress layers continue to own criterion grant, completion notification, and persistence.
- Do one trigger migration wave at a time.
