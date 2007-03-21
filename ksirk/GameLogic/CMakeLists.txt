
project(ksirk_gamelogic)

KDE4_NO_ENABLE_FINAL(ksirk_gamelogic)

include_directories( 
  ${CMAKE_SOURCE_DIR}/ksirk 
  ${CMAKE_SOURCE_DIR}/ksirk/Dialogs 
  ${CMAKE_BINARY_DIR}/ksirk/Dialogs 
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

set(ksirk_gamelogic_LIB_SRCS
  aiColsonPlayer.cpp
  aiplayer.cpp
  aiplayerio.cpp
  continent.cpp
  country.cpp
  dice.cpp
  eventslistproperty.cpp
  gameautomaton.cpp
  goal.cpp
  KsirkChatDelegate.cpp
  KsirkChatModel.cpp
  KsirkChatItem.cpp
  KMessageParts.cpp
  nationality.cpp
  onu.cpp
  player.cpp
)

kde4_automoc(${ksirk_gamelogic_LIB_SRCS})

kde4_add_library(ksirk_gamelogic SHARED ${ksirk_gamelogic_LIB_SRCS})

# target_link_libraries(ksirk_gamelogic  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(ksirk_gamelogic PROPERTIES VERSION 1.9.0 SOVERSION 1 )
install(TARGETS ksirk_gamelogic  DESTINATION ${LIB_INSTALL_DIR} )

