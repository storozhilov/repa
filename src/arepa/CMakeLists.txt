project(arepa)

find_package(Boost COMPONENTS filesystem program_options system thread REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(SndFile REQUIRED sndfile)
pkg_search_module(ALSA REQUIRED alsa)
pkg_search_module(GTKMM REQUIRED gtkmm-2.4)

set(CMAKE_CXX_STANDARD 11)

file(GLOB AREPA_GTK2_SRC_LIST "*.cpp" "gtk2/*.cpp")
set(AREPA_GTK2_BINARY "${PROJECT_NAME}-gtk2")
add_executable(${AREPA_GTK2_BINARY} ${AREPA_GTK2_SRC_LIST})
target_link_libraries(${AREPA_GTK2_BINARY} ${GTKMM_LIBRARIES} ${SndFile_LIBRARIES} ${ALSA_LIBRARIES} ${Boost_LIBRARIES})
target_include_directories(${AREPA_GTK2_BINARY} PUBLIC ${GTKMM_INCLUDE_DIRS})

file(GLOB AREPA_CLI_SRC_LIST "*.cpp" "cli/*.cpp")
set(AREPA_CLI_BINARY "${PROJECT_NAME}-cli")
add_executable(${AREPA_CLI_BINARY} ${AREPA_CLI_SRC_LIST})
target_link_libraries(${AREPA_CLI_BINARY} ${SndFile_LIBRARIES} ${ALSA_LIBRARIES} ${Boost_LIBRARIES})
