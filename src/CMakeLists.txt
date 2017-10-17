project(repa)

find_package(Boost COMPONENTS filesystem program_options system thread REQUIRED)
exec_program(pkg-config ARGS --libs sndfile OUTPUT_VARIABLE SndFile_LIBRARIES)
exec_program(pkg-config ARGS --libs alsa OUTPUT_VARIABLE ALSA_LIBRARIES)

file(GLOB SRC_LIST "*.cpp")
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${SndFile_LIBRARIES} ${ALSA_LIBRARIES} ${Boost_LIBRARIES})
