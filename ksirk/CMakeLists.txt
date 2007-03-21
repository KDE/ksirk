# add_subdirectory( Dialogs ) 
# add_subdirectory( GameLogic ) 
# add_subdirectory( SaveLoad ) 
# add_subdirectory( skins ) 
# add_subdirectory( Sprites ) 

include_directories( 
  ${CMAKE_SOURCE_DIR}/ksirk 
  ${CMAKE_BINARY_DIR}/ksirk 
  ${CMAKE_SOURCE_DIR}/ksirk/Dialogs 
  ${CMAKE_BINARY_DIR}/ksirk/Dialogs 
  ${CMAKE_SOURCE_DIR}/ksirk/GameLogic 
  ${CMAKE_SOURCE_DIR}/ksirk/SaveLoad 
  ${CMAKE_SOURCE_DIR}/ksirk/skins 
  ${CMAKE_SOURCE_DIR}/ksirk/Sprites 
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR}  
)

include_directories (
  ${QDBUS_INCLUDE_DIRS} 
  ${CMAKE_SOURCE_DIR} 
  ${CMAKE_BINARY_DIR} 
  ${KDE4_INCLUDES}
)

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
  main.cpp 
  decoratedgameframe.cpp 
  gestionSprites.cpp 
  kgamewin.cpp 
  kgamewinslots.cpp 
  ksirkConfigDialog.cpp 
)

set(ksirk_dialogs_UI
  Dialogs/qplayersetupdialog.ui
  Dialogs/newGameDialog.ui
  Dialogs/qwaitedplayersetupdialog.ui 
  Dialogs/restartOrExitDialog.ui
)


kde4_add_ui_files(ksirk_SRCS ${ksirk_dialogs_UI} )

kde4_add_kcfg_files( ksirk_SRCS ksirksettings.kcfgc )

kde4_add_ui_files( ksirk_SRCS preferences.ui )

kde4_automoc(${ksirk_SRCS})

kde4_add_executable(ksirk ${ksirk_SRCS})

