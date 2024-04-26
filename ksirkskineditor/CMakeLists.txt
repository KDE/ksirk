# add_subdirectory( skins )

add_executable(ksirkskineditor)

target_sources(ksirkskineditor PRIVATE
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

    ksirkskineditor.qrc
)

ecm_qt_declare_logging_category(ksirkskineditor
    HEADER ksirkskineditor_debug.h
    IDENTIFIER KSIRKSKINEDITOR_LOG
    CATEGORY_NAME ksirk.skineditor
    OLD_CATEGORY_NAMES org.kde.ksirk.skineditor
    DESCRIPTION "KsirK game (skineditor)"
    EXPORT KSIRK
)

ki18n_wrap_ui(ksirkskineditor
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

kconfig_add_kcfg_files( ksirkskineditor ksirkskineditorsettings.kcfgc )

target_link_libraries(ksirkskineditor
    KDEGames6
    KDEGames6Private
    KF6::Crash
    KF6::I18n
    KF6::Completion
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::XmlGui
    Qt6::Xml
    Qt6::Svg
    Qt6::Core5Compat
    Qt6::SvgWidgets
 )

install( TARGETS ksirkskineditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES ksirkskineditorsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} )
install( PROGRAMS org.kde.ksirkskineditor.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

install( FILES
    cross.png
    target.png

    DESTINATION ${KDE_INSTALL_DATADIR}/ksirkskineditor
)
