#######################################################################
# CMake configuration
cmake_minimum_required(VERSION 3.25.1)
cmake_policy(SET CMP0042 NEW) # MACOSX_RPATH is enabled by default.
cmake_policy(SET CMP0091 NEW) # MSVC runtime library flags are selected by an abstraction.

# Adds custom CMake scripts/modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/dependencies/emeraude-engine/cmake)

message("Using CMake ${CMAKE_VERSION}")



#######################################################################
# Configure the parallel job count
cmake_host_system_information(RESULT DETECTED_JOB_COUNT QUERY NUMBER_OF_PHYSICAL_CORES)

set(JOB_COUNT ${DETECTED_JOB_COUNT} CACHE STRING "The number of job used for compilation. Default auto.")
set(CMAKE_BUILD_PARALLEL_LEVEL ${JOB_COUNT})

message("Job processes used : ${CMAKE_BUILD_PARALLEL_LEVEL}")



#######################################################################
# NOTE: This must be set before anything for macos x86_x64, to be sure all targets use the right platform and SDK.
if ( APPLE )
	# DO NOT SET A DEFAULT VALUE HERE !!!
	set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "The macOS architecture targeted. Must be manually specified (arm64 or x86_64) !")

	if ( CMAKE_OSX_ARCHITECTURES STREQUAL "" )
		message(FATAL_ERROR
			"macOS compilation requires to manually set the architecture ! Use "
			"-DCMAKE_OSX_ARCHITECTURES=arm64 for 'Apple silicon CPU' or "
			"-DCMAKE_OSX_ARCHITECTURES=x86_64 for Intel CPU."
		)
	endif ()
endif ()

#######################################################################
# Declare project and options.
project(
	projet-nihil
	VERSION 1.0.0
	DESCRIPTION "Projet-Nihil is a starting point to use Emeraude-Engine."
	HOMEPAGE_URL "https://github.com/EmeraudeEngine/projet-nihil"
	LANGUAGES C CXX
)

set(CMAKE_VERBOSE_MAKEFILE On)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})

set_property(GLOBAL PROPERTY USE_FOLDERS On)

# Print out project paths
message("[${PROJECT_NAME}] Source directory : ${CMAKE_SOURCE_DIR}, current : ${CMAKE_CURRENT_SOURCE_DIR}")
message("[${PROJECT_NAME}] Binary directory : ${CMAKE_BINARY_DIR}, current : ${CMAKE_CURRENT_BINARY_DIR}")
message("[${PROJECT_NAME}] Install directory : ${CMAKE_INSTALL_PREFIX}")

# Projet-Nihil application options.
set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

if ( UNIX AND NOT APPLE )
	message("Linux platform detected !")

	set(APP_ARCH "x86_64" CACHE STRING "Target architecture for Linux.")
	set(LOCAL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/emeraude-engine/dependencies/linux.${APP_ARCH}-${CMAKE_BUILD_TYPE} CACHE STRING "Final directory for local libraries.")
	set(PROJECT_COMPILATION_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}" CACHE STRING "")
	set(PROJECT_INSTALL_DIR "." CACHE STRING "")
