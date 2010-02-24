add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include(CheckIncludeFiles)
check_include_files(sys/stropts.h SYS_STROPTS_H_FOUND)
configure_file(config-ksirk.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksirk.h)

add_subdirectory( skins )
add_subdirectory( iris )

# FIND_PACKAGE(QCA2 REQUIRED)
# FIND_PACKAGE(ZLIB REQUIRED)
# macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed to build Ksirk Jabber support's GroupWise and Jabberprotocols.")

include_directories( 
  ${CMAKE_BINARY_DIR}/ksirk 
  ./Dialogs
  ./GameLogic 
  ./SaveLoad 
  ./skins 
  ./Sprites 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}  
  ${CMAKE_CURRENT_SOURCE_DIR}/iris/include/iris
  ${CMAKE_CURRENT_SOURCE_DIR}/iris/src
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/include/
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/jabber/
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-core/
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-im/
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/util/
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/network/
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/
#   ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/
  ${QCA2_INCLUDE_DIR}
  )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/iris/ )

########### next target ###############

set(ksirk_SRCS 
  GameLogic/gameautomaton.cpp
  kgamewin.cpp
  kgamewinslots.cpp
  gestionSprites.cpp
  Sprites/backgnd.cpp 
  Sprites/animsprite.cpp
  Sprites/cavalrysprite.cpp
  Sprites/animspriteslist.cpp
  Sprites/animspritespool.cpp
  Sprites/animspritesgroup.cpp
  Sprites/infantrysprite.cpp
  Sprites/cannonsprite.cpp
  Sprites/skinSpritesData.cpp
  Sprites/arrowsprite.cpp
  SaveLoad/ksirkgamexmlhandler.cpp
  SaveLoad/ksirkgamexmlloader.cpp
  GameLogic/aiColsonPlayer.cpp
  GameLogic/aiplayer.cpp
  GameLogic/aiplayerio.cpp
  GameLogic/continent.cpp
  GameLogic/country.cpp
  GameLogic/dice.cpp
  GameLogic/eventslistproperty.cpp
  GameLogic/goal.cpp
  GameLogic/KsirkChatItem.cpp
  GameLogic/KsirkChatModel.cpp
  GameLogic/KsirkChatDelegate.cpp
  GameLogic/KMessageParts.cpp
  GameLogic/nationality.cpp
  GameLogic/onu.cpp
  GameLogic/player.cpp
  GameLogic/distributiondata.cpp
  GameLogic/newplayerdata.cpp
  Dialogs/kplayersetupwidget.cpp
  Dialogs/newGameDialogImpl.cpp
  Dialogs/kwaitedplayersetupdialog.cpp
  Dialogs/restartOrExitDialogImpl.cpp
  Dialogs/joingame.cpp
  Dialogs/jabbergameui.cpp
  Dialogs/InvasionSlider.cpp
  Dialogs/newGameSummaryWidget.cpp
  Dialogs/tcpconnectwidget.cpp
  main.cpp
  decoratedgameframe.cpp 
  ksirkConfigDialog.cpp
  fightArena.cpp
  mainMenu.cpp
  krightdialog.cpp
  newgamesetup.cpp
#   Jabber/jabberaccount.cpp
  Jabber/kmessagejabber.cpp
  Jabber/jabberbytestream.cpp
  Jabber/jabberclient.cpp
  Jabber/jabberconnector.cpp
  Jabber/privacylist.cpp
  Jabber/privacylistitem.cpp
  Jabber/privacymanager.cpp
)

set(ksirk_UI
  Dialogs/qplayersetupdialog.ui
  Dialogs/newGameDialog.ui
  Dialogs/qwaitedplayersetupdialog.ui 
  Dialogs/restartOrExitDialog.ui
  Dialogs/joingame.ui
  Dialogs/jabbergameui.ui
  Dialogs/newGameSummary.ui
  Dialogs/tcpconnectwidget.ui
  preferences.ui
  mainMenu.ui
  )


kde4_add_ui_files(ksirk_SRCS ${ksirk_UI} )

kde4_add_kcfg_files( ksirk_SRCS ksirksettings.kcfgc )

kde4_add_app_icon(ksirk_SRCS "hi*-app-ksirk.png")

kde4_add_executable(ksirk ${ksirk_SRCS})

target_link_libraries(ksirk
  ${QT_QTXML_LIBRARY}
  ${QT_QTSVG_LIBRARY} 
  ${KDE4_KIO_LIBS} 
  ${KDE4_PHONON_LIBRARY}
  ${KDE4_KNEWSTUFF3_LIBRARY}
  ${QCA2_LIBRARIES}
  iris_ksirk
  kdegames
  )

install( TARGETS ksirk ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )

install( FILES ksirksettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES ksirk.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES ksirkui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksirk )
install( FILES jabber.png  DESTINATION  ${DATA_INSTALL_DIR}/ksirk )
install( FILES ksirk.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
