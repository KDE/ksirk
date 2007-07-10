
project(ksirk_sprites)

KDE4_NO_ENABLE_FINAL(ksirk_sprites)

include_directories( 
  ${CMAKE_SOURCE_DIR}/ksirk 
  ${CMAKE_BINARY_DIR}/ksirk
  ${CMAKE_SOURCE_DIR}/ksirk/Dialogs 
  ${CMAKE_SOURCE_DIR}/ksirk/GameLogic 
  ${CMAKE_SOURCE_DIR}/ksirk/SaveLoad 
  ${CMAKE_SOURCE_DIR}/ksirk/skins 
  ${CMAKE_SOURCE_DIR}/ksirk/Sprites 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}  
  ${QDBUS_INCLUDE_DIRS} 
  ${CMAKE_SOURCE_DIR} 
  ${CMAKE_BINARY_DIR} 
  ${KDE4_INCLUDES}
  ${KDE4_KDECORE_INCLUDES}
)

########### next target ###############

set(ksirk_sprites_LIB_SRCS
  backgnd.cpp
  animsprite.cpp
  animspritespool.cpp
  animspritesgroup.cpp
  cavalrysprite.cpp
  animspriteslist.cpp
  infantrysprite.cpp
  cannonsprite.cpp
  skinSpritesData.cpp
)

kde4_add_library(ksirk_sprites SHARED ${ksirk_sprites_LIB_SRCS})

# target_link_libraries(ksirk_sprites  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(ksirk_sprites PROPERTIES VERSION 1.9.0 SOVERSION 1 )
install(TARGETS ksirk_sprites  DESTINATION ${LIB_INSTALL_DIR} )