elseif ( APPLE )
	message("Apple platform detected !")

	if ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "x86_64" )
		set(APP_ARCH "x86_64" CACHE STRING "Target architecture for macOS.")
		set(APP_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum SDK for macOS (Intel).")
	elseif ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64" )
		set(APP_ARCH "arm64" CACHE STRING "Target architecture for macOS.")
		set(APP_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum SDK for macOS (Apple silicon).")
	else ()
		message(FATAL_ERROR "Please specify the architecture for macOS with -DCMAKE_OSX_ARCHITECTURES=x86_64 or -DCMAKE_OSX_ARCHITECTURES=arm64 !")
	endif ()

	set(CMAKE_OSX_ARCHITECTURES ${APP_ARCH})
	set(CMAKE_OSX_DEPLOYMENT_TARGET ${APP_OSX_DEPLOYMENT_TARGET})

	message("macOS target arch ${CMAKE_OSX_ARCHITECTURES} with SDK ${CMAKE_OSX_DEPLOYMENT_TARGET}")

	set(LOCAL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/emeraude-engine/dependencies/mac.${APP_ARCH}-${CMAKE_BUILD_TYPE} CACHE STRING "Final directory for local libraries for macOS.")
	set(PROJECT_COMPILATION_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.app" CACHE STRING "")
	set(PROJECT_INSTALL_DIR "./${PROJECT_NAME}.app" CACHE STRING "")
elseif ( MSVC )
	message("Windows platform detected !")

	# FIXME: Pass these info directly to MSVC via flags
	set(CMAKE_C_STANDARD 17)
	set(CMAKE_C_STANDARD_REQUIRED On)
	set(CMAKE_C_EXTENSIONS On)
	set(CMAKE_C_FLAGS_DEBUG "-DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG")
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED On)
	set(CMAKE_CXX_EXTENSIONS On)
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")

	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	set(APP_ARCH "x86_64" CACHE STRING "Target architecture for Windows.")
	set(LOCAL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/emeraude-engine/dependencies/windows.${APP_ARCH}-${CMAKE_BUILD_TYPE} CACHE STRING "Final directory for local libraries for Windows.")
	set(PROJECT_COMPILATION_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}" CACHE STRING "")
	set(PROJECT_INSTALL_DIR "." CACHE STRING "")
endif ()



#######################################################################
# Configure prerequisites.
include(InstallEmeraudeEngine)



#######################################################################
# Project header/sources files for main binary.
message("Gather headers/sources from directory : ${PROJECT_SOURCE_DIR}")

# Main application header/source files.
set(MAIN_HEADER_FILES
	${PROJECT_SOURCE_DIR}/Application.hpp
	${PROJECT_SOURCE_DIR}/ApplicationSettingKeys.hpp
)

set(MAIN_SOURCE_FILES
	${PROJECT_SOURCE_DIR}/Application.cpp
)

# Headers and source files per OS
if ( UNIX AND NOT APPLE )
	message("Configuring headers/sources for Linux compilation ...")

	list(APPEND MAIN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/Boot/linux/main.cpp)
elseif ( APPLE )
	message("Configuring headers/sources for macOS compilation ...")

	list(APPEND MAIN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/Boot/mac/main.mm)
elseif ( MSVC )
	message("Configuring headers/sources for Windows compilation ...")

	list(APPEND MAIN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/Boot/windows/main.cpp)
endif ()

# Debug source file
set(SHOW_SOURCES_FILES On CACHE BOOL "Show project header/source files found to cmake output.")

if ( ${SHOW_SOURCES_FILES} )
	string(REPLACE ";" "\n" LIST "${MAIN_HEADER_FILES}")
	message("Main application headers :\n ${LIST}")

	string(REPLACE ";" "\n" LIST "${MAIN_SOURCE_FILES}")
	message("Main application sources :\n ${LIST}")
endif ()

# Create the executable
add_executable(${PROJECT_NAME} ${MAIN_HEADER_FILES} ${MAIN_SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED On
	C_EXTENSIONS On
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED On
	CXX_EXTENSIONS On
)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:NDEBUG>)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include)

if ( MSVC )
	message("Configuring C++ compiler (>= MSVC 2022) for Windows ...")

	target_compile_options(${PROJECT_NAME} PRIVATE
		/wd4100       # Ignore "unreferenced formal parameter" warning
		/wd4127       # Ignore "conditional expression is constant" warning
		/wd4244       # Ignore "conversion possible loss of data" warning
		/wd4324       # Ignore "structure was padded due to alignment specifier" warning
		/wd4481       # Ignore "nonstandard extension used: override" warning
		/wd4512       # Ignore "assignment operator could not be generated" warning
		/wd4701       # Ignore "potentially uninitialized local variable" warning
		/wd4702       # Ignore "unreachable code" warning
		/wd4996       # Ignore "function or variable may be unsafe" warning
		/MP${CMAKE_BUILD_PARALLEL_LEVEL} # Multiprocess compilation
		/Gy           # Enable function-level linking
		/EHsc		  # Enable exceptions
		/W4           # Warning level 4
		/Ob2
		$<$<CONFIG:Debug>:-DDEBUG
			/RTC1         # Disable optimizations
			/Od           # Enable basic run-time checks
			/WX           # Treat warnings as errors
			/Zi
		>
		$<$<CONFIG:Relase>:-DNDEBUG
			/O2           # Optimize for maximum speed
			/Ob2          # Inline any suitable function
			/GF           # Enable string pooling
		>
	)
elseif ( UNIX )
	if ( APPLE )
		message("Configuring C++ compiler (>= AppleClang 17.0) for macOS ...")

		target_compile_options(${PROJECT_NAME} PRIVATE
				-Wno-unknown-pragmas
				-Wno-unused
				-fno-exceptions
				-Wno-nullability-extension # Removes warnings from external dependencies libzip.
				$<$<CONFIG:Debug>:
					-O0
					-g
					-Wall
					-Wextra
					-Wpedantic
					-Werror
				>
				$<$<CONFIG:Relase>:
					-O2
					-ffast-math
					-fdata-sections
					-ffunction-sections
				>
		)
	else ()
		message("Configuring C++ compiler (>= G++13.3) for Linux ...")

		target_compile_options(${PROJECT_NAME} PRIVATE
			-Wno-unknown-pragmas
			-Wno-unused
			-fno-exceptions
			$<$<CONFIG:Debug>:
				-DDEBUG
				-O0
				-g
				-Wall
				-Wextra
				-Wpedantic
				-Werror
			>
			$<$<CONFIG:Relase>:
				-DNDEBUG
				-O2
				-ffast-math
				-fdata-sections
				-ffunction-sections
			>
		)
	endif ()
else ()
	message(FATAL_ERROR "Unsupported platform!")
endif ()



#######################################################################
# Emeraude-engine library.
set(EMERAUDE_COMPILATION_DIR "${PROJECT_COMPILATION_DIR}" CACHE STRING "" FORCE)
set(EMERAUDE_INSTALL_DIR "${PROJECT_INSTALL_DIR}" CACHE STRING "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/emeraude-engine Emeraude EXCLUDE_FROM_ALL)

message("Emeraude-Engine source directory : ${EMERAUDE_SOURCE_DIR}")
message("Emeraude-Engine dependencies directory : ${EMERAUDE_DEPENDENCIES_DIR}")
message("Emeraude-Engine local libraries directory : ${LOCAL_LIB_DIR}")

target_include_directories(${PROJECT_NAME} PRIVATE ${EMERAUDE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Emeraude)



#######################################################################
# Resources and configuration files

# Write the project C++ config file.
configure_file(${PROJECT_SOURCE_DIR}/config.hpp.in ${CMAKE_BINARY_DIR}/include/config.hpp)

# Resource files
if ( APPLE )
	set(PROJECT_DATA_DIR ${PROJECT_COMPILATION_DIR}/Contents/Resources)
else ()
	set(PROJECT_DATA_DIR ${PROJECT_COMPILATION_DIR})
endif ()

# Copy all application data files into the build directory.
# NOTE: This is done once per cmake configuration.
file(COPY ${CMAKE_SOURCE_DIR}/resources/data DESTINATION ${PROJECT_DATA_DIR})



#######################################################################
# Installing

# TODO ...