
project(ksirk_dialogs)

KDE4_NO_ENABLE_FINAL(ksirk_dialogs)

include_directories( 
  ${CMAKE_SOURCE_DIR}/ksirk 
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

set(ksirk_dialogs_LIB_SRCS
  kplayersetupdialog.cpp
  newGameDialogImpl.cpp
  kwaitedplayersetupdialog.cpp
  restartOrExitDialogImpl.cpp
)

set(ksirk_dialogs_UI
  qplayersetupdialog.ui
  newGameDialog.ui
  qwaitedplayersetupdialog.ui 
  restartOrExitDialog.ui
)

kde4_add_ui_files(ksirk_dialogs_LIB_SRCS ${ksirk_dialogs_UI} )

kde4_add_library(ksirk_dialogs SHARED ${ksirk_dialogs_LIB_SRCS})

# target_link_libraries(ksirk_dialogs  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} kdefx )

set_target_properties(ksirk_dialogs PROPERTIES VERSION 1.9.0 SOVERSION 1 )
install(TARGETS ksirk_dialogs  DESTINATION ${LIB_INSTALL_DIR} )

