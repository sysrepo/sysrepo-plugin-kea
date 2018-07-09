
cmake_minimum_required(VERSION 2.8)

include_directories("/usr/local/include" "${CMAKE_CURRENT_BINARY_DIR}")
link_directories("/usr/local/lib")

# plugin-kea
add_library(plugin-kea SHARED plugin-kea.cc yang-kea.cc yang-kea.h)
target_link_libraries(plugin-kea sysrepo)
set(CMAKE_C_FLAGS "-g -O0")
set(CLIENT_DIR "${CMAKE_SOURCE_DIR}/kea-client" CACHE PATH "Client directory")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/plugin-kea.h.in" "${CMAKE_CURRENT_BINARY_DIR}/plugin-kea.h" ESCAPE_QUOTES @ONLY)

add_executable(get_config get_config.cc yang-kea.cc yang-kea.h)
target_link_libraries(get_config sysrepo)

add_executable(basic_config basic_config.c)
target_link_libraries(basic_config sysrepo)
# plugins should be installed into ${PLUGINS_DIR}
# (default is "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/sysrepo/plugins/")
# install(TARGETS plugin-kea DESTINATION ${PLUGINS_DIR})
