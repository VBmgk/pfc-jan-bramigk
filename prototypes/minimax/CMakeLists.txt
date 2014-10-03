cmake_minimum_required(VERSION 2.8.9)

project(minimax)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

include_directories(inc)
set(SRC
  src/minimax.cpp
  src/board.cpp
)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
protobuf_generate_cpp(PROTO_SRC PROTO_HDR
  proto/update.proto
  proto/discrete.proto
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

add_executable(minimax_cli ${SRC} ${PROTO_SRC} src/main.cpp)
target_link_libraries(minimax_cli ${PROTOBUF_LIBRARIES} zmq ${ARMADILLO_LIBRARIES})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})

add_executable(minimax_gui ${SRC} ${PROTO_SRC} src/opengl.cpp)
target_link_libraries(minimax_gui ${PROTOBUF_LIBRARIES} zmq ${ARMADILLO_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

# Docs
find_package(Sphinx)
if(SPHINX_FOUND)
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
