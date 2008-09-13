add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( skins )
add_subdirectory( libiris )

FIND_PACKAGE(QCA2 REQUIRED)
macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed to build Kopete's GroupWise and Jabberprotocols.")

include_directories( 
  ${CMAKE_BINARY_DIR}/ksirk 
  ./Dialogs 
  ${CMAKE_BINARY_DIR}/ksirk/Dialogs 
  ./GameLogic 
  ./SaveLoad 
  ./skins 
  ./Sprites 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}  
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/jabber/
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-core/
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-im/
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/util/
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/network/
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/
  ${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/
  ${QCA2_INCLUDE_DIR}
  )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libiris/ )

########### next target ###############

set(ksirk_SRCS 
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
  GameLogic/gameautomaton.cpp
  GameLogic/goal.cpp
  GameLogic/KsirkChatItem.cpp
  GameLogic/KsirkChatModel.cpp
  GameLogic/KsirkChatDelegate.cpp
  GameLogic/KMessageParts.cpp
  GameLogic/nationality.cpp
  GameLogic/onu.cpp
  GameLogic/player.cpp
  Dialogs/kplayersetupdialog.cpp
  Dialogs/newGameDialogImpl.cpp
  Dialogs/kwaitedplayersetupdialog.cpp
  Dialogs/restartOrExitDialogImpl.cpp
  Dialogs/joingame.cpp
  Dialogs/jabberconnect.cpp
  main.cpp 
  decoratedgameframe.cpp 
  gestionSprites.cpp 
  kgamewin.cpp 
  kgamewinslots.cpp 
  ksirkConfigDialog.cpp
  MessageBubble.cpp
  fightArena.cpp
  mainMenu.cpp
  krightdialog.cpp
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
  Dialogs/jabberconnect.ui
  Dialogs/joingame.ui
  preferences.ui
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
  ${CRYPT_LIBS}
  ${KDEGAMES_LIBRARY}
  iris_ksirk
  kdegames
  )

install( TARGETS ksirk ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )

install( FILES ksirksettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES ksirk.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES ksirkui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksirk )
install( FILES ksirk.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
