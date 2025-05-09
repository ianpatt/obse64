option(OBSE_COPY_OUTPUT "Copies output files to the Oblivion Remastered directory" OFF)

if (OBSE_COPY_OUTPUT)
	add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "$ENV{Oblivion64Path}"
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> "$ENV{Oblivion64Path}"
	)
endif()
