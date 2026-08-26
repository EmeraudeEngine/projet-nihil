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

That is the **whole** of `src/` — four files plus the three boot entry points. If you are about
to add a fifth, ask whether it belongs in the engine instead (see § 1).

## 4. What it demonstrates (the newcomer learning path)

A reader should be able to follow `onCoreStarted()` top-to-bottom and learn:

- **Application lifecycle** — init → run loop → shutdown, the right hooks for each phase.
- **Scene management** — creating a scene, skybox, procedural terrain (Perlin noise).
- **Resource loading** — meshes, materials, geometry via the resource manager.
- **Scene graph** — nodes with transforms (camera node, cube, torus, material spheres).
- **Camera** — an animated camera with smooth interpolation.
- **Lighting** — sky-driven vs hand-authored dynamic lighting (toggleable), in real
  photometric units.
- **Post-processing** — the sensor / optics / artistic split, and the three cycling
  photographic looks (golden hour, silver noir, raw sensor).
- **Animations** — keyframe-based animation with interpolation.
- **Input & feedback** — keyboard events, on-screen notifications (`Core::notifyUser()`), and a
  chime synthesized from scratch (`WaveFactory::Synthesizer` into an `Audio::SoundResource`).

### Lighting: the two modes (Jul 2026, dusk revamp Sep 2026)

The `App/UseSkyLighting` setting (`src/ApplicationSettingKeys.hpp`, default `true`) picks
between the two, and each teaches a different half of the engine's lighting model:

| Mode | What it does |
|------|--------------|
| **Sky-driven** (`true`, default) | `Scene::applyBackgroundLighting()` — the engine derives the whole lighting from the background's photometric manifest: ambient (average sky color × ambient illuminance, served by the baked IBL irradiance) plus one directional light per declared celestial body. The demo's authored dusk sky declares **one setting sun** (2700 K, 6000 lx, ~14° high), so the single call produces the warm ambient AND a directional light with a 4096² shadow map — the whole lighting, zero further authoring. Remove the star from the manifest and the call falls back to pure image-based lighting. |
| **Dynamic** (`false`) | Hand-authored: a 6 000 lx setting sun with a 4096² shadow map, two animated orbiting 80–100 klm floodlights, a spotlight, and an explicit 2 000 lx warm ambient so shadowed sides are not black. |

