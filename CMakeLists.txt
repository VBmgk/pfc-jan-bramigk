cmake_minimum_required(VERSION 2.8.8)

project(ai) # TODO: new name maybe?

# Super strict warnings =)
set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/.cmake/modules)
set(CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
if("${CMAKE_GENERATOR}" STREQUAL "Ninja" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions("-Xclang -fcolor-diagnostics")
endif()

include_directories(src)
include_directories(vendor/imgui)
include_directories(vendor/cppzmq)

set(SRC
  src/gui.cpp
  src/draw.cpp
  src/app.cpp
  src/action.cpp
  src/decision.cpp
  src/optimization.cpp
  src/vector.cpp
  src/consts.cpp
  src/state.cpp
  src/filter.cpp
  src/suggestion_table.cpp
  src/suggestions.cpp
  vendor/imgui/imgui.cpp
)

set(INC
  src/action.h
  #src/adaptive_control.h
  src/app.h
  src/array.h
  src/colors.h
  src/consts.h
  src/decision.h
  src/decision_source.h
  src/decision_table.h
  src/draw.h
  src/filter.h
  src/gui.h
  src/id_table.h
  src/minimax.h
  src/optimization.h
  src/player.h
  src/segment.h
  src/state.h
  src/suggestion_table.h
  src/suggestions.h
  src/utils.h
  src/valued_decision.h
  src/vector.h
  #src/weight.h
)

set(DOC
  doc/anexos.tex
  doc/ssl.tex
  doc/arquitetura.tex
  doc/conclusao.tex
  doc/definicoes.tex
  doc/discretizacao.tex
  doc/funcao_objetivo.tex
  doc/gui.tex
  doc/introducao.tex
  doc/modelagem.tex
  doc/dec_table.tex
  doc/posicoes_chave.tex
  doc/resultados.tex
  doc/resumo.tex
  doc/distr_move.tex
  doc/estr_busca.tex
  doc/results/kick_pos_var.tex
  doc/results/dist_total_mv.tex
  doc/results/dist_max_mv.tex
  doc/results/atack_cost.tex
  doc/results/see_enemy_goal.tex
  doc/results/block_goal.tex
  doc/results/min_gap_kick.tex
)

set(IMG
  img/arq_geral_prog.eps
  img/arquitetura_ssl.png
  img/cmra_campo.png
  img/communication.eps
  img/distr_mov.eps
  img/ex_tatica.eps
  img/gui.eps
  img/gui_ball_move.png
  img/gui_ball_stop.png
  img/gui_field.png
  img/gui_kick.png
  img/gui_move.png
  img/gui_params.png
  img/gui_pass.png
  img/gui_widgets.png
  img/gui_zoom_in.png
  img/gui_zoom_out.png
  img/kick_pos_var_1.png
  img/kick_pos_var_2.png
  img/minimax-tictactoe.jpg
  img/minimax-tree.ps
  img/minimax_flow.eps
  img/padroes_ssl.png
  img/pos_chave_barreira.png
  img/pos_chave_lateral.png
  img/result/atack_atq_5000.png
  img/result/atack_def_5000.png
  img/result/block_goal_atq_0.png
  img/result/block_goal_def_0.png
  img/result/default_atq.png
  img/result/default_def.png
  img/result/kickpos_024_atq.png
  img/result/kickpos_024_def.png
  img/result/kickpos_0_atq.png
  img/result/kickpos_0_def.png
  img/result/min_gap_atq_5.png
  img/result/min_gap_def_5.png
  img/result/mov_dist_max_atq_10.png
  img/result/mov_dist_max_def_10.png
  img/result/mov_dist_total_atq_10.png
  img/result/mov_dist_total_def_10.png
  img/result/see_enemy_goal_atq_1000.png
  img/result/see_enemy_goal_def_1000.png
  img/result/total_max_gap_atq_0.png
  img/result/total_max_gap_def_0.png
  img/rob_data.png
  img/robo.png
  img/robocup2013.jpg
  img/tab_dec.eps
  img/threads.eps
  img/xadrez.jpg
)

set(PROTO
  proto/update.proto
  proto/discrete.proto
)

#
# DEPS
#

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
protobuf_generate_cpp(PROTO_SRC PROTO_HDR ${PROTO})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#find_package(Armadillo REQUIRED)
#include_directories(${ARMADILLO_INCLUDE_DIRS})

find_package(ZMQ REQUIRED)
include_directories(${ZMQ_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

#find_package(PkgConfig REQUIRED)
#pkg_search_module(GLFW REQUIRED glfw3)
#include_directories(${GLFW_INCLUDE_DIRS})
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
add_subdirectory(vendor/glfw)
include_directories(vendor/glfw/include)

find_package(Threads REQUIRED)

#
# BIN
#

add_library(core OBJECT ${PROTO_SRC} ${SRC})
set(COMMON_LIBRARIES
  ${PROTOBUF_LIBRARIES}
  ${ZMQ_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

#add_executable(minimax_tests tests/main.cpp $<TARGET_OBJECTS:core>)
#target_link_libraries(minimax_tests ${COMMON_LIBRARIES})

#add_executable(minimax_cli src/main.cpp $<TARGET_OBJECTS:core>)
#target_link_libraries(minimax_cli ${COMMON_LIBRARIES})

#add_executable(ai-gui src/main-gui.cpp $<TARGET_OBJECTS:core>)
#target_link_libraries(ai-gui ${COMMON_LIBRARIES} ${GLFW_STATIC_LIBRARIES})

add_executable(ai src/main.cpp $<TARGET_OBJECTS:core>)
#target_link_libraries(ai ${COMMON_LIBRARIES} ${GLFW_STATIC_LIBRARIES})
target_link_libraries(ai ${COMMON_LIBRARIES} glfw ${GLFW_LIBRARIES})

add_custom_target(run
  COMMAND ai
  DEPENDS ai
  WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

find_package(LATEX)
if (PDFLATEX_COMPILER)
  option(BUILD_REPORT "Build report?" ON)
endif()
if(BUILD_REPORT)
  SET(LATEX_COMPILER_FLAGS
    --enable-write18 -file-line-error -shell-escape
    CACHE STRING "Flags passed to latex."
  )
  include(${CMAKE_SOURCE_DIR}/.cmake/UseLATEX.cmake)

  add_latex_document(relatorio.tex
    INPUTS ${DOC} ${INC} ${PROTO}
    CONFIGURE relatorio.tex
    BIBFILES referencias.bib
    IMAGES ${IMG}
    USE_INDEX
    NO_DEFAULT
  )

  add_latex_document(apresentacao.tex
    IMAGE_DIRS img
    NO_DEFAULT
    MANGLE_TARGET_NAMES
  )

  find_program(OPEN NAMES gnome-open xdg-open open)

  add_custom_target(open
    COMMAND ${OPEN} relatorio.pdf
    DEPENDS pdf
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  )

  add_custom_target(pres
    COMMAND ${OPEN} apresentacao.pdf
    DEPENDS apresentacao_pdf
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
  )
endif()

# vim: et sw=2 ts=2 sts=2
