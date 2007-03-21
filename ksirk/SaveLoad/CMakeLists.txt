
project(ksirk_saveload)

KDE4_NO_ENABLE_FINAL(ksirk_saveload)

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

set(ksirk_saveload_LIB_SRCS
  ksirkgamexmlhandler.cpp
  ksirkgamexmlloader.cpp
)

kde4_automoc(${ksirk_saveload_LIB_SRCS})

kde4_add_library(ksirk_saveload SHARED ${ksirk_saveload_LIB_SRCS})

# target_link_libraries(ksirk_saveload  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(ksirk_saveload PROPERTIES VERSION 1.9.0 SOVERSION 1 )
install(TARGETS ksirk_saveload  DESTINATION ${LIB_INSTALL_DIR} )