> [!IMPORTANT]
> **The old "static lighting" mode is gone.** `LightSet::enableAsStaticLighting()` and
> `Saphir::StaticLighting` were removed engine-side (commit `ca85b8bf`, "Sky: photometric
> background contract, sky-driven lighting, static lighting removal") — a pre-photometry
> performance shortcut. `RenderPassType::SimplePass` is now strictly **unlit**. Sky-driven
> lighting is its didactic successor: the cheap, zero-authoring path, but physically derived
> instead of faked.

> [!WARNING]
> **All light quantities are PHOTOMETRIC, in real-world units** — not `[0..1]` sliders.
> A directional light takes an **illuminance in lux** (direct sun 100 000, overcast 10 000,
> office ~500, full moon 0.25); a point or spot light takes a **luminous power in lumens**
> (household bulb 800, large floodlight ~100 000). World units are **metres**.

### Scene scale: human, on purpose (Jul 2026)

The scene is **human-scale**: a 1 m cube, 35 cm spheres, a 60 cm torus on a 82 m terrain, with
the camera orbiting 5 m out. It was rescaled from a monumental version (100 m cube, lamps
orbiting 300 m out) for one reason: **photometric units only teach anything at a scale where
the real numbers apply.** At 300 m the orbiting lamps needed ~10⁹ lumens to register against
the sun — arithmetically correct (`E = I/d²`) but pedagogically backwards. At metres they are
ordinary catalogue fixtures.

When editing the scene, know which numbers follow the scale and which do not:

| Follows the scale (world units) | Scale-independent |
|---|---|
| positions, radii, grid size, orbit radii, bob amplitudes, shadow coverage | **lux** (a directional light's illuminance) |
| Perlin `factor` (a displacement) | Perlin `size` (a UV-space frequency) |
| point/spot `radius` (a culling bound) | animation periods (**seconds**), cone **angles** (degrees) |
| | iridescence film thickness (**nanometres**), bloom threshold (**nits**), roughness/metalness |

> [!NOTE]
> The dynamic mode's sun is deliberately a **gentle 6 000 lx setting sun**, not clear-day
> 100 000 lx. At 100 000 lx the hand-authored fixtures — the whole point of that mode —
> contribute ~2% and are invisible. Lighting is about **ratios**; raising the sun to
> `100'000.0F` to watch the coloured lamps vanish is the intended experiment.

### The dusk revamp (Sep 2026) — measured traps to preserve

The demo was rebuilt around an **authored dusk sky** after the PBR/HDR (nits) engine pass left
the old version bright, white and flat. What was measured then, and must not be re-learned:

- **The default SKYBOX is a trap for a demo**: its cubemap is a retro *sunset* gradient, but it
  emits at `DaylightSkyLuminance` (8000 nits) and declares **no celestial body** — a huge
  direction-less white-ish dome. The demo authors `DemoDuskSky` instead: same cubemap, 1000 nits,
  warm average color, 1200 lx ambient, one declared 2700 K / 6000 lx setting sun. That manifest
  is what `applyBackgroundLighting()` consumes.
- **`SkyBoxResource::load(material)` leaves the photometry AND the IBL source to the caller**:
  `setLuminance()` must match the material's emissive strength, and `setEnvironmentCubemap()`
  must be called explicitly — without it the sky renders fine while every surface reflects the
  default environment (the setter's doc says so; verified).
- **The ground must be created with `getOrCreateResourceSync()`**: the plain
  `getOrCreateResource()` defers loading to a loader thread, and until it lands
  `GroundLevelInterface::getLevelAt()` silently answers **0** (measured: every probe flat while
  the terrain visibly rolled). All placements are terrain-relative, so the grid must exist first.
- **The bloom threshold must sit above the brightest DIFFUSE surface.** At dusk the sun-lit
  porcelain reaches ~2400 nits; a 1500-nit threshold bloomed the whole cube into a white blob
  (measured). It sits at 3000 nits: glare for the metal/gem speculars only.
- **A white glazed material easily washes out**: full-strength environment reflection + hot
  specular + deep subsurface on a white albedo flattened every cube face to the same white. The
  porcelain keeps modest values (reflection 0.35, specular 1.0, subsurface 0.15).
- **Verification workflow**: the engine's `tools/demo-capture-bench.py` works with nihil —
  enable `Core/Console/EnableRemoteListener` in the user settings and pass any `--demo` value
  (nihil ignores it and always builds its scene).

### Comment policy: an invitation, not a wall (Sep 2026)

`Application.cpp` is read by people **discovering** the engine. Its comments are short and
didactic — *how you do X with the engine* — never internal development notes, war stories or
measurement logs (those live here). When editing the demo, keep that register.

### World convention: Y-up (Aug 2026)

The engine moved from **Y-DOWN to right-handed Y-UP** — `+X` right, `+Y` up, `-Z` forward, one unit
= one metre, the convention of glTF 2.0 / USD / FBX. The old Y-down triad `(right, down, back)` was
physically *left*-handed while every cross product in the engine assumed right-handed algebra, so
the renderer produced a **mirror image**. Full account, measurements included:
[`emeraude-engine/docs/coordinate-system.md`](https://github.com/EmeraudeEngine/emeraude-engine/blob/main/docs/coordinate-system.md).

`onCoreStarted()` and `onCoreProcessLogics()` were converted in the same move: every world
**vertical was negated**, and a Y value now reads directly as an ALTITUDE above the terrain.

| Site | Y-down (before) | Y-up (now) |
|---|---|---|
| cube, spheres, camera target | `-0.75F` | `0.75F` |
| torus | `-2.0F` | `2.0F` |
| orbiting floodlights | `-2.5F` / `-2.0F` | `2.5F` / `2.0F` |
| spotlight | `-5.0F` | `5.0F` |
| sun (its direction is the cursor position → origin) | `-10.0F` | `10.0F` |
| camera sweep band | `yMax -0.7F` … `yMin -3.5F` | `heightMin 0.7F` … `heightMax 3.5F` |

The vertical oscillations kept their **exact motion**: negating `base + A·sin(…)` yields
`base − A·sin(…)`, so the sign in front of the oscillating term flipped while the amplitude stayed
a positive magnitude. The demo renders as it did before the flip.

> [!NOTE]
> **Since Sep 2026 the placements are TERRAIN-RELATIVE**: the scene queries
> `Scene::groundLevel()->getLevelAt(x, z, deltaY)` (the same `GroundLevelInterface` the physics
> uses), so the altitudes in the table above are now *deltas above the local Perlin ground*, and
> the camera orbit and the orbiting lights clamp their keyframes above it.

> [!NOTE]
> **Only the world verticals moved.** X/Z coordinates, radii, cone angles, animation periods, lux
> and lumens are untouched. The `yaw()` / `pitch()` calls in `onCoreProcessLogics()` deliberately
> **kept their positive signs**: with `+Y` up, a positive world yaw is now counter-clockwise seen
> from above where it used to be clockwise. That is the idiomatic Y-up reading and the spin
> direction of a decorative cube is arbitrary — negate them if the exact former spin ever matters.

### Post-processing: three layers, only one is a matter of taste

`onCoreStarted()` separates them explicitly, and the `KeySpace` shortcut moves **only the
third** (see `Application::buildPhotographicLooks()` and `applyLook()`):

1. **The sensor — tone mapping.** `camera->enableHDR(true)`. Mandatory, not an effect: the
   renderer produces physical radiance and a screen cannot show it. Without it the raw
   radiance reaches an LDR swap-chain and a daylight scene comes out pure white.
2. **The optics — bloom.** `camera->enableBloom(true)`, declared on the camera so the engine
   materializes it at the right place in the chain (after defocus, before the sensor). Its
   threshold is an absolute luminance in **nits**, its intensity the **fraction** of energy
   the glass scatters (a clean lens: a few percent) — not an artistic gain.
3. **The artistic pass** — god rays (scene chain) + colour grading, vignetting, chromatic
   aberration, film grain (camera lens effects). The only creative layer, packaged as three
   **photographic looks** (`buildPhotographicLooks()` / `applyLook()`): golden hour, silver
   noir, raw sensor. `KeySpace` cycles them with an on-screen notification and a synthesized
   chime; tone mapping and bloom never move, so every look stays photographically valid.

> [!NOTE]
> Do **not** toggle `Renderer::postProcessor().enable()` from an application: it is a global
> kill-switch that would take the tone mapper down with everything else, leaving a blown-out
> image. It defaults to ON and costs nothing when there is nothing to run. Toggle your own
> effects instead — which is why both states of `KeySpace` stay photographically valid.
> Note also that lens effects are compiled **into** the composite shader, so the camera's
> effect *list* is the program cache key: disabling one means taking it off the camera
> (`clearLensEffects()`), not clearing a flag.

## Open work — `docs/todo/`, one file per idea

> [!IMPORTANT]
> **Project-wide rule (all repositories).** Open work lives in `docs/todo/`, **one file per idea
> to do**; there is no root `TODO.md`. **Done = the file is deleted** — no `[x]`, no "DONE"
> section, no `done/` archive. The knowledge that must survive the work (measurements, traps,
> decisions) goes to the documentation, written **before** the item file is deleted.
> Every item file carries a YAML front-matter: `id` (== file name, kebab-case), `title`,
> `status` (`open` | `in-progress` | `blocked` | `parked`), `priority`
> (`high` | `medium` | `low` | `unranked`), `scope`, `opened` (`YYYY-MM-DD` or `unknown`),
> plus optional `blocked-by` / `tags`. An ambiguous inherited item is **asked about**, never
> guessed.

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

### README ownership (Jul 2026)

The family's user-facing READMEs are **layered, non-redundant**: each level documents only what
it owns and links to the others by absolute GitHub URL
(`https://github.com/EmeraudeEngine/<repo>/blob/main/README.md`).

| README | Sole owner of |
|---|---|
| [emeraude-base](https://github.com/EmeraudeEngine/emeraude-base/blob/main/README.md) | foundation API & targets, **toolchain requirements**, **compile policy** (`EMERAUDE_*` options, standards, warnings-as-errors), **all external-dependency detail** (archive `v013`, folder grammar with ABI tags, auto-download, symlink bypass, overrides) |
| [emeraude-engine](https://github.com/EmeraudeEngine/emeraude-engine/blob/main/README.md) | Vulkan runtime & systems, **Vulkan SDK + platform packages**, engine submodules and options, the `EmEn::Core` contract, built-in shortcuts, remote console, RenderDoc, MoltenVK/platform troubleshooting |
| [`README.md`](README.md) (here) | what the demo teaches: human scale, the two lighting modes, the three post-processing layers, controls & settings, source map |

> [!IMPORTANT]
> **Do not restate dependency, toolchain or Vulkan-SDK detail in this README** — link to the
> owning level instead. The only sanctioned duplication is between projet-nihil and projet-alpha
> (both are entry doors, so both carry the copy-pasteable prerequisite one-liner).

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