# Projet-Nihil

![License](https://img.shields.io/badge/license-LGPLv3-blue.svg)
![Version](https://img.shields.io/badge/version-1.6.0-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)

The **showcase project** of [Emeraude Engine](https://github.com/EmeraudeEngine/emeraude-engine):
the smallest possible real application that still does something meaningful. One class,
`Application final : public EmEn::Core`, four lifecycle hooks, a complete 3D scene.

If you are discovering the engine, **start here**. This is a teaching artifact as much as a
program: it stays deliberately simple and readable, so you can follow `onCoreStarted()`
top-to-bottom and learn how an application is built.

```bash
git clone https://github.com/EmeraudeEngine/projet-nihil.git
cd projet-nihil
python3 build.py
./cmake-build-release/Release/projet-nihil
```

CMake clones the engine (which clones the foundation library) and downloads the prebuilt
external dependencies by itself. You manage none of it.

## Where to read what

```
ext-deps-generator  →  emeraude-base  →  emeraude-engine  →  projet-nihil
```

| Level | Documents |
|---|---|
| [`emeraude-base`](https://github.com/EmeraudeEngine/emeraude-base/blob/main/README.md) | The foundation library (`EmEn::Base`), the **toolchain requirements**, the compile policy, and **everything about external dependencies**. |
| [`emeraude-engine`](https://github.com/EmeraudeEngine/emeraude-engine/blob/main/README.md) | The Vulkan runtime: systems, **Vulkan SDK**, build options, the `EmEn::Core` contract, GPU debugging, troubleshooting. |
| **`projet-nihil`** (here) | This application: what it demonstrates, its scene, its controls, and how to read its source. |

> [!NOTE]
> Nothing about the engine's internals or the dependency machinery is repeated here. When
> something fails to **build**, the answer is in one of the two READMEs above; when something is
> unclear about **how to write an application**, it is here.

## Requirements

- The **toolchain** (CMake 3.25.1+, a C++20 compiler, Python 3, Git) —
  [see emeraude-base](https://github.com/EmeraudeEngine/emeraude-base/blob/main/README.md#requirements).
- The **Vulkan SDK 1.4+** and the platform packages —
  [see emeraude-engine](https://github.com/EmeraudeEngine/emeraude-engine/blob/main/README.md#requirements).

In short, on Debian 13+ / Ubuntu 24.04+ / Mint 22.3+:

```bash
sudo apt install build-essential cmake python3 ninja-build git \
    libvulkan-dev vulkan-tools vulkan-validationlayers vulkan-validationlayers-dev \
    libfontconfig-dev libwayland-dev libxkbcommon-dev xorg-dev
```

On macOS and Windows, install the Vulkan SDK 1.4+ at its default location (links in the
engine README). macOS also needs **MoltenVK ≥ 1.4**, otherwise the terrain renders pure
black — that specific trap is documented in the
[engine troubleshooting section](https://github.com/EmeraudeEngine/emeraude-engine/blob/main/README.md#macos-correct-sky-pure-black-terrain).

## Build and run

`build.py` configures a Release build with the right generator and architecture for your OS,
then builds it:

```bash
python3 build.py
```

Or drive CMake yourself — the cascade is fetched on the first configure either way:

```bash
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --config Release -j$(nproc)
```

The binary lands in `cmake-build-<config>/<Config>/projet-nihil` — `.exe` on Windows, a
`projet-nihil.app` bundle on macOS. Any CMake-aware IDE (CLion, Visual Studio, VSCode) works:
open the folder, let it configure, build.

You should see a dusk sky over a dark, polished stone terrain, a rotating porcelain cube, a
floating iridescent torus, bobbing gem and metal spheres lit by a low warm sun, and a camera
orbiting the stage.

## Controls

| Key | Action |
|---|---|
| **F1** | Show the application information dialog. |
| **Space** | Cycle the photographic look: *golden hour* → *silver noir* → *raw sensor* (see below), with an on-screen notification and a small chime synthesized by the engine. |

These two are the *application's* shortcuts — the ones registered in `Application::Application()`.
The engine contributes its own on top (`Shift+F1…F12`, `Shift+Escape`, screenshot, editor
mode…): see the
[engine shortcuts table](https://github.com/EmeraudeEngine/emeraude-engine/blob/main/README.md#built-in-shortcuts).

## Settings

| Key | Default | Effect |
|---|---|---|
| `App/UseSkyLighting` | `true` | Selects the lighting mode (see below). |

## What it demonstrates

Read `src/Application.cpp` in order — it is written as a guided tour:

| Hook | Role |
|---|---|
| `onBeforeCoreSecondaryServicesInitialization()` | Tweak services before they spin up. |
| `onCoreStarted(arguments, settings)` | **Build the scene**: skybox, terrain, camera, lights, nodes, materials, animations, post-processing. The bulk of the demo. |
| `onCoreProcessLogics(engineCycle)` | Per-cycle update loop — rotations, bobbing, camera aim. |
| `onCoreKeyRelease(key, scancode, modifiers)` | Input handling and application shortcuts. |

Along the way: a **photometric skybox authored in code** (luminance, ambient, a declared
setting sun), **resource loading** through the resource manager (meshes, PBR materials,
geometry), **procedural terrain** from Perlin noise with **terrain-relative placement**, a
**scene graph** of nodes with transforms, **keyframe animations** with interpolation, the
**Toolkit** scene builder, on-screen notifications (`Core::notifyUser()`), procedurally
generated background music, and a notification chime synthesized from scratch with
`WaveFactory::Synthesizer`.

### Everything is in metres, and that matters

The scene is deliberately **human-scale**: a 1 m cube, 35 cm spheres, a 60 cm torus, on a ~82 m
terrain, with the camera orbiting about 5 m out. It was rescaled down from a monumental version
for one reason: **photometric units only teach anything at a scale where the real numbers
apply.** At 300 m, the orbiting lamps needed ~10⁹ lumens to register against the sun —
arithmetically correct (`E = I/d²`), pedagogically backwards. At metres, they are ordinary
catalogue fixtures.

> [!WARNING]
> **All light quantities are photometric, in real-world units** — not `[0..1]` sliders. A
> directional light takes an **illuminance in lux** (direct sun 100 000, overcast 10 000, office
> ~500, full moon 0.25); a point or spot light takes a **luminous power in lumens** (household
> bulb 800, large floodlight ~100 000).

When editing the scene, know which numbers follow the scale and which do not:

| Follows the scale (world units) | Scale-independent |
|---|---|
| positions, radii, grid size, orbit radii, bob amplitudes, shadow coverage | **lux** (a directional light's illuminance) |
| Perlin `factor` (a displacement, in metres) | Perlin `size` (a frequency in UV space) |
| point/spot `radius` (a culling bound) | animation periods (**seconds**), cone **angles** (degrees) |
| | iridescence film thickness (**nanometres**), bloom threshold (**nits**), roughness/metalness |

### `+Y` is up, `-Z` is forward

The engine world is **right-handed and Y-up** (`+X` right, `+Y` up, `-Z` forward), the convention
of glTF 2.0, USD and FBX. Every vertical number in `onCoreStarted()` therefore reads directly as an
**altitude**: the cube rests 0.75 m above its patch of ground, the camera sweeps between 0.7 m
and 3.5 m. Only the verticals are concerned — X/Z, radii, angles and periods
mean the same thing in any convention. The full contract, and why the engine left Y-down, belongs
to the engine:
[`docs/coordinate-system.md`](https://github.com/EmeraudeEngine/emeraude-engine/blob/main/docs/coordinate-system.md).

### Two lighting modes, two halves of the model

`App/UseSkyLighting` picks between them, and each teaches something different:

| Mode | What it does |
|---|---|
| **Sky-driven** (`true`, default) | `Scene::applyBackgroundLighting()` — the engine derives the whole lighting from the background's photometric manifest: ambient (average sky colour × ambient illuminance, served by the baked IBL irradiance) plus one directional light per declared celestial body. The demo's dusk sky declares **one setting sun** (2700 K, 6000 lx, ~14° high), so this single call produces the warm ambient *and* a sun with a 4096² shadow map — the whole lighting, zero further authoring. Remove the star from the manifest and the same call falls back to pure image-based lighting. |
| **Dynamic** (`false`) | Hand-authored: a 6 000 lx setting sun with a 4096² shadow map, two animated orbiting 80–100 klm floodlights, a spotlight, and an explicit 2 000 lx warm ambient so shadowed sides are not black. |

> [!NOTE]
> The dynamic mode's sun is deliberately a **gentle 6 000 lx setting sun**, not a clear-day
> 100 000 lx. At 100 000 lx the hand-authored fixtures — the whole point of that mode —
> contribute ~2% and become invisible. Lighting is about **ratios**; raising the sun to
> `100'000.0F` to watch the coloured lamps vanish is the intended experiment.

### The sky is a light source you author

The engine's default skybox emits its sunset cubemap at a clear-day luminance and declares no
celestial body. The demo builds its own `DemoDuskSky` instead (the first block of
`onCoreStarted()`): the same cubemap, but a dusk luminance (1000 nits), a warm average colour,
an ambient illuminance, and one declared setting sun. This photometric manifest is exactly what
a JSON skybox would declare — the demo simply writes it in code.

### Nothing sits at an absolute height

The terrain is a Perlin surface rolling by ±1.5 m, so every decoration asks the scene's ground
— the same `GroundLevelInterface` the physics uses — for its local height
(`getLevelAt(x, z, deltaY)`) and sits **relative to it**; the camera orbit and the orbiting
lights clamp their keyframes above it. This is also why the ground is created with
`getOrCreateResourceSync()`: the placements need the terrain to exist immediately (the plain,
asynchronous variant would silently answer height 0 until loaded).

### Post-processing: three layers, only one is a matter of taste

`onCoreStarted()` separates them explicitly, and the **Space** shortcut moves only the third
(see `Application::buildPhotographicLooks()` and `applyLook()`):

1. **The sensor — tone mapping.** `camera->enableHDR(true)`. Mandatory, not an effect: the
   renderer produces physical radiance and a screen cannot show it. Without it, a daylight scene
   comes out pure white.
2. **The optics — bloom.** `camera->enableBloom(true)`, declared on the camera so the engine
   materializes it at the right place in the chain (after defocus, before the sensor). Its
   threshold is an absolute luminance in **nits**, its intensity the **fraction** of energy the
   glass scatters (a clean lens: a few percent) — not an artistic gain.
3. **The artistic pass** — god rays (scene chain) plus colour grading, vignetting, chromatic
   aberration and film grain (camera lens effects). The only creative layer, packaged as three
   ready-made **looks** the shortcut cycles: *golden hour* (warm grade, god rays), *silver noir*
   (monochrome, contrasty) and *raw sensor* (no artistic effect at all). Tone mapping and bloom
   never move, so every look stays photographically valid.

> [!IMPORTANT]
> Never toggle `Renderer::postProcessor().enable()` from an application: it is a global
> kill-switch that would take the tone mapper down with everything else, leaving a blown-out
> image. It defaults to ON and costs nothing when there is nothing to run. Toggle your own
> effects instead. Note also that lens effects are compiled **into** the composite shader, so
> disabling one means taking it off the camera (`clearLensEffects()`), not clearing a flag.

## Source layout

```
src/Application.{hpp,cpp}      the whole program — one class, four hooks
src/ApplicationSettingKeys.hpp setting keys and their defaults
src/Boot/{linux,mac,windows}/  per-OS entry points (the only platform-specific code)
src/config.hpp.in              CMake-generated configuration header
cmake/InstallEmeraudeEngine.cmake   clones the engine at configure time
resources/                     application assets
```

## License

**GNU Lesser General Public License v3.0 (LGPLv3)** — the same license as Emeraude Engine. See
[`LICENSE`](LICENSE).

Use it as the starting point of your own project: copy it, rename it, and replace the contents of
`onCoreStarted()`.

## Support

- **Issues:** [projet-nihil issues](https://github.com/EmeraudeEngine/projet-nihil/issues)
- **Engine discussions:** [GitHub Discussions](https://github.com/EmeraudeEngine/emeraude-engine/discussions)
- **AI assistants:** [`AGENTS.md`](AGENTS.md)
