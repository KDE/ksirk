# add_subdirectory( skins )

include_directories( 
  ${CMAKE_BINARY_DIR}/ksirkskineditor 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}  
)

########### next target ###############

set(ksirkskineditor_SRCS
  main.cpp
  mainwindow.cpp
  ksirkSkinEditorConfigDialog.cpp
)

set(ksirkskineditor_UI
#   ksirkskineditor.ui
  preferences.ui
)


kde4_add_ui_files(ksirkskineditor_SRCS ${ksirkskineditor_UI} )

kde4_add_kcfg_files( ksirkskineditor_SRCS ksirkskineditorsettings.kcfgc )

kde4_add_app_icon(ksirkskineditor_SRCS "hi*-app-ksirkskineditor.png")

kde4_add_executable(ksirkskineditor ${ksirkskineditor_SRCS})

target_link_libraries(ksirkskineditor
  ${QT_QTXML_LIBRARY}
  ${QT_QTSVG_LIBRARY} 
  ${KDE4_KIO_LIBS} 
  ${KDE4_PHONON_LIBRARY}
  ${CRYPT_LIBS}
  ${KDEGAMES_LIBRARY}
 )

install( TARGETS ksirkskineditor ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )

install( FILES ksirkskineditorsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES ksirkskineditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES ksirkskineditorui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksirkskineditor )
