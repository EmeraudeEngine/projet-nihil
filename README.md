# Projet-Nihil

## Introduction

Projet-Nihil is a starting point to use Emeraude-Engine.


## Requirements

- A C++20 compiler. This library is maintained from :
    - "Debian 13 (GNU/Linux)" using "G++ 14.2.0" compiler
    - "Ubuntu 24.04 LTS (GNU/Linux)" using "G++ 13.3.0" compiler
    - "Apple macOS Sequoia 15.5" using "Apple Clang 17.0" compiler and the minimal SDK version 12.0
    - "Microsoft Windows 11" using "MSVC 19.43.34812.0" compiler ("Visual Studio 2022 Community Edition")
- CMake 3.25.1+ to generate the project
- Python 3
- Vulkan SDK 1.4.309.0 from https://vulkan.lunarg.com/sdk/home


## Installation

First, clone the repository.

```bash
git clone https://github.com/EmeraudeEngine/projet-nihil.git
```

Next, in your favorite IDE compatible with CMake projects, you can open the project and let CMake do the configuration.
This will clone the Emeraude-Engine repository.

If you want to do it without an IDE, you can do the following:

```bash
cmake -S ./projet-nihil -B ./projet-nihil-build
```

### The external dependencies (precompiled binaries)

You need to manually download external dependencies for your system. This is not automated by now.

Here is the link: https://drive.google.com/drive/folders/1nDv35NuAPEg-XAGQIMZ7uCoqK3SK0VxZ?usp=drive_link

Download the zip file and extract it in the `./projet-nihil/dependencies/emeraude-engine/dependencies/` folder. The result should be something like:

For linux:
 - `./projet-nihil/dependencies/emeraude-engine/dependencies/linux.x86_64.Release`
 - `./projet-nihil/dependencies/emeraude-engine/dependencies/linux.x86_64.Debug`

or for macOS:
 - `./projet-nihil/dependencies/emeraude-engine/dependencies/mac.arm64.Release`
 - `./projet-nihil/dependencies/emeraude-engine/dependencies/mac.arm64.Debug`

or for Windows:
 - `./projet-nihil/dependencies/emeraude-engine/dependencies/windows.x86_64.Release-MD`
 - `./projet-nihil/dependencies/emeraude-engine/dependencies/windows.x86_64.Debug-MD`

### The Vulkan SDK

For Linux, use the SDK provided by your distribution.

For macOS, you can download it from https://sdk.lunarg.com/sdk/download/1.4.309.0/mac/vulkansdk-macos-1.4.309.0.zip

For Windows, you can download it from https://sdk.lunarg.com/sdk/download/1.4.309.0/windows/VulkanSDK-1.4.309.0-Installer.exe


## Compilation

You can now compile the project in your IDE. 

Or in the terminal:
```bash
cmake --build ./projet-nihil-build --config Release
```

At the end, you should normally see a basic scene on the screen when the application starts.


## Quick recap in the terminal

```bash
# Clone the base project
git clone https://github.com/EmeraudeEngine/projet-nihil.git

# Projet configuration
cmake -S ./projet-nihil -B ./projet-nihil-build

# Here, manually download the external dependencies.

# Compilation
cmake --build ./projet-nihil-build --config Release

# Execute the application
./projet-nihil-build/Release/projet-nihil
```
