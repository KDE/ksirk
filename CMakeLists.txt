cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(ksirk VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.91.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Widgets
    Svg
    Test
)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS SvgWidgets)
endif()

find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Completion
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DocTools
    I18n
    KIO
    NewStuff
    Wallet
    WidgetsAddons
    XmlGui
)

find_package(KF5KDEGames 7.3.0 REQUIRED)

find_package(Phonon4Qt${QT_MAJOR_VERSION} REQUIRED)

find_package(Qca-qt${QT_MAJOR_VERSION} 2.1.0)
set_package_properties(Qca-qt5 PROPERTIES
    DESCRIPTION "Support for encryption"
    URL "https://download.kde.org/stable/qca-qt5"
    TYPE REQUIRED
)

kde_enable_exceptions()

add_definitions(
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_NO_URL_CAST_FROM_STRING
)

ecm_set_disabled_deprecation_versions(
    QT 5.14
    KF 5.79
    KDEGAMES 7.3
)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX KSIRK
    VERSION_HEADER ksirk_version.h
)

add_subdirectory(ksirk)
add_subdirectory(ksirkskineditor)
add_subdirectory(doc)

ki18n_install(po)
kdoctools_install(po)

ecm_qt_install_logging_categories(
    EXPORT KSIRK
    FILE ksirk.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
