#!/usr/bin/env python3
import platform
import subprocess
import sys
import os

def run_command(command):
    """Runs a shell command and raises an error if it fails."""
    try:
        print(f"Running: {' '.join(command)}")
        subprocess.run(command, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {e}")
        sys.exit(1)

def main():
    print("Building Projet-Nihil ...")

    # Detect Platform
    os_name = platform.system()
    cmake_generator = ""
    extra_args = []

    if os_name == "Windows":
        cmake_generator = "Visual Studio 17 2022"
        extra_args = ["-A", "x64"]
    else:
        # Default to Ninja for Unix-like systems
        cmake_generator = "Ninja"

    # MacOS Specific explicit architecture handling
    if os_name == "Darwin":
        arch = platform.machine()
        print(f"Detected macOS architecture: {arch}")
        extra_args.append(f"-DCMAKE_OSX_ARCHITECTURES={arch}")

    build_dir = "cmake-build-release"
    
    # Configure
    print("\n======= CONFIGURING ======\n")
    configure_cmd = [
        "cmake",
        "-S", "./",
        "-B", f"./{build_dir}",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DEMERAUDE_ENABLE_TESTS=OFF",
        "-G", cmake_generator
    ] + extra_args
    run_command(configure_cmd)

    # Build
    print("\n======= COMPILING ======\n")
    build_cmd = [
        "cmake",
        "--build", f"./{build_dir}",
        "--config", "Release"
    ]
    run_command(build_cmd)

    print("\n======= FINISHED ======\n")

if __name__ == "__main__":
    main()
