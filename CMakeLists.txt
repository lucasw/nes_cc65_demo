cmake_minimum_required(VERSION 2.8.3)
project(nes_cc65_demo)

# crt0.o: crt0.s
#   ca65 crt0.s
# demo.o: demo.c
#   cc65 -Oi $< --add-source
#   ca65 $*.s
# demo.nes: demo.o crt0.o
#   ld65 -o demo.nes -C nes.cfg crt0.o $< runtime.lib

set(CC65_DIR $ENV{HOME}/other/cc65 CACHE PATH "location of cc65 directory")
set(CC65 ${CC65_DIR}/bin/cc65)
set(CA65 ${CC65_DIR}/bin/ca65)
set(LD65 ${CC65_DIR}/bin/ld65)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/crt0.o PRE_LINK
  COMMAND ${CA65} ${CMAKE_CURRENT_SOURCE_DIR}/src/crt0.s -o ${CMAKE_CURRENT_BINARY_DIR}/crt0.o
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crt0.s
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neslib.s
    ${CMAKE_CURRENT_SOURCE_DIR}/src/famitone2.s
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sounds.s
    ${CMAKE_CURRENT_SOURCE_DIR}/src/music.s
    ${CMAKE_CURRENT_SOURCE_DIR}/config/tileset.chr
  COMMENT "assemble crt0"
  )
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/demo.s PRE_LINK
  COMMAND ${CC65} -I ${CMAKE_CURRENT_SOURCE_DIR}/include -Oi ${CMAKE_CURRENT_SOURCE_DIR}/src/demo.c --add-source -o ${CMAKE_CURRENT_BINARY_DIR}/demo.s
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/crt0.o ${CMAKE_CURRENT_SOURCE_DIR}/src/demo.c
  COMMENT "compile demo"
  )
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/demo.o PRE_LINK
  COMMAND ${CA65} ${CMAKE_CURRENT_BINARY_DIR}/demo.s -o ${CMAKE_CURRENT_BINARY_DIR}/demo.o
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/demo.s
  COMMENT "assemble demo"
  )
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/demo.nes PRE_LINK
  COMMAND ${LD65} -o ${CMAKE_CURRENT_BINARY_DIR}/demo.nes -C ${CMAKE_CURRENT_SOURCE_DIR}/config/nes.cfg ${CMAKE_CURRENT_BINARY_DIR}/crt0.o ${CMAKE_CURRENT_BINARY_DIR}/demo.o ${CMAKE_CURRENT_SOURCE_DIR}/lib/runtime.lib
  # COMMAND ${LD65} demo.nes -C config/nes.cfg crt0.o demo.o lib/runtime.lib
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/crt0.o ${CMAKE_CURRENT_BINARY_DIR}/demo.o
  COMMENT "linker"
  )

add_custom_target(nes_cc65_demo DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/demo.nes)
# add_dependencies(demo_nes demo.nes)
