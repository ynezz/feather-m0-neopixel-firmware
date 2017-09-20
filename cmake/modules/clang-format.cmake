file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.cxx *.c *.h)
find_program(
	CLANG_FORMAT_COMMAND
	NAMES
		clang-format clang-format-3.9
		clang-format-3.8 clang-format-3.7
		clang-format-3.6)
add_custom_target(
	clang-format
	COMMAND ${CLANG_FORMAT_COMMAND} -i ${ALL_SOURCE_FILES}
)
