message("[ProjetNihil] Enabling Emeraude-Engine framework...")

target_include_directories(${PROJECT_NAME} PUBLIC ${EMERAUDE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Emeraude)
