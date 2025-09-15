message("[ProjetNihil] Installing Emeraude-Engine library ...")

set(EMERAUDE_ENGINE_GIT "https://github.com/EmeraudeEngine/emeraude-engine.git")
set(EMERAUDE_ENGINE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/emeraude-engine)

if ( NOT EXISTS ${EMERAUDE_ENGINE_PATH} )
	find_package(Git REQUIRED)

	execute_process(
		COMMAND ${GIT_EXECUTABLE}
		clone --branch main --recurse-submodules ${EMERAUDE_ENGINE_GIT} ${EMERAUDE_ENGINE_PATH}
		COMMAND_ERROR_IS_FATAL ANY
	)
else ()
	message("[ProjetNihil] The Emeraude-Engine repository is present !")
endif ()

if ( CMAKE_BUILD_TYPE MATCHES Release )
	# For Release, we use the Static MSVC Runtime (/MT),
	set(EMERAUDE_USE_STATIC_RUNTIME ON)
else ()
	# For Debug, we use the Dynamic MSVC Runtime (/MDd) to avoid false positives from the debugger.
	set(EMERAUDE_USE_STATIC_RUNTIME OFF)
endif ()

set(EMERAUDE_COMPILATION_DIR "${PROJECT_COMPILATION_DIR}" CACHE STRING "" FORCE)
set(EMERAUDE_INSTALL_DIR "${PROJECT_INSTALL_DIR}" CACHE STRING "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/emeraude-engine Emeraude EXCLUDE_FROM_ALL)
