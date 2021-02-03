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

ecm_qt_declare_logging_category(ksirkskineditor_SRCS
                                HEADER ksirkskineditor_debug.h
                                IDENTIFIER KSIRKSKINEDITOR_LOG
                                CATEGORY_NAME org.kde.ksirk.skineditor
                                DESCRIPTION "kdegames (ksirk skineditor)" EXPORT KSIRK
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

add_executable(ksirkskineditor ${ksirkskineditor_SRCS})

target_link_libraries(ksirkskineditor
  Qt5::Xml
  Qt5::Svg 
  KF5::KIOCore 
  Phonon::phonon4qt5
  KF5::XmlGui
  KF5KDEGames
  KF5KDEGamesPrivate
  ${CRYPT_LIBS}
 )

install( TARGETS ksirkskineditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES ksirkskineditorsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} )
install( PROGRAMS org.kde.ksirkskineditor.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install( FILES ksirkskineditorui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/ksirkskineditor )

set (pics_default_DATA
cross.png
target.png
)
install( FILES ${pics_default_DATA} DESTINATION ${KDE_INSTALL_DATADIR}/ksirkskineditor/ )
