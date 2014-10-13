cmake_minimum_required(VERSION 2.8.8)

project(minimax)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/.cmake/modules)
add_definitions("-std=c++11")
if("${CMAKE_GENERATOR}" STREQUAL "Ninja" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions("-Xclang -fcolor-diagnostics")
endif()

include_directories(inc)
set(SRC
  src/action.cpp
  src/minimax.cpp
  src/board.cpp
  src/run.cpp
  src/draw.cpp
)

#
# DEPS
#

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
protobuf_generate_cpp(PROTO_SRC PROTO_HDR
  proto/update.proto
  proto/discrete.proto
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

find_package(ZMQ REQUIRED)
include_directories(${ZMQ_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

find_package(Threads REQUIRED)

#
# BIN
#

add_library(minimax OBJECT ${PROTO_SRC} ${SRC})
set(COMMON_LIBRARIES
  ${PROTOBUF_LIBRARIES}
  ${ZMQ_LIBRARIES}
  ${ARMADILLO_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(minimax_cli src/main.cpp $<TARGET_OBJECTS:minimax>)
target_link_libraries(minimax_cli ${COMMON_LIBRARIES})

add_executable(minimax_gui src/glfw_main.cpp $<TARGET_OBJECTS:minimax>)
target_link_libraries(minimax_gui ${COMMON_LIBRARIES} ${GLFW_STATIC_LIBRARIES})

#
# DOCS
#

find_package(Sphinx)
if(SPHINX_FOUND)
  option(BUILD_DOCS "Build documentations?" OFF)
endif()
if(BUILD_DOCS)
  if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME default)
  endif()

  # configured documentation tools and intermediate build results
  set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

  # Sphinx cache with pickled ReST documents
  set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

  # HTML output directory
  set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

  #configure_file(
  #  "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
  #  "${BINARY_BUILD_DIR}/conf.py"
  #  @ONLY)

  add_custom_target(minimax_docs ALL
    ${SPHINX_EXECUTABLE}
    -q -b html
    -c "${CMAKE_CURRENT_SOURCE_DIR}/docs"
    -d "${SPHINX_CACHE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/docs"
    "${SPHINX_HTML_DIR}"
    COMMENT "Building HTML documentation with Sphinx")
endif()

# vim: et sw=2 ts=2 sts=2
