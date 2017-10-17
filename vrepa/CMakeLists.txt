project(vrepa)

find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GSTREAMERMM REQUIRED gstreamermm-1.0)

file(GLOB SRC_LIST "*.cpp")
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${GSTREAMERMM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${PROJECT_NAME} PUBLIC ${GSTREAMERMM_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${GSTREAMERMM_CFLAGS_OTHER})
