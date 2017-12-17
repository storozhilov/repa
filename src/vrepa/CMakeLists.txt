project(vrepa)

find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GSTREAMERMM REQUIRED gstreamermm-1.0)
pkg_search_module(GTKMM REQUIRED gtkmm-2.4)

file(GLOB SRC_LIST "*.cpp")
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${GSTREAMERMM_LIBRARIES} ${GTKMM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PUBLIC ${GSTREAMERMM_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${GSTREAMERMM_CFLAGS_OTHER})
