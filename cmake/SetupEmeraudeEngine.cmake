message("[ProjetNihil] Enabling EmeraudeEngine framework ...")

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

target_include_directories(${PROJECT_NAME} PUBLIC ${EMERAUDE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Emeraude)
