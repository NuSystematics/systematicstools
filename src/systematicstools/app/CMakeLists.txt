set(SYSTTOOLS_APPS
	ConvertFHiCLToYAML)

add_executable(ConvertFHiCLToYAML ConvertFHiCLToYAML.cxx)
target_link_libraries(ConvertFHiCLToYAML PRIVATE
	systtools::utility
	systtools::commondeps)

install(TARGETS ${SYSTTOOLS_APPS}
	RUNTIME DESTINATION bin COMPONENT Runtime)

