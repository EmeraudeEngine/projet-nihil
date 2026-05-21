# AGENTS.md - AI Context & Guidelines

## 1. Identity

**projet-nihil** is the **showcase / bootup project** of the Emeraude family: the minimal,
complete, *zero-effort* starting point for someone discovering `emeraude-engine`.

> [!IMPORTANT]
> **This is the newcomer's entry door.** Where `projet-alpha` is the heavy R&D testbed that
> pushes the engine to its limits, projet-nihil is the opposite: the **smallest possible
> real application** that still does something meaningful. If a new developer (or a new AI
> session) wants to learn "how do I build something on this engine?", they read this project
> first. Keep it **simple, readable, and exemplary** — it is a teaching artifact as much as a
> program. Resist adding complexity here; complexity belongs in the engine or in projet-alpha.

- **A single `Application` class** derived from `EmEn::Core` — nothing more.
- **Language:** C++20. **Indentation:** tabs. **Platforms:** Linux, macOS, Windows.
- **License:** LGPLv3 (it is a public reference, unlike the proprietary projet-alpha).

## 2. Position in the family

projet-nihil sits at the top of the same dependency cascade every Emeraude project shares:

```
ext-deps-generator   → prebuilt external deps (zlib, sndfile, freetype, …)
        ↓
emeraude-base        → foundation library (EmEn::Base): math, I/O, hashing, factories,
                       threading. Single source of truth for external deps + compile policy.
        ↓ (cloned by the engine)
emeraude-engine      → the Vulkan runtime (EmEn::Core, Scenes, Graphics, Audio, Physics).
        ↓ (cloned by InstallEmeraudeEngine.cmake)
projet-nihil         → THIS project. A minimal application on top of the engine.
```

When you configure projet-nihil, CMake **clones the engine automatically**
(`cmake/InstallEmeraudeEngine.cmake`), and the engine in turn clones `emeraude-base`. A
consumer of nihil does not manage any of this by hand — the cascade resolves itself.

> [!NOTE]
> **The emeraude-base split is transparent here.** projet-nihil consumes the engine; the
> engine consumes a pinned `emeraude-base`. The only thing nihil sees of the split is the
> namespace: foundation types are `EmEn::Base::*` (e.g. `EmEn::Base::PixelFactory::Color`,
> `EmEn::Base::Hash`, `EmEn::Base::ObservableTrait`). This is the former `EmEn::Libs::*`,
> renamed `Libs` → `Base`. Migrating nihil to the split required **only that namespace change
> in source — zero CMake changes** — which is the cleanest proof the separation is sound.

## 3. Architecture

The whole program is one class: [`src/Application.hpp`](src/Application.hpp) /
[`src/Application.cpp`](src/Application.cpp), `class Application final : public EmEn::Core`.

It overrides four `EmEn::Core` lifecycle hooks — this is the canonical pattern a newcomer
should learn:

| Hook | Role |
|------|------|
| `onBeforeCoreSecondaryServicesInitialization()` | Tweak services before they spin up. |
| `onCoreStarted(arguments, settings)` | **Build the scene**: skybox, terrain, camera, lights, nodes, animations, materials. The bulk of the demo lives here. |
| `onCoreProcessLogics(engineCycle)` | Per-cycle update loop (animation, logic). Runs on the main thread. |
| `onCoreKeyRelease(key, scancode, modifiers)` | Input handling / application shortcuts. |

Supporting pieces:
- **`src/Boot/{linux/main.cpp, mac/main.mm, windows/main.cpp}`** — per-OS entry points that
  instantiate `Application` and run it. The only platform-specific code in the project.
- **`src/config.hpp.in`** — CMake-generated configuration header (`config.hpp`).
- **`src/ApplicationSettingKeys.hpp`** — string keys for settings.
- **`src/Scenes/StaticEntity.hpp`** — a small local scene-entity helper.

## 4. What it demonstrates (the newcomer learning path)

A reader should be able to follow `onCoreStarted()` top-to-bottom and learn:

- **Application lifecycle** — init → run loop → shutdown, the right hooks for each phase.
- **Scene management** — creating a scene, skybox, procedural terrain (Perlin noise).
- **Resource loading** — meshes, materials, geometry via the resource manager.
- **Scene graph** — nodes with transforms (camera node, cube, torus, material spheres).
- **Camera** — an animated camera with smooth interpolation.
- **Lighting** — static vs dynamic lighting (toggleable).
- **Animations** — keyframe-based animation with interpolation.
- **Input** — keyboard events and shortcuts.

## 5. Conventions

- **Tabs** for indentation, never spaces.
- **C++20**, cross-platform strict.
- Foundation types come from **`EmEn::Base::*`** (math, hashing, pixels, traits); engine
  types from `EmEn::*` (`EmEn::Core`, `EmEn::Scenes`, …).
- Keep additions **minimal and didactic** — this project's value is its readability.

## 6. Build & run

projet-nihil ships a Python helper, [`build.py`](build.py) (see [`README.md`](README.md) for
the full prerequisites: Vulkan SDK, CMake ≥ 3.25.1, a C++20 compiler).

```bash
# Configure (clones emeraude-engine, which clones emeraude-base) + build:
python build.py            # or the standard CMake flow:
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug -j$(nproc)
```

The external dependency archives and the engine/base compile policy are provided by
`emeraude-base` through the engine — projet-nihil does not configure them.

## 7. Relationship with the rest of the family

| Project | Role | This project's link to it |
|---------|------|----------------------------|
| `emeraude-base` | foundation library (`EmEn::Base`) | consumed transitively via the engine |
| `emeraude-engine` | Vulkan runtime (`EmEn::Core`, …) | the direct dependency, auto-cloned |
| `projet-alpha` | proprietary R&D testbed for the engine | sibling — the heavy counterpart to this light showcase |

> [!IMPORTANT]
> **Doc-first rule (family-wide).** AI documentation is as important as the code. If you
> change projet-nihil's structure, lifecycle usage, or its relationship to the engine/base,
> update this `AGENTS.md` in the same session and tell the user what changed.