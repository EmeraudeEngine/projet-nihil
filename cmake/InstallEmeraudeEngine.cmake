message("Installing Emeraude-Engine library ...")

set(EMERAUDE_ENGINE_GIT "https://github.com/EmeraudeEngine/emeraude-engine.git")
set(EMERAUDE_ENGINE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/emeraude-engine)

if ( NOT EXISTS ${EMERAUDE_ENGINE_PATH} )
	find_package(Git REQUIRED)

	execute_process(
		COMMAND ${GIT_EXECUTABLE}
		clone --branch master --recurse-submodules ${EMERAUDE_ENGINE_GIT} ${EMERAUDE_ENGINE_PATH}
		COMMAND_ERROR_IS_FATAL ANY
	)
else ()
	message("The Emeraude-Engine repository is present !")
endif ()
