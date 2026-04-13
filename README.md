# Speeduino Minus

Speeduino Minus is a modular fork of Speeduino.

It keeps the Speeduino engine management stack, but changes how the firmware is organized and built: instead of treating every feature as part of one always-on binary, this project tries to make features smaller, clearer, and removable.

The original idea for the name was `Speeduino+`, to signal a more configurable variant of the project. The `+` did not survive repository naming cleanly, so the project became `Speeduino Minus`. That ended up matching the goal better anyway: Speeduino, with less baggage in each build.

## What This Project Is

This is not a rewrite of Speeduino.

It is a refactor-oriented fork that keeps the existing firmware behavior as the baseline, while reorganizing the codebase around:

- a smaller orchestration/core layer
- removable feature modules
- clearer ownership of state, pages, storage, and runtime hooks
- build-time composition for different vehicle/use-case profiles

The intent is straightforward: make the codebase easier to navigate, make the firmware easier to tailor, and stop forcing every build to carry features it does not need.

## Why Modularization Exists Here

The original problem is not just binary size. It is also codebase shape.

A large ECU firmware naturally accumulates:

- features used by only a subset of vehicles
- niche motorsport logic
- multiple communication paths
- storage/page layouts tied to historical growth
- behavior that started generic but became highly specialized over time

When all of that lives in the same center of gravity, two things happen:

1. It becomes harder to understand the project without reading file by file.
2. It becomes harder to make a smaller or more specialized firmware without cutting across unrelated code.

Modularization is the answer to both problems.

In this repository, that means:

- `core` and orchestration stay responsible for making the firmware run
- optional features live under `speeduino/modules/`
- page/storage/runtime hooks move toward feature ownership
- profiles such as `street`, `race`, and `motorsport` can be composed at build time

## What This Solves

This work is meant to solve practical problems, not just make the folder tree look nicer.

- Smaller firmware variants become possible without hand-editing the codebase.
- Features that only matter to small groups of users can be isolated instead of leaking into the whole system.
- New work becomes easier to place. A future feature like ETB, knock strategy expansion, or richer transient fueling can land in a domain instead of the center of the firmware.
- The code becomes easier to reason about because state, hooks, storage, and pages are moving closer to the feature that owns them.
- Different users can build different products from the same repository without every build pretending to be the same firmware.

The goal is not “maximum number of modules”. The goal is better boundaries.

Some things should become modules. Some things should remain parameters inside a stable module. This repository is intentionally moving toward that distinction.

## Current Direction

The codebase is being organized around:

- `speeduino/orchestration/` for runtime flow
- `speeduino/model/` and `speeduino/data/` for state and config contracts
- `speeduino/storage/` for pages and persistence
- `speeduino/boards/` for platform-specific code
- `speeduino/modules/` for optional or specialized domains

Today, modules already exist for areas such as:

- `logging`
- `secondary_serial`
- `comms_extended`
- `table_switching`
- `boost`
- `vvt`
- `wmi`
- `engine_protection`
- `launch_flatshift`
- `fan_aircon`
- `nitrous`
- `knock`

This is still an evolving architecture, but the direction is deliberate: more feature ownership, less incidental coupling.

## Build The Firmware

Builds are done with PlatformIO from the repository root.

Base build:

```bash
platformio run -e megaatmega2560
```

Configurable build using the feature configuration:

```bash
python3 tools/build_firmware.py --env megaatmega2560-configurable --profile street
```

Other useful builds:

```bash
platformio run -e megaatmega2560-core-only
platformio run -e teensy41
platformio test -e native_code_coverage
```

## Build Profiles

The repository includes profile and feature configuration files:

- [`firmware.config.ini`](./firmware.config.ini)
- [`firmware.presets/street.ini`](./firmware.presets/street.ini)
- [`firmware.presets/race.ini`](./firmware.presets/race.ini)
- [`firmware.presets/motorsport.ini`](./firmware.presets/motorsport.ini)
- [`firmware.presets/full.ini`](./firmware.presets/full.ini)

The configurable build flow works like this:

1. A profile or manual config selects modules and subfeatures.
2. The build scripts translate that into feature flags and source filters.
3. PlatformIO builds only the selected firmware composition.

That means a `street` firmware does not need to look like a `motorsport` firmware, and a minimal firmware does not need to drag in every specialized path just because they share one repository.

## Documentation

The upstream Speeduino manual is still the best reference for ECU behavior and tuning concepts:

https://wiki.speeduino.com

For this fork specifically, the important local references are:

- `platformio.ini`
- `firmware.config.ini`
- `firmware.presets/`
- `tools/build_firmware.py`
- `tools/pio_feature_config.py`

## Hardware And Community

This repository follows the Speeduino ecosystem. If you are looking for hardware, documentation, or community support around the broader project:

- [Where to buy Speeduino-compatible hardware](https://speeduino.com/home/where-to-buy)
- [Discord](https://discord.gg/YWCEexaNDe)
- [Speeduino Forum](https://speeduino.com/forum)
- [Facebook Group](https://www.facebook.com/groups/191918764521976/)

## Contributing

If you want to work on the code, start by reading [`contributing.md`](./contributing.md).

The most important thing to understand before changing this repository is that modularization here is not cosmetic. File moves only matter when they improve ownership, reduce coupling, or make feature composition more honest.
