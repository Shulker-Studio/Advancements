# Advancements Architecture

This document keeps both the current architecture and the proposed target architecture for future refactor and optimization waves.

## Current Architecture

The current implementation is reload-centered. Advancement JSON is parsed on reload, indexes are rebuilt, plugin-owned event sources publish typed game facts, trigger modules create `TriggerContext`, and `TriggerDispatcher` grants progress through `ProgressService`.

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

    subgraph Criteria["Criteria / Registry Layer"]
        Registry["TriggerRegistry"]
        CriteriaFacade["TriggerCriteriaRegistry facade"]
        Descriptor["TriggerDescriptor"]
        Compile["compile conditions"]
        Match["match TriggerContext"]
    end

    subgraph EventSources["Event Sources"]
        PlayerEvents["src/mod/event/player"]
        ItemEvents["src/mod/event/item"]
        EntityEvents["src/mod/event/entity"]
        BlockEvents["src/mod/event/block"]
    end

    subgraph TriggerModules["Trigger Modules"]
        Triggers["src/mod/trigger/triggers/*Trigger"]
        Predicates["src/mod/predicate/*Predicate"]
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

    Registry --> CriteriaFacade
    CriteriaFacade --> Descriptor
    Descriptor --> Compile
    Compile --> TriggerIndex

    PlayerEvents --> Triggers
    ItemEvents --> Triggers
    EntityEvents --> Triggers
    BlockEvents --> Triggers
    Triggers --> Predicates
    Triggers --> Context

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
    participant EventSource as src/mod/event source
    participant Trigger as Trigger module
    participant Adapter as dispatchTrigger()
    participant Index as TriggerIndex
    participant Dispatcher as TriggerDispatcher
    participant Criteria as TriggerDescriptor.match()
    participant Progress as ProgressService
    participant Store as ProgressStore
    participant Notifier as AdvancementNotifier

    EventSource->>Trigger: publish typed game fact
    Trigger->>Trigger: match trigger-specific conditions
    Trigger->>Adapter: create TriggerContext
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
        Dispatcher-->>Trigger: no-op
    end
```

### Current Pain Points

- The old `trigger/runtime/*` layer has been removed; keep future runtime seams in `src/mod/event/**` and trigger-specific logic in `src/mod/trigger/triggers/**`.
- `trigger/criteria/*` remains descriptor-facing glue and should not regain reusable vanilla/wiki predicate parsing that now belongs in `src/mod/predicate/**`.
- `TriggerIndex.h` still stores compiled descriptor-backed bindings; avoid expanding compatibility variants unless a concrete advancement requires it.
- Common predicate shapes such as player, entity, item, block, location, distance, and damage should stay centralized in predicate helpers.

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
    participant EventBus as LeviLamina EventBus
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
- Per-trigger state such as location polling cadence, levitation start positions, or cure-tracking ownership where a Bedrock seam still needs trigger-local state.
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
        EventCurrent["event/<domain>/*Event.*"]
        TriggerCurrent["trigger/triggers/*Trigger.*"]
        PredicateCurrent["predicate/*Predicate.*"]
        RegistryCurrent["TriggerRegistry / TriggerIndex"]
    end

    subgraph Future["Future cleanup"]
        EventNew["event/<domain>/*Event.*"]
        PredicateNew["predicate/*Predicate.*"]
        TriggerNew["trigger/triggers/*Trigger.*"]
        RegistryNew["TriggerRegistry / TriggerIndex"]
    end

    EventCurrent --> EventNew
    TriggerCurrent --> TriggerNew
    PredicateCurrent --> PredicateNew
    RegistryCurrent --> RegistryNew
```

Current post-0.1.2 state:

1. `trigger/runtime/*` has been removed from the current source tree.
2. Existing migrated triggers consume plugin-owned events from `src/mod/event/**`.
3. Reusable predicate helpers live under `src/mod/predicate/**`.
4. Trigger descriptors are registered through `TriggerRegistry` while `TriggerIndex` and `TriggerDispatcher` still own reload-time binding and grant dispatch.
5. Future work should add one concrete event/trigger slice at a time and avoid broad generic trigger scaffolding.

## Stable Rules

- Event layer describes game facts only.
- Predicate layer parses and matches reusable wiki/vanilla predicates only.
- Trigger modules listen to events, compose predicates, and dispatch trigger hits.
- Dispatcher and progress layers continue to own criterion grant, completion notification, and persistence.
- Do one trigger migration wave at a time.
