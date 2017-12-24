project(arepa)

find_package(Boost COMPONENTS filesystem program_options system thread REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(SndFile REQUIRED sndfile)
pkg_search_module(ALSA REQUIRED alsa)
pkg_search_module(GTKMM REQUIRED gtkmm-2.4)

file(GLOB SRC_LIST "*.cpp")
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${SndFile_LIBRARIES} ${ALSA_LIBRARIES} ${Boost_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${GTKMM_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${GTKMM_CFLAGS_OTHER})
