# Projet-Nihil

![License](https://img.shields.io/badge/license-LGPLv3-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)

A complete working example demonstrating how to use [Emeraude Engine](https://github.com/EmeraudeEngine/emeraude-engine) to build 3D applications. This project serves as a starting point and reference implementation for developers getting started with the engine.

## What This Project Demonstrates

- **Application lifecycle:** Proper initialization, update loop, and shutdown patterns
- **Scene management:** Creating and configuring 3D scenes with skybox and terrain
- **Resource loading:** Using the resource manager for meshes, materials, and geometry
- **Camera system:** Setting up an animated camera with smooth interpolation
- **Lighting:** Configuring both static and dynamic lighting setups
- **Scene nodes:** Creating and manipulating scene graph nodes with transforms
- **Animations:** Implementing keyframe-based animations with interpolation
- **Procedural generation:** Generating terrain using Perlin noise
- **Input handling:** Processing keyboard events and implementing application shortcuts


## Requirements

### Build Tools
- **CMake:** 3.25.1 or higher
- **Python:** 3.0 or higher
- **C++20 Compiler:**
  - **Linux:** GCC 13.3.0+ (Ubuntu 24.04 LTS) or GCC 14.2.0+ (Debian 13)
  - **macOS:** Apple Clang 17.0+ with SDK 12.0+ (tested on macOS Sequoia 15.5)
  - **Windows:** MSVC 19.43+ / Visual Studio 2022

### Dependencies
- **Vulkan SDK:** 1.4.309.0 from [vulkan.lunarg.com](https://vulkan.lunarg.com/sdk/home)
- **Emeraude Engine:** Automatically cloned as a git submodule during configuration
- **Precompiled libraries:** Required for engine dependencies (see installation steps below)


## Installation

### Step 1: Install Vulkan SDK

**Linux:**
```bash
sudo apt install libvulkan-dev vulkan-tools vulkan-validationlayers vulkan-validationlayers-dev
```

**macOS:**
```bash
# Download from:
https://sdk.lunarg.com/sdk/download/1.4.309.0/mac/vulkansdk-macos-1.4.309.0.zip
```

**Windows:**
```bash
# Download and run installer from:
https://sdk.lunarg.com/sdk/download/1.4.309.0/windows/VulkanSDK-1.4.309.0-Installer.exe
```

### Step 2: Clone the Repository

```bash
git clone https://github.com/EmeraudeEngine/projet-nihil.git
cd projet-nihil
```

### Step 3: Download Precompiled Dependencies

Download the precompiled libraries for your platform from [Google Drive](https://drive.google.com/drive/folders/1nDv35NuAPEg-XAGQIMZ7uCoqK3SK0VxZ?usp=drive_link).

Extract the archive into `./dependencies/emeraude-engine/dependencies/`:

**Linux:**
```
./dependencies/emeraude-engine/dependencies/
├── linux.x86_64.Release/
└── linux.x86_64.Debug/
```

**macOS:**
```
./dependencies/emeraude-engine/dependencies/
├── mac.arm64.Release/
└── mac.arm64.Debug/
```

**Windows:**
```
./dependencies/emeraude-engine/dependencies/
├── windows.x86_64.Release-MD/
└── windows.x86_64.Debug-MD/
```

*Note*: If you want to compile external dependencies manually, you can use : https://github.com/EmeraudeEngine/ext-deps-generator

### Step 4: Configure and Build

**Using an IDE (CLion, Visual Studio, VSCode):**
- Open the project folder
- Let CMake configure automatically
- Build the Release configuration

**Using command line:**
```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
./build/Release/projet-nihil  # Linux/macOS
.\build\Release\projet-nihil.exe  # Windows
```

You should see a 3D scene with a rotating cube on procedurally generated terrain.

## Usage

### Controls

- **F1:** Show application information dialog
- **ESC:** Exit application (standard window close)

### Code Structure

The main application code is located in `src/Application.cpp`. Key methods:

- `onBeforeCoreSecondaryServicesInitialization()`: Called before graphics/audio initialization
- `onCoreStarted()`: Main setup - create scenes, cameras, lights, and objects
- `onCoreProcessLogics()`: Called every frame - update game logic
- `onCoreKeyPress()` / `onCoreKeyRelease()`: Handle keyboard input

## Learning from the Code

### Scene Setup (Application.cpp:107-113)
```cpp
const auto newScene = this->sceneManager().newScene(
    "EmptyScene",
    1000.0F,           // View distance
    defaultSkyBox,     // Skybox resource
    defaultSceneArea,  // Ground/terrain
    nullptr            // Optional background music
);
```

### Creating Scene Nodes with Camera (Application.cpp:117-119)
```cpp
const auto sceneNode = newScene->root()->createChild(
    "TheCameraNode",
    Math::CartesianFrame{-512.0F, -80.0F, 256.0F}
);
sceneNode->componentBuilder< Component::Camera >("TheCamera").asPrimary().build(true);
sceneNode->lookAt(Math::Vector< 3, float >{0.0F, -75.0F, 0.0F}, false);
```

### Procedural Terrain Generation (Application.cpp:93-104)
```cpp
const auto defaultSceneArea = resourceManager.container< Renderable::BasicFloorResource >()
    ->getOrCreateResource("TheSceneArea", [&resourceManager] (Renderable::BasicFloorResource & newResource) {
        constexpr auto Boundary{4096.0F};
        const auto materialResource = resourceManager.container< Material::BasicResource >()
            ->getDefaultResource();

        if ( !newResource.loadPerlinNoise(Boundary, 256, 10.0F, 200.0F, materialResource, 1.0F) )
        {
            return false;
        }

        return true;
    });
```

### Animation System (Application.cpp:130-149)
```cpp
const auto interpolation = std::make_shared< Animations::Sequence >(30'000);  // 30 seconds

for ( uint32_t index = 0; index <= segmentCount; ++index ) {
    const auto timePoint = static_cast< float >(index) / static_cast< float >(segmentCount);
    const auto currentAngle = timePoint * (2.0F * std::numbers::pi_v< float >);

    const Math::Vector< 3, float > position{
        radius * std::cos(currentAngle),
        yCenter + (yAmplitude * std::cos(currentAngle * 2.0F)),
        radius * std::sin(currentAngle)
    };

    interpolation->addKeyFrame(timePoint, Variant{position}, Math::InterpolationType::Linear);
}

interpolation->play();  // Start the animation
sceneNode->addAnimation(Scenes::Node::WorldPosition, interpolation);
```

## Additional Resources

- **Engine Documentation:** [Emeraude Engine README](https://github.com/EmeraudeEngine/emeraude-engine)
- **API Reference:** Coming soon
- **Community:** [GitHub Discussions](https://github.com/EmeraudeEngine/emeraude-engine/discussions)

## License

This project is licensed under the **GNU Lesser General Public License v3.0 (LGPLv3)** - the same license as Emeraude Engine.

See [LICENSE](LICENSE) for full details.
