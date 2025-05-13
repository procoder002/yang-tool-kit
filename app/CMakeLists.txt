cmake_minimum_required(VERSION 2.8)

project(ytk_client_app)

include_directories(
   ${LIBYANG_DIR}
   /usr/include/libxml2
   ../build/auto_gen
   ./common
   ../include
   )

link_directories(
   ${LIBYANG_DIR}
)

set(SOURCES
   src/client_app.cpp
   ../src/utility.cpp
   src/yang_subs_mgr.cpp
   ../build/auto_gen/gen_network_device.cpp
)

add_executable(ytk_client_app ${SOURCES})
target_link_libraries(ytk_client_app yang xml2)