target_link_libraries(ksirk
#   ksirk_dialogs
#   ksirk_gamelogic
#   ksirk_saveload
#   ksirk_sprites
  ${QT_AND_KDECORE_LIBS} 
  ${QT_QT_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${QT_QTSVG_LIBRARY} 
  ${KDE4_KHTML_LIBS} 
  ${KDE4_KPARTS_LIBS} 
  ${KDE4_KDEUI_LIBS} 
  ${KDE4_KDECORE_LIBS} 
  ${KDE4_KIO_LIBS} 
  ${KDE4_PHONON_LIBS} 
  ${KDE4_KDEPRINT_LIBS} 
  ${KDE4_KWALLETCLIENT_LIBS} 
  ${KDE4_KDEFX_LIBS} 
  ${KDE4_KDEGAMES_LIBS} 
  ${CRYPT_LIBS}
  kdegames
  phononcore
  phononui
 )

install( TARGETS ksirk DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )

install( FILES ksirksettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES ksirk.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES ksirk.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

install(FILES skins/skinsdir DESTINATION ${DATA_INSTALL_DIR}/ksirk/skins )

install(FILES skins/default/Data/onu.xml DESTINATION ${DATA_INSTALL_DIR}/ksirk/skins/default/Data )

set (pics_default_DATA
./skins/default/Images/reddice1.png
./skins/default/Images/reddice2.png
./skins/default/Images/reddice3.png
./skins/default/Images/reddice4.png
./skins/default/Images/reddice5.png
./skins/default/Images/reddice6.png
./skins/default/Images/bluedice1.png
./skins/default/Images/bluedice2.png
./skins/default/Images/bluedice3.png
./skins/default/Images/bluedice4.png
./skins/default/Images/bluedice5.png
./skins/default/Images/bluedice6.png
./skins/default/Images/snapshot.jpg
./skins/default/Images/moveFinish.png
./skins/default/Images/joueurSuivant.png
./skins/default/Images/moveBackFive.png
./skins/default/Images/moveFive.png
./skins/default/Images/newNetGame.png
./skins/default/Images/moveArmies.png
./skins/default/Images/saveGame.png
./skins/default/Images/attackTwo.png
./skins/default/Images/aide.png
./skins/default/Images/map.svg
./skins/default/Images/moveOne.png
./skins/default/Images/preferences.png
./skins/default/Images/countries/vietnam.png
./skins/default/Images/countries/japan.png
./skins/default/Images/countries/westus.png
./skins/default/Images/countries/middleeast.png
./skins/default/Images/countries/ontario.png
./skins/default/Images/countries/brazil.png
./skins/default/Images/countries/mongolia.png
./skins/default/Images/countries/algeria.png
./skins/default/Images/countries/zair.png
./skins/default/Images/countries/eastus.png
./skins/default/Images/countries/france.png
./skins/default/Images/countries/egypt.png
./skins/default/Images/countries/prussia.png
./skins/default/Images/countries/iran.png
./skins/default/Images/countries/alaska.png
./skins/default/Images/countries/kazakstan.png
./skins/default/Images/countries/arabia.png
./skins/default/Images/countries/chile.png
./skins/default/Images/countries/scandinavia.png
./skins/default/Images/countries/china.png
./skins/default/Images/countries/island.png
./skins/default/Images/countries/benelux.png
./skins/default/Images/countries/quebec.png
./skins/default/Images/countries/alberta.png
./skins/default/Images/countries/papouasia.png
./skins/default/Images/countries/gabon.png
./skins/default/Images/countries/pakistan.png
./skins/default/Images/countries/groenland.png
./skins/default/Images/countries/india.png
./skins/default/Images/countries/newzealand.png
./skins/default/Images/countries/westernaustralia.png
./skins/default/Images/countries/mexico.png
./skins/default/Images/countries/uk.png
./skins/default/Images/countries/russia.png
./skins/default/Images/countries/paraguay.png
./skins/default/Images/countries/siberia.png
./skins/default/Images/countries/newsouthwalles.png
./skins/default/Images/countries/niger.png
./skins/default/Images/countries/madagascar.png
./skins/default/Images/countries/columbia.png
./skins/default/Images/countries/indonesia.png
./skins/default/Images/countries/southafrica.png
./skins/default/Images/sprites/exploding.png
./skins/default/Images/sprites/belgium.png
./skins/default/Images/sprites/japan.png
./skins/default/Images/sprites/infantry.png
./skins/default/Images/sprites/france.png
./skins/default/Images/sprites/cavalry.png
./skins/default/Images/sprites/8020.png
./skins/default/Images/sprites/uk.png
./skins/default/Images/sprites/us.png
./skins/default/Images/sprites/cannon.png
./skins/default/Images/sprites/firing.png
./skins/default/Images/sprites/italy.png
./skins/default/Images/attackOne.png
./skins/default/Images/annuler.png
./skins/default/Images/map-mask.png
./skins/default/Images/pool-en.png
./skins/default/Images/quitter.png
./skins/default/Images/moveBackTen.png
./skins/default/Images/ouvrirJeu.png
./skins/default/Images/logoRight.png
./skins/default/Images/distributionArmeesFinie.png
./skins/default/Images/distributionArmees.png
./skins/default/Images/logoLeft.png
./skins/default/Images/defendTwo.png
./skins/default/Images/attackThree.png
./skins/default/Images/SoldatAGenoux1.png
./skins/default/Images/defendOne.png
./skins/default/Images/nouveauJeu.png
./skins/default/Images/moveTen.png
./skins/default/Images/moveBackOne.png
)
install( FILES ${pics_default_DATA} DESTINATION ${DATA_INSTALL_DIR}/ksirk/skins/default/Images/ )

set (sprites_default_DATA 
skins/default/Images/sprites/belgium.svg 
skins/default/Images/sprites/cannon.svg 
skins/default/Images/sprites/cavalry.svg 
skins/default/Images/sprites/exploding.svg 
skins/default/Images/sprites/firing.svg 
skins/default/Images/sprites/france.svg 
skins/default/Images/sprites/infantry.svg 
skins/default/Images/sprites/italy.svg 
skins/default/Images/sprites/japan.svg 
skins/default/Images/sprites/uk.svg 
skins/default/Images/sprites/us.svg)

install( FILES ${sprites_default_DATA} DESTINATION ${DATA_INSTALL_DIR}/ksirk/skins/default/Images/sprites )

set (sounds_default_DATA 
skins/default/Sounds/canon.wav 
skins/default/Sounds/crash.wav 
skins/default/Sounds/roule.wav)

install(FILES ${sounds_default_DATA} DESTINATION ${DATA_INSTALL_DIR}/ksirk/skins/default/Sounds)


