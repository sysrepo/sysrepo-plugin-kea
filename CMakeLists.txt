
cmake_minimum_required(VERSION 3.5)

# include_directories("${PROJECT_SOURCE_DIR}/inc")

# turing-machine plugin
add_library(plugin-kea SHARED plugin-kea.c)
target_link_libraries(plugin-kea sysrepo)

# plugins should be installed into ${PLUGINS_DIR}
# (default is "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/sysrepo/plugins/")
# install(TARGETS plugin-kea DESTINATION ${PLUGINS_DIR})
