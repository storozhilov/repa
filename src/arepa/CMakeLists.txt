project(arepa)

find_package(Boost COMPONENTS filesystem program_options system thread REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(SndFile REQUIRED sndfile)
pkg_search_module(ALSA REQUIRED alsa)
pkg_search_module(GTKMM REQUIRED gtkmm-2.4)

file(GLOB AREPA_GTK2_SRC_LIST "*.cpp" "arepa-gtk2/*.cpp")
set(AREPA_GTK2_BINARY "${PROJECT_NAME}-gtk2")
add_executable(${AREPA_GTK2_BINARY} ${AREPA_GTK2_SRC_LIST})
target_link_libraries(${AREPA_GTK2_BINARY} ${GTKMM_LIBRARIES} ${SndFile_LIBRARIES} ${ALSA_LIBRARIES} ${Boost_LIBRARIES})
target_include_directories(${AREPA_GTK2_BINARY} PUBLIC ${GTKMM_INCLUDE_DIRS})
target_compile_options(${AREPA_GTK2_BINARY} PUBLIC ${GTKMM_CFLAGS_OTHER})
