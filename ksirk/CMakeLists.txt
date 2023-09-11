set(HAVE_JABBER_SUPPORT ${WITH_JABBER_SUPPORT})
configure_file(config-ksirk.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-ksirk.h)

if(WITH_JABBER_SUPPORT)
    add_subdirectory(iris)
endif()

add_subdirectory(skins)
add_subdirectory(icons)

add_executable(ksirk)

target_sources(ksirk PRIVATE
    GameLogic/gameautomaton.cpp
    GameLogic/gamesequence.cpp
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

    ksirk.qrc
)

if(WITH_JABBER_SUPPORT)
    target_sources(ksirk PRIVATE
        Dialogs/jabbergameui.cpp
    #   Jabber/jabberaccount.cpp
        Jabber/kmessagejabber.cpp
        Jabber/jabberbytestream.cpp
        Jabber/jabberclient.cpp
        Jabber/jabberconnector.cpp
        Jabber/privacylist.cpp
        Jabber/privacylistitem.cpp
        Jabber/privacymanager.cpp
    )
endif()

ecm_qt_declare_logging_category(ksirk
    HEADER ksirk_debug.h
    IDENTIFIER KSIRK_LOG
    CATEGORY_NAME org.kde.ksirk
    DESCRIPTION "kdegames(ksirk)"
    EXPORT KSIRK
)

if(WITH_JABBER_SUPPORT)
    ecm_qt_declare_logging_category(ksirk
        HEADER jabber_protocol_debug.h
        IDENTIFIER JABBER_PROTOCOL_LOG
        CATEGORY_NAME org.kde.ksirk.jabber
        DESCRIPTION "kdegames (ksirk jabber)"
        EXPORT KSIRK
    )
endif()

ki18n_wrap_ui(ksirk
    Dialogs/qplayersetupdialog.ui
    Dialogs/newGameDialog.ui
    Dialogs/qwaitedplayersetupdialog.ui
    Dialogs/restartOrExitDialog.ui
    Dialogs/joingame.ui
    Dialogs/newGameSummary.ui
    Dialogs/tcpconnectwidget.ui
    preferences.ui
    mainMenu.ui
)
if(WITH_JABBER_SUPPORT)
    ki18n_wrap_ui(ksirk
        Dialogs/jabbergameui.ui
    )
endif()

kconfig_add_kcfg_files( ksirk ksirksettings.kcfgc )

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hicolor/*-apps-ksirk.png")
ecm_add_app_icon(ksirk ICONS ${ICONS_SRCS})

target_include_directories(ksirk
    PRIVATE
        ./Dialogs
        ./GameLogic
        ./SaveLoad
        ./skins
        ./Sprites
)

target_link_libraries(ksirk
    Qt${QT_MAJOR_VERSION}::Xml
    Qt${QT_MAJOR_VERSION}::Svg
    KF${KF_MAJOR_VERSION}::Crash
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::XmlGui
    KF${KF_MAJOR_VERSION}::NewStuffWidgets
    KF${KF_MAJOR_VERSION}::Wallet
    Phonon::phonon4qt${QT_MAJOR_VERSION}
)
if(WITH_JABBER_SUPPORT)
    target_link_libraries(ksirk
        iris_ksirk
    )
endif()
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(ksirk
        KDEGames6
        KDEGames6Private
        Qt6::SvgWidgets
        Qt6::Core5Compat
    )
else()
    target_link_libraries(ksirk
        KF5KDEGames
        KF5KDEGamesPrivate
    )
endif()

install( TARGETS ksirk ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ksirksettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(PROGRAMS org.kde.ksirk.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ksirk.knsrc  DESTINATION  ${KDE_INSTALL_KNSRCDIR})
install(FILES org.kde.ksirk.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
if(WITH_JABBER_SUPPORT)
    install(FILES jabber.png  DESTINATION  ${KDE_INSTALL_DATADIR}/ksirk)
endif()
