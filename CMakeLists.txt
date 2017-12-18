cmake_minimum_required(VERSION 3.2)

add_subdirectory(src)
add_subdirectory(tests)

enable_testing()
add_test(NAME test_repa
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/test_repa)
