# add_subdirectory( skins )

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}  
)

########### next target ###############

set(ksirkskineditor_SRCS
  main.cpp
  mainwindow.cpp
  ksirkSkinEditorConfigDialog.cpp
  ksirkskineditorscene.cpp
  ksirkskineditorpixmapitem.cpp
  ksirkskineditortextitem.cpp
  ksirkcountrydefinition.cpp
  onu.cpp
  continent.cpp
  country.cpp
  nationality.cpp
  skinSpritesData.cpp
  goal.cpp
)

set(ksirkskineditor_UI
#   ksirkskineditor.ui
  preferences.ui
  ksirkskineditorwidget.ui
  ksirkskindefinition.ui
  ksirkcountrydefinition.ui
  ksirkcontinentdefinition.ui
  ksirkgoaldefinition.ui
  ksirknationalitydefinition.ui
  ksirkspritesdefinition.ui
  ksirkskineditorcountriesselectiondialog.ui
)


ki18n_wrap_ui(ksirkskineditor_SRCS ${ksirkskineditor_UI} )

kconfig_add_kcfg_files( ksirkskineditor_SRCS ksirkskineditorsettings.kcfgc )

#kde4_add_app_icon(ksirkskineditor_SRCS "hi*-app-ksirk.png")

add_executable(ksirkskineditor ${ksirkskineditor_SRCS})

target_link_libraries(ksirkskineditor
  Qt5::Xml
  Qt5::Svg 
  KF5::KIOCore 
  Phonon::phonon4qt5
  KF5KDEGames
  KF5KDEGamesPrivate
  ${CRYPT_LIBS}
 )

install( TARGETS ksirkskineditor ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

ecm_install_icons( ${ICON_INSTALL_DIR} )

install( FILES ksirkskineditorsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( PROGRAMS org.kde.ksirkskineditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES ksirkskineditorui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/ksirkskineditor )

set (pics_default_DATA
cross.png
target.png
)
install( FILES ${pics_default_DATA} DESTINATION ${DATA_INSTALL_DIR}/ksirkskineditor/ )
