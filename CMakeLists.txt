#######################################################################
# CMake configuration
cmake_minimum_required(VERSION 3.25.1)
cmake_policy(SET CMP0042 NEW) # MACOSX_RPATH is enabled by default.
cmake_policy(SET CMP0091 NEW) # MSVC runtime library flags are selected by an abstraction.

# Adds custom CMake scripts/modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/dependencies/emeraude-engine/cmake)

message("[ProjetNihil] Using CMake ${CMAKE_VERSION}")



#######################################################################
# Configure the parallel job count
cmake_host_system_information(RESULT DETECTED_JOB_COUNT QUERY NUMBER_OF_PHYSICAL_CORES)

set(JOB_COUNT ${DETECTED_JOB_COUNT} CACHE STRING "The number of job used for compilation. Default auto.")
set(CMAKE_BUILD_PARALLEL_LEVEL ${JOB_COUNT})

message("[ProjetNihil] Job processes used : ${CMAKE_BUILD_PARALLEL_LEVEL}")



#######################################################################
# NOTE: This must be set before anything for macos x86_x64, to be sure all targets use the right platform and SDK.
if ( APPLE )
	# DO NOT SET A DEFAULT VALUE HERE !!!
	set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "The macOS architecture targeted. Must be manually specified (arm64 or x86_64) !")

	if ( CMAKE_OSX_ARCHITECTURES STREQUAL "" )
		message(FATAL_ERROR
			"[ProjetNihil] macOS compilation requires to manually set the architecture ! Use "
			"-DCMAKE_OSX_ARCHITECTURES=arm64 for 'Apple silicon CPU' or "
			"-DCMAKE_OSX_ARCHITECTURES=x86_64 for Intel CPU."
		)
	endif ()
endif ()

#######################################################################
# Declare project and options.
project(
	projet-nihil
	VERSION 1.3.0
	DESCRIPTION "Projet-Nihil is a starting point to use Emeraude-Engine."
	HOMEPAGE_URL "https://github.com/EmeraudeEngine/projet-nihil"
	LANGUAGES C CXX
)

set(CMAKE_VERBOSE_MAKEFILE On)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

# Configure output directory by compilation type.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

set_property(GLOBAL PROPERTY USE_FOLDERS On)



# Projet-Nihil application options.
set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

if ( UNIX AND NOT APPLE )
	message("[ProjetNihil] Linux platform detected !")

	set(APP_ARCH "x86_64" CACHE STRING "Target architecture for Linux.")

	message("[ProjetNihil] Linux platform targets arch ${APP_ARCH}")

	set(PROJECT_COMPILATION_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}" CACHE STRING "")
	set(PROJECT_INSTALL_DIR "." CACHE STRING "")
elseif ( APPLE )
	message("[ProjetNihil] Apple platform detected !")

	if ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "x86_64" )
		set(APP_ARCH "x86_64" CACHE STRING "Target architecture for macOS.")
		set(APP_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum SDK for macOS (Intel).")
	elseif ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64" )
		set(APP_ARCH "arm64" CACHE STRING "Target architecture for macOS.")
		set(APP_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum SDK for macOS (Apple silicon).")
	else ()
		message(FATAL_ERROR "Please specify the architecture for macOS with -DCMAKE_OSX_ARCHITECTURES=x86_64 or -DCMAKE_OSX_ARCHITECTURES=arm64 !")
	endif ()

	message("[ProjetNihil] Apple platform targets arch ${APP_ARCH} with SDK ${APP_OSX_DEPLOYMENT_TARGET}")

	set(PROJECT_COMPILATION_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.app" CACHE STRING "")
	set(PROJECT_INSTALL_DIR "./${PROJECT_NAME}.app" CACHE STRING "")

	set(CMAKE_OSX_ARCHITECTURES ${APP_ARCH})
	set(CMAKE_OSX_DEPLOYMENT_TARGET ${APP_OSX_DEPLOYMENT_TARGET})
elseif ( MSVC )
	message("[ProjetNihil] Windows platform detected !")

	set(APP_ARCH "x86_64" CACHE STRING "Target architecture for Windows.")

	message("[ProjetNihil] Windows platform targets arch ${APP_ARCH}")

	set(PROJECT_COMPILATION_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}" CACHE STRING "")
	set(PROJECT_INSTALL_DIR "." CACHE STRING "")

	# NOTE: In Debug we are using the \MDd (dynamic debug) flag and in Release we are using \MT (static release).
	if ( CMAKE_BUILD_TYPE MATCHES Debug )
		message(STATUS "[ProjetNihil] Forcing dynamic MSVC Runtime library for Debug builds!")

		set(CEF_RUNTIME_LIBRARY_FLAG "/MD" CACHE STRING "" FORCE) # NOTE: Do not add 'd' here, CEF CMakelists.txt will do it.
	else ()
		message(STATUS "[ProjetNihil] Forcing static MSVC Runtime library for Release builds!")

		set(CEF_RUNTIME_LIBRARY_FLAG "/MT" CACHE STRING "" FORCE)
	endif ()

	if ( NOT CMAKE_MSVC_RUNTIME_LIBRARY )
		# Set /MT for Release or /MDd for Debug to compiler flags (Mixed behavior).
		set(CMAKE_MSVC_RUNTIME_LIBRARY "$<IF:$<CONFIG:Debug>,MultiThreadedDebugDLL,MultiThreaded>")
	endif ()

	# Remove default MSVC flags added by CMake to avoid D9025 warnings
	# These options are controlled by Emeraude's EMERAUDE_COMPILE_OPTIONS
	string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif ()



#######################################################################
# Configure prerequisites.
include(InstallEmeraudeEngine)



#######################################################################
# Project header/sources files for main binary.
message("[ProjetNihil] Gather headers/sources from directory : ${PROJECT_SOURCE_DIR}")

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
	message("[ProjetNihil] Configuring headers/sources for Linux compilation ...")

	list(APPEND MAIN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/Boot/linux/main.cpp)
elseif ( APPLE )
	message("[ProjetNihil] Configuring headers/sources for macOS compilation ...")

	list(APPEND MAIN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/Boot/mac/main.mm)
elseif ( MSVC )
	message("[ProjetNihil] Configuring headers/sources for Windows compilation ...")

	list(APPEND MAIN_SOURCE_FILES ${PROJECT_SOURCE_DIR}/Boot/windows/main.cpp)
endif ()

# Debug source file
set(SHOW_SOURCES_FILES Off CACHE BOOL "Show project header/source files found to cmake output.")

if ( ${SHOW_SOURCES_FILES} )
	string(REPLACE ";" "\n" LIST "${MAIN_HEADER_FILES}")
	message("[ProjetNihil] Main application headers :\n ${LIST}")

	string(REPLACE ";" "\n" LIST "${MAIN_SOURCE_FILES}")
	message("[ProjetNihil] Main application sources :\n ${LIST}")
endif ()

# Create the executable
if ( MSVC )
	message("[ProjetNihil] Configuring C++ compiler (>= MSVC 2022) for Windows ...")

	add_executable(${PROJECT_NAME} WIN32 ${MAIN_HEADER_FILES} ${MAIN_SOURCE_FILES})
elseif ( APPLE )
	message("[ProjetNihil] Configuring C++ compiler (>= AppleClang 17.0) for macOS ...")

	add_executable(${PROJECT_NAME} ${MAIN_HEADER_FILES} ${MAIN_SOURCE_FILES})
elseif ( UNIX )
	message("[ProjetNihil] Configuring C++ compiler (>= G++13.3) for Linux ...")

	add_executable(${PROJECT_NAME} ${MAIN_HEADER_FILES} ${MAIN_SOURCE_FILES})
else ()
	message(FATAL_ERROR "[ProjetNihil] Unsupported platform!")
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED On
	C_EXTENSIONS Off
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED On
	CXX_EXTENSIONS Off
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ${EMERAUDE_COMPILE_DEFINITIONS})

message("[ProjetNihil] '${PROJECT_NAME}' will be compiled with definitions: ${EMERAUDE_COMPILE_DEFINITIONS}")

target_compile_options(${PROJECT_NAME} PRIVATE ${EMERAUDE_COMPILE_OPTIONS})

message("[ProjetNihil] '${PROJECT_NAME}' will be compiled with flags: ${EMERAUDE_COMPILE_OPTIONS}")

target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}
	${CMAKE_BINARY_DIR}/include
)



#######################################################################
# Configure dependencies

# Configure "EmeraudeEngine" for the window management.
include(SetupEmeraudeEngine)



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
