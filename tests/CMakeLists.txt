project(test_repa)

set(CMAKE_CXX_STANDARD 11)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
	gtest
	URL https://github.com/google/googletest/archive/master.zip
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
	# Disable install step
	INSTALL_COMMAND "")

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

set_target_properties(libgtest PROPERTIES
	"IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
	"IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")

add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

set_target_properties(libgmock PROPERTIES
	"IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
	"IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")

# Audio-repa testing binary setup
find_package(Boost COMPONENTS filesystem program_options system thread REQUIRED)
exec_program(pkg-config ARGS --libs sndfile OUTPUT_VARIABLE SndFile_LIBRARIES)
exec_program(pkg-config ARGS --libs alsa OUTPUT_VARIABLE ALSA_LIBRARIES)

file(GLOB TEST_AREPA_SRC_LIST "test_arepa.cpp" "arepa/unit/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/arepa/AudioProcessor.cpp"
	"${CMAKE_SOURCE_DIR}/src/arepa/CaptureChannel.cpp"
	"${CMAKE_SOURCE_DIR}/src/arepa/Indicator.cpp")
add_executable(test_arepa ${TEST_AREPA_SRC_LIST})

add_dependencies(test_arepa gtest)
include_directories(
	"${source_dir}/googletest/include"
	"${source_dir}/googlemock/include"
	"${CMAKE_SOURCE_DIR}/src/arepa")

target_link_libraries(test_arepa 
	${SndFile_LIBRARIES} ${ALSA_LIBRARIES} ${Boost_LIBRARIES}
	libgtest libgmock pthread)

# TODO: Video-repa testing binary setup
