include(CheckIncludeFiles)
check_include_files(sys/stropts.h SYS_STROPTS_H_FOUND)
configure_file(config-ksirk.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksirk.h)

add_subdirectory(skins)
add_subdirectory(iris)
add_subdirectory(icons)

# FIND_PACKAGE(ZLIB REQUIRED)

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
  )

link_directories(${CMAKE_CURRENT_BINARY_DIR}/iris/)

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


ki18n_wrap_ui(ksirk_SRCS ${ksirk_UI} )

kconfig_add_kcfg_files( ksirk_SRCS ksirksettings.kcfgc )

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hicolor/*-apps-ksirk.png")
ecm_add_app_icon(ksirk_SRCS ICONS ${ICONS_SRCS})

add_executable(ksirk ${ksirk_SRCS})

target_link_libraries(ksirk
    Qt5::Xml
    Qt5::Svg
    qca-qt5
    iris_ksirk
    KF5::Crash
    KF5::KDELibs4Support
    KF5::KIOCore
    KF5::NewStuff
    KF5::Wallet
    Phonon::phonon4qt5
    KF5KDEGames
    KF5KDEGamesPrivate
)

install( TARGETS ksirk ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES ksirksettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(PROGRAMS org.kde.ksirk.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ksirkui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/ksirk)
install(FILES jabber.png  DESTINATION  ${KDE_INSTALL_DATADIR}/ksirk)
install(FILES ksirk.knsrc  DESTINATION  ${KDE_INSTALL_CONFDIR})
