/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

// application specific includes
#include "mainwindow.h"
#include "ksirkSkinEditorConfigDialog.h"
#include "ksirkskineditorsettings.h"
#include "ksirkskineditorscene.h"
#include "ksirkskindefinition.h"
#include "ksirkcountrydefinition.h"
#include "ksirkcontinentdefinition.h"
#include "ksirkgoaldefinition.h"
#include "ksirknationalitydefinition.h"
#include "ksirkspritesdefinition.h"
#include "ksirkskineditorpixmapitem.h"
#include "ksirkskineditortextitem.h"
#include "ksirkskineditorcountriesselectiondialog.h"
#include "skinSpritesData.h"
#include "goal.h"

//include files for QT
#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QTreeView>
#include <QGridLayout>
#include <QString>
#include <QMovie>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QBitmap>
#include <QPixmap>
#include <QInputDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QSvgRenderer>

// include files for KDE
#include <KMessageBox>
#include <KLocalizedString>
#include <KConfig>
#include <KGameStandardAction>
#include <KStandardAction>
#include <KActionCollection>
#include "ksirkskineditor_debug.h"
#include <KToolBar>
#include <KAboutApplicationDialog>
#include <KRecentFilesAction>


#include <assert.h>


namespace KsirkSkinEditor
{

MainWindow::MainWindow(QWidget* parent) :
  KXmlGuiWindow(parent),
  m_selectedSprite(None),
  m_onu(nullptr),
  m_rfa(nullptr),
  m_mapItem(nullptr),
  m_updateHighlightPosition(false)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << "MainWindow constructor begin";
  KSirkSkinEditorWidget* mainWidget = new KSirkSkinEditorWidget(this);
  setCentralWidget(mainWidget);
  
  m_mapScene = new Scene(0,0,1024,768,this);
  connect(m_mapScene, &Scene::position, this, &MainWindow::slotPosition);
  connect(m_mapScene, &Scene::pressPosition, this, &MainWindow::slotPressPosition);
  connect(m_mapScene, &Scene::releasePosition, this, &MainWindow::slotReleasePosition);
  m_mapView = new QGraphicsView(m_mapScene,mainWidget->mapScrollArea);
  m_mapView->setInteractive(true);

  mainWidget->mapScrollArea->setBackgroundRole(QPalette::Dark);
  mainWidget->mapScrollArea->setWidget(m_mapView);
  
  QHBoxLayout* layout = new QHBoxLayout;
  mainWidget->buttonsScrollArea->setLayout(layout);

  m_flagButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_flagButton);
  m_flagButton->setCheckable(true);
  m_flagButton->setEnabled(false);
  connect(m_flagButton,&QAbstractButton::clicked,this,&MainWindow::slotFlagButtonClicked);

  m_infantryButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_infantryButton);
  m_infantryButton->setCheckable(true);
  m_infantryButton->setEnabled(false);
  connect(m_infantryButton,&QAbstractButton::clicked,this,&MainWindow::slotInfantryButtonClicked);

  m_cavalryButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_cavalryButton);
  m_cavalryButton->setCheckable(true);
  m_cavalryButton->setEnabled(false);
  connect(m_cavalryButton,&QAbstractButton::clicked,this,&MainWindow::slotCavalryButtonClicked);

  m_cannonButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_cannonButton);
  m_cannonButton->setCheckable(true);
  m_cannonButton->setEnabled(false);
  connect(m_cannonButton,&QAbstractButton::clicked,this,&MainWindow::slotCannonButtonClicked);

  QString anchorFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("cross.png"));
  if (anchorFileName.isNull())
  {
    KMessageBox::error(nullptr, i18n("Cannot load anchor icon<br>Program cannot continue"));
    exit(2);
  }
  QPixmap anchorPix = QPixmap(anchorFileName);
  m_anchorButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_anchorButton->setIcon(anchorPix);
  layout->addWidget(m_anchorButton);
  m_anchorButton->setCheckable(true);
  m_anchorButton->setEnabled(false);
  connect(m_anchorButton,&QAbstractButton::clicked,this,&MainWindow::slotAnchorButtonClicked);
  
  QString centerFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("target.png"));
  if (centerFileName.isNull())
  {
    KMessageBox::error(nullptr, i18n("Cannot load center icon<br>Program cannot continue"));
    exit(2);
  }
  QPixmap centerPix = QPixmap(centerFileName);
  m_centerButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_centerButton->setIcon(centerPix);
  layout->addWidget(m_centerButton);
  m_centerButton->setCheckable(true);
  m_centerButton->setEnabled(false);
  connect(m_centerButton,&QAbstractButton::clicked,this,&MainWindow::slotCenterButtonClicked);

  
  //   m_accels.setEnabled(true);
  
  QString iconFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("ksirkskineditor.png"));
/*  if (iconFileName.isNull())
  {
      KMessageBox::error(0, i18n("Cannot load icon<br>Program cannot continue"));
      exit(2);
  }*/
  QPixmap icon(iconFileName);

//    qCDebug(KSIRKSKINEDITOR_LOG) << "Before initActions";
  initActions();

  m_skinDefWidget = new KSirkSkinDefinitionWidget(this);
  addDockWidget ( Qt::LeftDockWidgetArea, m_skinDefWidget);
  
  connect(m_skinDefWidget->fontrequester, &KFontRequester::fontSelected, this, &MainWindow::slotFontSelected);
  connect(m_skinDefWidget->fgcolorbutton, &KColorButton::changed, this, &MainWindow::slotFgSelected);
  connect(m_skinDefWidget->bgcolorbutton, &KColorButton::changed, this, &MainWindow::slotBgColorSelected);

  m_countryDefWidget = new KsirkCountryDefinitionWidget(this);
  m_countryDefWidget->neighbourslist->setSortingEnabled(true);
  m_countryDefWidget->neighbourslist->setSelectionMode(QAbstractItemView::ExtendedSelection);
  addDockWidget ( Qt::RightDockWidgetArea, m_countryDefWidget);
  m_countryDefWidget->hide();
  
  m_continentDefWidget = new KsirkContinentDefinitionWidget(this);
  m_continentDefWidget->countrieslist->setSortingEnabled(true);
  m_continentDefWidget->countrieslist->setSelectionMode(QAbstractItemView::ExtendedSelection);
  addDockWidget ( Qt::RightDockWidgetArea, m_continentDefWidget);
  m_continentDefWidget->hide();
  
  m_goalDefWidget = new KsirkGoalDefinitionWidget(this);
  addDockWidget ( Qt::RightDockWidgetArea, m_goalDefWidget);
  m_goalDefWidget->hide();
  connect(m_goalDefWidget->worldtype,&QAbstractButton::clicked,this,&MainWindow::slotGoalTypeWorldClicked);
  connect(m_goalDefWidget->playertype,&QAbstractButton::clicked,this,&MainWindow::slotGoalTypePlayerClicked);
  connect(m_goalDefWidget->countriestype,&QAbstractButton::clicked,this,&MainWindow::slotGoalTypeCountriesClicked);
  connect(m_goalDefWidget->continentstype,&QAbstractButton::clicked,this,&MainWindow::slotGoalTypeContinentsClicked);
  connect (m_goalDefWidget->description,&QTextEdit::textChanged, this, &MainWindow::slotGoalDescriptionEdited);
  connect (m_goalDefWidget->nbcountries, &QSpinBox::valueChanged, this, &MainWindow::slotGoalNbCountriesChanged);
  connect (m_goalDefWidget->armiesbycountry, &QSpinBox::valueChanged, this, &MainWindow::slotGoalNbArmiesByCountryChanged);
  connect (m_goalDefWidget->anycontinent,&QCheckBox::toggled, this, &MainWindow::slotGoalAnyContinentChanged);
  connect (m_goalDefWidget->selectcontinentsbutton, &QAbstractButton::clicked, this, &MainWindow::slotGoalContinents);
  connect(m_skinDefWidget->newGoalButton, &QAbstractButton::clicked, this, &MainWindow::slotNewGoal);
  connect(m_skinDefWidget->deleteGoalButton, &QAbstractButton::clicked, this, &MainWindow::slotDeleteGoal);
  
  m_nationalityDefWidget = new KsirkNationalityDefinitionWidget(this);
  addDockWidget ( Qt::RightDockWidgetArea, m_nationalityDefWidget);
  m_nationalityDefWidget->hide();
  
  m_spritesDefWidget = new KsirkSpritesDefinitionWidget(this);
  addDockWidget ( Qt::RightDockWidgetArea, m_spritesDefWidget);
  m_spritesDefWidget->hide();
  connect(m_spritesDefWidget->flagw, &QSpinBox::valueChanged, this, &MainWindow::slotFlagWidthChanged);
  connect(m_spritesDefWidget->flagh, &QSpinBox::valueChanged, this, &MainWindow::slotFlagHeightChanged);
  connect(m_spritesDefWidget->flagf, &QSpinBox::valueChanged, this, &MainWindow::slotFlagFramesChanged);
  connect(m_spritesDefWidget->flagv, &QSpinBox::valueChanged, this, &MainWindow::slotFlagVersionsChanged);

  connect(m_spritesDefWidget->infantryw, &QSpinBox::valueChanged, this, &MainWindow::slotInfantryWidthChanged);
  connect(m_spritesDefWidget->infantryh, &QSpinBox::valueChanged, this, &MainWindow::slotInfantryHeightChanged);
  connect(m_spritesDefWidget->infantryf, &QSpinBox::valueChanged, this, &MainWindow::slotInfantryFramesChanged);
  connect(m_spritesDefWidget->infantryv, &QSpinBox::valueChanged, this, &MainWindow::slotInfantryVersionsChanged);
  
  connect(m_spritesDefWidget->cavalryw, &QSpinBox::valueChanged, this, &MainWindow::slotCavalryWidthChanged);
  connect(m_spritesDefWidget->cavalryh, &QSpinBox::valueChanged, this, &MainWindow::slotCavalryHeightChanged);
  connect(m_spritesDefWidget->cavalryf, &QSpinBox::valueChanged, this, &MainWindow::slotCavalryFramesChanged);
  connect(m_spritesDefWidget->cavalryv, &QSpinBox::valueChanged, this, &MainWindow::slotCavalryVersionsChanged);
  
  connect(m_spritesDefWidget->cannonw, &QSpinBox::valueChanged, this, &MainWindow::slotCannonWidthChanged);
  connect(m_spritesDefWidget->cannonh, &QSpinBox::valueChanged, this, &MainWindow::slotCannonHeightChanged);
  connect(m_spritesDefWidget->cannonf, &QSpinBox::valueChanged, this, &MainWindow::slotCannonFramesChanged);
  connect(m_spritesDefWidget->cannonv, &QSpinBox::valueChanged, this, &MainWindow::slotCannonVersionsChanged);
  
  connect(m_spritesDefWidget->firingw, &QSpinBox::valueChanged, this, &MainWindow::slotFiringWidthChanged);
  connect(m_spritesDefWidget->firingh, &QSpinBox::valueChanged, this, &MainWindow::slotFiringHeightChanged);
  connect(m_spritesDefWidget->firingf, &QSpinBox::valueChanged, this, &MainWindow::slotFiringFramesChanged);
  connect(m_spritesDefWidget->firingv, &QSpinBox::valueChanged, this, &MainWindow::slotFiringVersionsChanged);
  
  connect(m_spritesDefWidget->explodingw, &QSpinBox::valueChanged, this, &MainWindow::slotExplodingWidthChanged);
  connect(m_spritesDefWidget->explodingh, &QSpinBox::valueChanged, this, &MainWindow::slotExplodingHeightChanged);
  connect(m_spritesDefWidget->explodingf, &QSpinBox::valueChanged, this, &MainWindow::slotExplodingFramesChanged);
  connect(m_spritesDefWidget->explodingv, &QSpinBox::valueChanged, this, &MainWindow::slotExplodingVersionsChanged);
  
  
  connect(m_skinDefWidget->qtabwidget, &QTabWidget::currentChanged, this, &MainWindow::slotSkinPartTabChanged);
  m_skinDefWidget->qtabwidget-> setCurrentIndex(0);
  
  m_skinDefWidget->countrieslist->setSortingEnabled (true);
  connect(m_skinDefWidget->countrieslist, &QListWidget::itemClicked, this, &MainWindow::slotCountrySelected);

  connect(m_skinDefWidget->goalslist, &QListWidget::itemClicked, this, &MainWindow::slotGoalSelected);
  
  connect (m_countryDefWidget->neighboursbutton, &QAbstractButton::clicked, this, &MainWindow::slotNeighbours);
  connect(m_countryDefWidget->flagx, &QSpinBox::valueChanged, this, &MainWindow::slotFLagxValueChanged);
  connect(m_countryDefWidget->flagy, &QSpinBox::valueChanged, this, &MainWindow::slotFLagyValueChanged);
  connect(m_countryDefWidget->centerx, &QSpinBox::valueChanged, this, &MainWindow::slotCenterxValueChanged);
  connect(m_countryDefWidget->centery, &QSpinBox::valueChanged, this, &MainWindow::slotCenteryValueChanged);
  connect(m_countryDefWidget->anchorx, &QSpinBox::valueChanged, this, &MainWindow::slotAnchorxValueChanged);
  connect(m_countryDefWidget->anchory, &QSpinBox::valueChanged, this, &MainWindow::slotAnchoryValueChanged);
  connect(m_countryDefWidget->infantryx, &QSpinBox::valueChanged, this, &MainWindow::slotInfantryxValueChanged);
  connect(m_countryDefWidget->infantryy, &QSpinBox::valueChanged, this, &MainWindow::slotInfantryyValueChanged);
  connect(m_countryDefWidget->cavalryx, &QSpinBox::valueChanged, this, &MainWindow::slotCavalryxValueChanged);
  connect(m_countryDefWidget->cavalryy, &QSpinBox::valueChanged, this, &MainWindow::slotCavalryyValueChanged);
  connect(m_countryDefWidget->cannonx, &QSpinBox::valueChanged, this, &MainWindow::slotCannonxValueChanged);
  connect(m_countryDefWidget->cannony, &QSpinBox::valueChanged, this, &MainWindow::slotCannonyValueChanged);
  

  connect (m_skinDefWidget->skinNameLineEdit, &QLineEdit::editingFinished, this, &MainWindow::slotSkinNameEdited);

  connect (m_skinDefWidget->widthLineEdit, &QSpinBox::valueChanged, this, &MainWindow::slotSkinWidthEdited);
  connect (m_skinDefWidget->heightLineEdit, &QSpinBox::valueChanged, this, &MainWindow::slotSkinHeightEdited);

  connect (m_skinDefWidget->descriptionTextEdit,&QTextEdit::textChanged, this, &MainWindow::slotSkinDescriptionEdited);

  QIntValidator* widthDiffValidator = new QIntValidator(this);
  m_spritesDefWidget->widthDiffLineEdit->setValidator(widthDiffValidator);
  connect (m_spritesDefWidget->widthDiffLineEdit, &QLineEdit::editingFinished, this, &MainWindow::slotSkinWidthDiffEdited);

  m_skinDefWidget->nationalitieslist->setSortingEnabled (true);
  connect(m_skinDefWidget->nationalitieslist, &QListWidget::itemClicked, this, &MainWindow::slotNationalitySelected);
  connect(m_skinDefWidget->newNationalityButton, &QAbstractButton::clicked, this, &MainWindow::slotNewNationality);
  connect(m_skinDefWidget->deleteNationalityButton, &QAbstractButton::clicked, this, &MainWindow::slotDeleteNationality);

  connect(m_nationalityDefWidget->name,&QLineEdit::editingFinished, this, &MainWindow::slotNationalityNameEdited);
  connect(m_nationalityDefWidget->leader,&QLineEdit::editingFinished, this, &MainWindow::slotNationalityLeaderNameEdited);
  connect(m_nationalityDefWidget->flag, &KComboBox::currentIndexChanged, this, &MainWindow::slotNationalityFlagEdited);
  
  
  connect(m_skinDefWidget->newCountryButton, &QAbstractButton::clicked, this, &MainWindow::slotNewCountry);
  connect(m_skinDefWidget->deleteCountryButton, &QAbstractButton::clicked, this, &MainWindow::slotDeleteCountry);    

  m_skinDefWidget->continentslist->setSortingEnabled (true);
  connect(m_skinDefWidget->continentslist, &QListWidget::itemClicked, this, &MainWindow::slotContinentSelected);

  connect(m_skinDefWidget->newContinentButton, &QAbstractButton::clicked, this, &MainWindow::slotNewContinent);
  connect(m_skinDefWidget->deleteContinentButton, &QAbstractButton::clicked, this, &MainWindow::slotDeleteContinent);
  
  connect (m_continentDefWidget->selectcountriesbutton, &QAbstractButton::clicked, this, &MainWindow::slotContinentCountries);
  
  connect(m_continentDefWidget->bonus, &QSpinBox::valueChanged, this, &MainWindow::slotContinentBonusEdited);
  
  qCDebug(KSIRKSKINEDITOR_LOG) << "Setting up GUI";
  setupGUI();
  
  //    qCDebug(KSIRKSKINEDITOR_LOG) << "Before initStatusBar";
  initStatusBar();
  
  menuBar()-> show();
  
  setMouseTracking(true);
  
}

MainWindow::~MainWindow()
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  KSharedConfig::Ptr config = KSharedConfig::openConfig();
  if (m_rfa != nullptr)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "saving recent files";
    m_rfa->saveEntries(KSharedConfig::openConfig()->group(QStringLiteral("ksirkskineditor")));
  }
}

void MainWindow::initActions()
{
  QAction *action;
  // standard game actions
  action = KGameStandardAction::load(this, &MainWindow::slotOpenSkin, this);
  actionCollection()->addAction(action->objectName(), action);
  action->setToolTip(i18n("Open a saved skin..."));

  m_rfa = KGameStandardAction::loadRecent (this, &MainWindow::slotURLSelected, this);
  actionCollection()->addAction(m_rfa->objectName(), m_rfa);
  m_rfa->setText(i18n("Load &Recent"));
  m_rfa->setToolTip(i18n("Open a recently saved skin..."));

  KSharedConfig::Ptr config = KSharedConfig::openConfig();
  qCDebug(KSIRKSKINEDITOR_LOG) << "loading recent files";
  m_rfa->loadEntries(KSharedConfig::openConfig()->group(QStringLiteral("ksirkskineditor")));
  
  m_saveGameAction = KGameStandardAction::save(this, &MainWindow::slotSaveSkin, this);
  actionCollection()->addAction(m_saveGameAction->objectName(), m_saveGameAction);
  m_saveGameAction->setToolTip(i18n("Save the current skin"));

  action = KGameStandardAction::quit(this, &MainWindow::close, this);
  actionCollection()->addAction(action->objectName(), action);

//   action = KGameStandardAction::gameNew(this, &MainWindow::slotNewGame, this);
//   actionCollection()->addAction(action->objectName(), action);

//   action = KStandardAction::zoomIn(this, &MainWindow::slotZoomIn, this);
//   actionCollection()->addAction(action->objectName(), action);

//   action = KStandardAction::zoomOut(this, &MainWindow::slotZoomOut, this);
//   actionCollection()->addAction(action->objectName(), action);

  KStandardAction::preferences( this, &MainWindow::optionsConfigure, actionCollection() );

}

void MainWindow::initStatusBar()
{
  statusBar()-> setSizeGripEnabled(true);
}

void MainWindow::slotOpenSkin()
{
  const QString skinDir = QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Choose the Root Folder of the Skin to Open"), QString());
  if (skinDir.isEmpty())
  {
    return;
  }

  openSkin(skinDir);
}

void MainWindow::openSkin(const QString& skinDir)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << skinDir;
  if (m_rfa != nullptr)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Adding" << skinDir << "to recent files";
    m_rfa->addUrl(QUrl::fromLocalFile(skinDir));
  }
  
  m_mapScene->clear();
  m_skinDefWidget->countrieslist->clear();
  m_skinDefWidget->continentslist->clear();
  m_skinDefWidget->goalslist->clear();
  
  delete m_onu;
  m_onu = new ONU(skinDir, this);

  m_skinDefWidget->fontrequester->setFont(m_onu->foregroundFont());
  m_skinDefWidget->fgcolorbutton->setColor(m_onu->foregroundColor());
  m_skinDefWidget->bgcolorbutton->setColor(m_onu->backgroundColor());
  
  m_skinDefWidget->skinNameLineEdit->setText(m_onu->name());
  m_skinDefWidget->widthLineEdit->setValue(m_onu->width());
  m_skinDefWidget->heightLineEdit->setValue(m_onu->height());
  m_skinDefWidget->descriptionTextEdit->setText(m_onu->description());
  m_spritesDefWidget->widthDiffLineEdit->setText(QString::number(SkinSpritesData::changeable().intData(QStringLiteral("width-between-flag-and-fighter"))));

  m_spritesDefWidget->flagw->setValue(SkinSpritesData::changeable().intData(QStringLiteral("flag-width")));
  m_spritesDefWidget->flagh->setValue(SkinSpritesData::changeable().intData(QStringLiteral("flag-height")));
  m_spritesDefWidget->flagf->setValue(SkinSpritesData::changeable().intData(QStringLiteral("flag-frames")));
  m_spritesDefWidget->flagv->setValue(SkinSpritesData::changeable().intData(QStringLiteral("flag-versions")));
  
  m_spritesDefWidget->infantryw->setValue(SkinSpritesData::changeable().intData(QStringLiteral("infantry-width")));
  m_spritesDefWidget->infantryh->setValue(SkinSpritesData::changeable().intData(QStringLiteral("infantry-height")));
  m_spritesDefWidget->infantryf->setValue(SkinSpritesData::changeable().intData(QStringLiteral("infantry-frames")));
  m_spritesDefWidget->infantryv->setValue(SkinSpritesData::changeable().intData(QStringLiteral("infantry-versions")));
  
  m_spritesDefWidget->cavalryw->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cavalry-width")));
  m_spritesDefWidget->cavalryh->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cavalry-height")));
  m_spritesDefWidget->cavalryf->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cavalry-frames")));
  m_spritesDefWidget->cavalryv->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cavalry-versions")));
  
  m_spritesDefWidget->cannonw->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cannon-width")));
  m_spritesDefWidget->cannonh->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cannon-height")));
  m_spritesDefWidget->cannonf->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cannon-frames")));
  m_spritesDefWidget->cannonv->setValue(SkinSpritesData::changeable().intData(QStringLiteral("cannon-versions")));
  
  m_spritesDefWidget->firingw->setValue(SkinSpritesData::changeable().intData(QStringLiteral("firing-width")));
  m_spritesDefWidget->firingh->setValue(SkinSpritesData::changeable().intData(QStringLiteral("firing-height")));
  m_spritesDefWidget->firingf->setValue(SkinSpritesData::changeable().intData(QStringLiteral("firing-frames")));
  m_spritesDefWidget->firingv->setValue(SkinSpritesData::changeable().intData(QStringLiteral("firing-versions")));
  
  m_spritesDefWidget->explodingw->setValue(SkinSpritesData::changeable().intData(QStringLiteral("exploding-width")));
  m_spritesDefWidget->explodingh->setValue(SkinSpritesData::changeable().intData(QStringLiteral("exploding-height")));
  m_spritesDefWidget->explodingf->setValue(SkinSpritesData::changeable().intData(QStringLiteral("exploding-frames")));
  m_spritesDefWidget->explodingv->setValue(SkinSpritesData::changeable().intData(QStringLiteral("exploding-versions")));

  m_nationalityDefWidget->flag->clear();
  foreach(const QString& key, m_onu->poolIds())
  {
    int flagWidth = SkinSpritesData::changeable().intData(QStringLiteral("flag-width"));
    int flagHeight = SkinSpritesData::changeable().intData(QStringLiteral("flag-height"));
    int flagFrames = SkinSpritesData::changeable().intData(QStringLiteral("flag-frames"));
    int flagVersions = SkinSpritesData::changeable().intData(QStringLiteral("flag-versions"));
    QPixmap flagIcon = m_onu->pixmapForId(key,flagWidth*flagFrames,flagHeight*flagVersions).copy(0,0,flagWidth,flagHeight);
    m_nationalityDefWidget->flag->insertItem(m_nationalityDefWidget->flag->count(),QIcon(flagIcon),key);
  }

  
  QPixmap mapPixmap(m_onu->pixmapForId(QStringLiteral("map"), m_onu->width(), m_onu->height()));
  m_mapItem = m_mapScene->addPixmap(mapPixmap);
  
  m_flagButton->setIcon(m_onu->flagIcon());
  //   m_flagButton->setIconSize(QSize(flagWidth,flagHeight));
  
  m_infantryButton->setIcon(m_onu->infantryIcon());
  //   m_infantryButton->setIconSize(QSize(infantryWidth,infantryHeight));
  
  m_cavalryButton->setIcon(m_onu->cavalryIcon());
  //   m_cavalryButton->setIconSize(QSize(cavalryWidth,cavalryHeight));
  
  m_cannonButton->setIcon(m_onu->cannonIcon());
  //   m_cannonButton->setIconSize(QSize(cannonWidth,cannonHeight));
  
  qCDebug(KSIRKSKINEDITOR_LOG) << "Adding nationalities items";
  foreach (Nationality* nationality, m_onu->nationalities())
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Adding "<<nationality->name()<<" items";
    m_skinDefWidget->nationalitieslist->addItem(nationality->name());
  }
  
  qCDebug(KSIRKSKINEDITOR_LOG) << "Adding countries items";
  foreach (Country* country, m_onu->countries())
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Adding "<<country->name()<<" items";
    m_skinDefWidget->countrieslist->addItem(country->name());
    
    if (!country->pointFlag().isNull())
    {
      createPixmapFor(country, Flag);
    }
    if (!country->pointInfantry().isNull())
    {
      createPixmapFor(country, Infantry);
    }
    if (!country->pointCavalry().isNull())
    {
      createPixmapFor(country, Cavalry);
    }
    if (!country->pointCannon().isNull())
    {
      createPixmapFor(country, Cannon);
    }
    if (!country->anchorPoint().isNull())
    {
      createPixmapFor(country, Anchor);
    }
    if (!country->centralPoint().isNull())
    {
      createPixmapFor(country, Center);
    }
  }

  qCDebug(KSIRKSKINEDITOR_LOG) << "Adding continents items";
  foreach (Continent* continent, m_onu->continents())
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Adding "<<continent<<" items";
    qCDebug(KSIRKSKINEDITOR_LOG) << "Adding "<<continent->name()<<" items";
    m_skinDefWidget->continentslist->addItem(continent->name());
  }

  qCDebug(KSIRKSKINEDITOR_LOG) << "Adding goals items";
  for (int i = 1; i <= m_onu->goals().size(); i++)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "Adding goal"<<i<<" items";
    m_skinDefWidget->goalslist->addItem("goal" + QString::number(i));
  }
}

void MainWindow::slotSaveSkin()
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  m_onu->saveConfig();
}

/**
  * Reimplementation of the inherited function called when a window close event arise
  */
bool MainWindow::queryClose()
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  // TODO : Test si jeu en cours
  if (m_onu && m_onu->dirty())
  {
    switch (KMessageBox::warningTwoActionsCancel(this,
                                             i18n("There are unsaved changes. What do you want to do?"),
                                             i18n("Exit Anyway?"),
                                             KGuiItem(i18n("Quit without saving")),
                                             KGuiItem(i18n("Save then quit")),
                                             KGuiItem(i18n("Do not quit"))))
    {
      case KMessageBox::PrimaryAction:
        return true;
        break;
      case KMessageBox::SecondaryAction:
        m_onu->saveConfig();
        return true;
        break;
      default:
        return false;
    }
  }
  KSharedConfig::openConfig()->sync();
  return true;
}

void MainWindow::optionsConfigure()
{
  //An instance of your dialog could be already created and could be cached, 
  //in which case you want to display the cached dialog instead of creating 
  //another one 
  if ( KsirkSkinEditorConfigurationDialog::showDialog( QStringLiteral("settings") ) ) 
    return;
 
  //KConfigDialog didn't find an instance of this dialog, so lets create it : 
  KsirkSkinEditorConfigurationDialog* dialog = new KsirkSkinEditorConfigurationDialog(
      this, "settings",
      KsirkSkinEditorSettings::self() ); 

  dialog->show();
}

// void MainWindow::slotZoomIn()
// {
//   qCDebug(KSIRKSKINEDITOR_LOG);
// }

// void MainWindow::slotZoomOut()
// {
//   qCDebug(KSIRKSKINEDITOR_LOG);
// }

void MainWindow::slotShowAboutApplication()
{
#if 0 //QT5
  KAboutApplicationDialog dialog(KGlobal::mainComponent().aboutData(), this);
  dialog.exec();
#endif
}

void MainWindow::slotFlagButtonClicked()
{
  m_infantryButton->setChecked(false);
  m_cavalryButton->setChecked(false);
  m_cannonButton->setChecked(false);
  m_anchorButton->setChecked(false);
  m_centerButton->setChecked(false);
  if (m_selectedSprite == Flag)
  {
    m_flagButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=nullptr)
  {
    m_mapView->setCursor(QCursor(m_onu->flagIcon(),0,0));
    m_selectedSprite = Flag;
    m_flagButton->setChecked(true);
  }
}

void MainWindow::slotInfantryButtonClicked()
{
  m_flagButton->setChecked(false);
  m_cavalryButton->setChecked(false);
  m_cannonButton->setChecked(false);
  m_anchorButton->setChecked(false);
  m_centerButton->setChecked(false);
  if (m_selectedSprite == Infantry)
  {
    m_infantryButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=nullptr)
  {
    m_mapView->setCursor(QCursor(m_onu->infantryIcon(),0,0));
    m_selectedSprite = Infantry;
    m_infantryButton->setChecked(true);
  }
}

void MainWindow::slotCavalryButtonClicked()
{
  m_flagButton->setChecked(false);
  m_infantryButton->setChecked(false);
  m_cannonButton->setChecked(false);
  m_anchorButton->setChecked(false);
  m_centerButton->setChecked(false);
  if (m_selectedSprite == Cavalry)
  {
    m_cavalryButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=nullptr)
  {
    m_mapView->setCursor(QCursor(m_onu->cavalryIcon(),0,0));
    m_selectedSprite = Cavalry;
    m_cavalryButton->setChecked(true);
  }
}

void MainWindow::slotCannonButtonClicked()
{
  m_flagButton->setChecked(false);
  m_infantryButton->setChecked(false);
  m_cavalryButton->setChecked(false);
  m_anchorButton->setChecked(false);
  m_centerButton->setChecked(false);
  if (m_selectedSprite == Cannon)
  {
    m_cannonButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=nullptr)
  {
    m_mapView->setCursor(QCursor(m_onu->cannonIcon(),0,0));
    m_selectedSprite = Cannon;
    m_cannonButton->setChecked(true);
  }
}

void MainWindow::slotAnchorButtonClicked()
{
  m_flagButton->setChecked(false);
  m_infantryButton->setChecked(false);
  m_cavalryButton->setChecked(false);
  m_cannonButton->setChecked(false);
  m_centerButton->setChecked(false);
  if (m_selectedSprite == Anchor)
  {
    m_anchorButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=nullptr)
  {
    QString anchorFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("cross.png"));
    if (anchorFileName.isNull())
    {
      KMessageBox::error(nullptr, i18n("Cannot load anchor icon<br>Program cannot continue"));
      exit(2);
    }
    QPixmap anchorPix = QPixmap(anchorFileName);
    anchorPix = anchorPix.scaled(16,16);
    
    m_mapView->setCursor(QCursor(anchorPix,0,0));
    m_selectedSprite = Anchor;
    m_anchorButton->setChecked(true);
  }
}

void MainWindow::slotCenterButtonClicked()
{
  m_flagButton->setChecked(false);
  m_infantryButton->setChecked(false);
  m_cavalryButton->setChecked(false);
  m_cannonButton->setChecked(false);
  m_anchorButton->setChecked(false);
  if (m_selectedSprite == Center)
  {
    m_centerButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=nullptr)
  {
    QString centerFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("target.png"));
    if (centerFileName.isNull())
    {
      KMessageBox::error(nullptr, i18n("Cannot load center icon<br>Program cannot continue"));
      exit(2);
    }
    QPixmap centerPix = QPixmap(centerFileName);
    centerPix = centerPix.scaled(16,16);
    m_mapView->setCursor(QCursor(centerPix,0,0));
    m_selectedSprite = Center;
    m_centerButton->setChecked(true);
  }
}

void MainWindow::slotPosition(const QPointF& point)
{
  QString message(QLatin1String(""));
  QTextStream ts( &message );
  ts << point.x() << " x " << point.y();
  statusBar()->showMessage(message);
//   qCDebug(KSIRKSKINEDITOR_LOG) << "selected sprite:" << m_selectedSprite;
  if (currentCountry() != nullptr && m_selectedSprite == Anchor
    && m_onu->itemFor(currentCountry(), m_selectedSprite) != nullptr && m_updateHighlightPosition
    && currentCountry()->highlighting() != nullptr)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << point << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    QGraphicsItem* anchorItem = m_onu->itemFor(currentCountry(), Anchor);
    qCDebug(KSIRKSKINEDITOR_LOG) << "anchorItem=" << anchorItem;
    currentCountry()->anchorPoint(point);
    QPointF anchorPoint = currentCountry()->anchorPoint();

    double hw = currentCountry()->highlighting()->boundingRect().width();
    double hh = currentCountry()->highlighting()->boundingRect().height();
    currentCountry()->highlighting()->setPos(anchorPoint - QPointF(hw/2,hh/2 ));

    return;
  }
}

void MainWindow::slotPressPosition(const QPointF& clickedPoint)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << clickedPoint << (void*)currentCountry() << m_selectedSprite;
  QPixmap pix;
  QPixmap alphacopy;
  QString fileName;
  QPointF point = clickedPoint;
  m_updateHighlightPosition = true;
  if (currentCountry() == nullptr
    || m_onu->itemFor(currentCountry(), m_selectedSprite) != nullptr)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    if (currentCountry() != nullptr)
    {
      currentCountry()->clearHighlighting();
    }
    return;
  }
  PixmapItem* item = new PixmapItem();
  item->setZValue(10);
  connect(item, &PixmapItem::pressed,this, &MainWindow::slotItemPressed);
  connect(item, &PixmapItem::placed,this, &MainWindow::slotItemPlaced);
  
  switch (m_selectedSprite)
  {
    case Flag:
      pix = m_onu->flagIcon();
//       pix = pix.scaled(20,20);
      m_countryDefWidget->flagx->setValue(point.x());
      m_countryDefWidget->flagy->setValue(point.y());
      currentCountry()->pointFlag(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Flag));
      break;
    case Infantry:
      pix = m_onu->infantryIcon();
//       pix = pix.scaled(23,32);
      m_countryDefWidget->infantryx->setValue(point.x());
      m_countryDefWidget->infantryy->setValue(point.y());
      currentCountry()->pointInfantry(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Infantry));
      break;
    case Cavalry:
      pix = m_onu->cavalryIcon();
//       pix = pix.scaled(32,32);
      m_countryDefWidget->cavalryx->setValue(point.x());
      m_countryDefWidget->cavalryy->setValue(point.y());
      currentCountry()->pointCavalry(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Cavalry));
      break;
    case Cannon:
      qCDebug(KSIRKSKINEDITOR_LOG) << "Adding cannon";
      pix = m_onu->cannonIcon();
//       pix = pix.scaled(32,32);
      m_countryDefWidget->cannonx->setValue(point.x());
      m_countryDefWidget->cannony->setValue(point.y());
      currentCountry()->pointCannon(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Cannon));
      break;
    case Anchor:
      qCDebug(KSIRKSKINEDITOR_LOG) << "Adding anchor";
      fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("cross.png"));
      if (fileName.isNull())
      {
        KMessageBox::error(nullptr, i18n("Cannot load anchor icon<br>Program cannot continue"));
        exit(2);
      }
      pix = QPixmap(fileName);
      pix = pix.scaled(16,16);
      point += QPointF(-pix.width()/2,-pix.height()/2);
      m_countryDefWidget->anchorx->setValue(point.x());
      m_countryDefWidget->anchory->setValue(point.y());
      currentCountry()->anchorPoint(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Anchor));
      m_updateHighlightPosition = true;
      break;
    case Center:
      qCDebug(KSIRKSKINEDITOR_LOG) << "Adding center";
      fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("target.png"));
      if (fileName.isNull())
      {
        KMessageBox::error(nullptr, i18n("Cannot load center icon<br>Program cannot continue"));
        exit(2);
      }
      pix = QPixmap(fileName);
      pix = pix.scaled(16,16);
      point += QPointF(-pix.width()/2,-pix.height()/2);
      m_countryDefWidget->centerx->setValue(point.x());
      m_countryDefWidget->centery->setValue(point.y());
      currentCountry()->centralPoint(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Center));
      break;
    default: ;
  }
  if (item != nullptr)
  {
    item->setPixmap(pix);
    m_mapScene->addItem(item);
    item->setFlag(QGraphicsItem::ItemIsMovable, true);
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    item->setPos(point);
  }
}

void MainWindow::slotReleasePosition(const QPointF& point)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << point;
  m_updateHighlightPosition = false;

  qCDebug(KSIRKSKINEDITOR_LOG) << "selected sprite:" << m_selectedSprite;
  if (currentCountry() != nullptr && m_selectedSprite == Anchor
    && m_onu->itemFor(currentCountry(), m_selectedSprite) != nullptr && m_updateHighlightPosition
    && currentCountry()->highlighting() != nullptr)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    currentCountry()->anchorPoint(point);
    currentCountry()->highlighting()->setPos((currentCountry()->anchorPoint().x()-currentCountry()->highlighting()->boundingRect().width()/2),(currentCountry()->anchorPoint().y()-currentCountry()->highlighting()->boundingRect().height()/2));
    
    return;
  }
}

void MainWindow::slotNationalitySelected(QListWidgetItem* item)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  Nationality* nationality = m_onu->nationalityNamed(item->text());
  if (nationality != nullptr)
  {
    initNationalityWidgetWith(nationality);
  }
}

void MainWindow::initNationalityWidgetWith(Nationality* nationality)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  m_nationalityDefWidget->name->setText(nationality->name());
  m_nationalityDefWidget->leader->setText(nationality->leaderName());
  int item = m_nationalityDefWidget->flag->findText(nationality->flagFileName());
  if (item != -1)
  {
      m_nationalityDefWidget->flag->setCurrentIndex(item);
  }
}

void MainWindow::slotCountrySelected(QListWidgetItem* item)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  Country* country = m_onu->countryNamed(item->text());
  if (country != nullptr)
  {
    m_countryDefWidget->initWith(country);
    initSpritesButtonsWith(country);
  }
}

void MainWindow::slotContinentSelected(QListWidgetItem* item)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  Continent* continent = m_onu->continentNamed(item->text());
  if (continent != nullptr)
  {
    initContinentWidgetWith(continent);
  }
}

void MainWindow::initContinentWidgetWith(Continent* continent)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  m_continentDefWidget->bonus->setValue(continent->bonus());

  m_continentDefWidget->countrieslist->clear();
  foreach(Country* country, continent->members())
  {
    m_continentDefWidget->countrieslist->addItem(country->name());
  }
}

void MainWindow::slotGoalSelected(QListWidgetItem* item)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  int row = m_skinDefWidget->goalslist->row(item);
  Goal* goal = m_onu->goals()[row];
  if (goal != nullptr)
  {
    initGoalWidgetWith(goal);
  }
}

void MainWindow::initGoalWidgetWith(Goal* goal)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  switch (goal->type())
  {
    case Goal::NoGoal:
      m_goalDefWidget->worldtype->setChecked(true);
      m_goalDefWidget->playertype->setChecked(false);
      m_goalDefWidget->countriestype->setChecked(false);
      m_goalDefWidget->continentstype->setChecked(false);
      break;
    case Goal::GoalPlayer:
      m_goalDefWidget->worldtype->setChecked(false);
      m_goalDefWidget->playertype->setChecked(true);
      m_goalDefWidget->countriestype->setChecked(false);
      m_goalDefWidget->continentstype->setChecked(false);
      break;
    case Goal::Countries:
      m_goalDefWidget->worldtype->setChecked(false);
      m_goalDefWidget->playertype->setChecked(false);
      m_goalDefWidget->countriestype->setChecked(true);
      m_goalDefWidget->continentstype->setChecked(false);
      break;
    case Goal::Continents:
      m_goalDefWidget->worldtype->setChecked(false);
      m_goalDefWidget->playertype->setChecked(false);
      m_goalDefWidget->countriestype->setChecked(false);
      m_goalDefWidget->continentstype->setChecked(true);
      break;
    default:;
  }
  m_goalDefWidget->description->setText(goal->description());
  m_goalDefWidget->nbcountries->setValue(goal->nbCountries());
  m_goalDefWidget->armiesbycountry->setValue(goal->nbArmiesByCountry());
  m_goalDefWidget->anycontinent->setChecked(false);
  m_goalDefWidget->continentslist->clear();
  foreach(const QString& id, goal->continents())
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "continent" << id;
    if (id.isNull())
    {
      m_goalDefWidget->anycontinent->setChecked(true);
    }
    foreach(Continent* continent, m_onu->continents())
    {
      if (continent->name() == id)
      {
        m_goalDefWidget->continentslist->addItem(continent->name());
      }
    }
  }
}

Country* MainWindow::currentCountry()
{
  if (m_skinDefWidget->countrieslist->currentItem() == nullptr)
    return nullptr;
//   qCDebug(KSIRKSKINEDITOR_LOG) << m_skinDefWidget->countrieslist->currentItem()->text();
  return m_onu->countryNamed(m_skinDefWidget->countrieslist->currentItem()->text());
}

void MainWindow::slotItemPlaced(QGraphicsItem* item, const QPointF&)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_onu->itemsMap().contains(item))
  {
    Country* country = m_onu->itemsMap()[item].first;
    SpriteType type = m_onu->itemsMap()[item].second;
    m_countryDefWidget->initWith(country);
    for (int i = 0; i != m_skinDefWidget->countrieslist->count(); i++)
    {
      if (m_skinDefWidget->countrieslist->item(i)->text() == country->name())
      {
        m_skinDefWidget->countrieslist->setCurrentRow (i, QItemSelectionModel::ClearAndSelect);
        break;
      }
    }
    QPointF anchorPos;
    QPointF centerPos;
    switch (type)
    {
      case Flag:
        country->pointFlag(item->scenePos());
        m_countryDefWidget->flagx->setValue(item->scenePos().x());
        m_countryDefWidget->flagy->setValue(item->scenePos().y());
        m_flagButton->setChecked(false);
        m_flagButton->setEnabled(false);
        m_mapView->unsetCursor();
        break;
      case Infantry:
        country->pointInfantry(item->scenePos());
        m_countryDefWidget->infantryx->setValue(item->scenePos().x());
        m_countryDefWidget->infantryy->setValue(item->scenePos().y());
        m_infantryButton->setChecked(false);
        m_infantryButton->setEnabled(false);
        m_mapView->unsetCursor();
        break;
      case Cavalry:
        country->pointCavalry(item->scenePos());
        m_countryDefWidget->cavalryx->setValue(item->scenePos().x());
        m_countryDefWidget->cavalryy->setValue(item->scenePos().y());
        m_cavalryButton->setChecked(false);
        m_cavalryButton->setEnabled(false);
        m_mapView->unsetCursor();
        break;
      case Cannon:
        country->pointCannon(item->scenePos());
        m_countryDefWidget->cannonx->setValue(item->scenePos().x());
        m_countryDefWidget->cannony->setValue(item->scenePos().y());
        m_cannonButton->setChecked(false);
        m_cannonButton->setEnabled(false);
        m_mapView->unsetCursor();
        break;
      case Anchor:
        anchorPos = QPointF(item->scenePos().x()+(item->boundingRect().width()/2),item->scenePos().y()+(item->boundingRect().height()/2));
        country->anchorPoint(anchorPos);
        m_countryDefWidget->anchorx->setValue(anchorPos.x());
        m_countryDefWidget->anchory->setValue(anchorPos.y());
        m_anchorButton->setChecked(false);
        m_anchorButton->setEnabled(false);
        m_mapView->unsetCursor();
        break;
      case Center:
        centerPos = QPointF(item->scenePos().x()+(item->boundingRect().width()/2),item->scenePos().y()+(item->boundingRect().height()/2));
        country->centralPoint(centerPos);
        m_countryDefWidget->centerx->setValue(centerPos.x());
        m_countryDefWidget->centery->setValue(centerPos.y());
        m_centerButton->setChecked(false);
        m_centerButton->setEnabled(false);
        m_mapView->unsetCursor();
        break;
      default:;
    }
  }
}

void MainWindow::slotItemPressed(QGraphicsItem* item, const QPointF& point)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_onu->itemsMap().contains(item))
  {
    Country* country = m_onu->itemsMap()[item].first;
    m_countryDefWidget->initWith(country);
    for (int i = 0; i != m_skinDefWidget->countrieslist->count(); i++)
    {
      if (m_skinDefWidget->countrieslist->item(i)->text() == country->name())
      {
        m_skinDefWidget->countrieslist->setCurrentRow (i, QItemSelectionModel::ClearAndSelect);
        break;
      }
    }
    switch (m_onu->itemsMap()[item].second)
    {
      case Flag:
        m_selectedSprite = Flag;
        country->clearHighlighting();
        break;
      case Infantry:
        m_selectedSprite = Infantry;
        country->clearHighlighting();
        break;
      case Cavalry:
        m_selectedSprite = Cavalry;
        country->clearHighlighting();
        break;
      case Cannon:
        m_selectedSprite = Cannon;
        country->clearHighlighting();
        break;
      case Anchor:
        m_selectedSprite = Anchor;
        item = m_onu->itemFor(country, Anchor);
        if (item == nullptr)
        {
          qCCritical(KSIRKSKINEDITOR_LOG) << "item " << Anchor << "not found for" << country->name();
          break;
        }
        country->anchorPoint(point);
        item->setPos( point - QPointF(item->boundingRect().width()/2, item->boundingRect().height()/2) );
        country->highlight(m_mapScene, m_onu, Qt::white,128);
//         item->setPos((country->anchorPoint().x()-country->highlighting()->boundingRect().width()/2),
//                      (country->anchorPoint().y()-country->highlighting()->boundingRect().height()/2));
        break;
      case Center:
        m_selectedSprite = Center;
        country->clearHighlighting();
        break;
      default:
        m_selectedSprite = None;
        country->clearHighlighting();
    }
  }
}

void MainWindow::createPixmapFor(Country* country, SpriteType type)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << country->name() << type;
  QPixmap pix;
  QPixmap alphacopy;
  QPointF point;
  QString fileName;
  if (country == nullptr
    || m_onu->itemFor(country, type) != nullptr)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << (void*)country << (void*)m_onu->itemFor(country, type);
    return;
  }
  QGraphicsItem* item = nullptr;
                    
  switch (type)
  {
    case Flag:
      item = new PixmapItem();
      item->setZValue(3);
      pix = m_onu->flagIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      //       pix = pix.scaled(20,20);
      point = country->pointFlag();
      m_countryDefWidget->flagx->setValue(point.x());
      m_countryDefWidget->flagy->setValue(point.y());
      m_onu->itemsMap().insert(item,qMakePair(country,Flag));
      break;
    case Infantry:
      item = new PixmapItem();
      item->setZValue(3);
      pix = m_onu->infantryIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      point = country->pointInfantry();
      //       pix = pix.scaled(23,32);
      m_countryDefWidget->infantryx->setValue(point.x());
      m_countryDefWidget->infantryy->setValue(point.y());
      m_onu->itemsMap().insert(item,qMakePair(country,Infantry));
      break;
    case Cavalry:
      item = new PixmapItem();
      item->setZValue(3);
      pix = m_onu->cavalryIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      point = country->pointCavalry();
      //       pix = pix.scaled(32,32);
      m_countryDefWidget->cavalryx->setValue(point.x());
      m_countryDefWidget->cavalryy->setValue(point.y());
      m_onu->itemsMap().insert(item,qMakePair(country,Cavalry));
      break;
    case Cannon:
      item = new PixmapItem();
      item->setZValue(3);
      qCDebug(KSIRKSKINEDITOR_LOG) << "Adding cannon";
      pix = m_onu->cannonIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      point = country->pointCannon();
      //       pix = pix.scaled(32,32);
      m_countryDefWidget->cannonx->setValue(point.x());
      m_countryDefWidget->cannony->setValue(point.y());
      m_onu->itemsMap().insert(item,qMakePair(country,Cannon));
      break;
    case Anchor:
      item = new PixmapItem();
      item->setZValue(3);
      qCDebug(KSIRKSKINEDITOR_LOG) << "Adding anchor";
      fileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("cross.png"));
      if (fileName.isNull())
      {
        KMessageBox::error(nullptr, i18n("Cannot load anchor icon<br>Program cannot continue"));
        exit(2);
      }
      pix = QPixmap(fileName);
      pix = pix.scaled(16,16);
      ((PixmapItem*)item)->setPixmap(pix);
      point = country->anchorPoint() - QPointF(item->boundingRect().width()/2, item->boundingRect().height()/2);
      m_countryDefWidget->anchorx->setValue(point.x());
      m_countryDefWidget->anchory->setValue(point.y());
      m_onu->itemsMap().insert(item,qMakePair(country,Anchor));
      break;
    case Center:
      qCDebug(KSIRKSKINEDITOR_LOG) << "Adding center";
      item = new TextItem();
      item->setZValue(2);
      ((TextItem*)item)->setFont(m_onu->foregroundFont());
      ((TextItem*)item)->setPlainText(country->name());
      point = country->centralPoint() - QPointF(item->boundingRect().width()/2, item->boundingRect().height()/2);
      qCDebug(KSIRKSKINEDITOR_LOG) << "Adding center" << country->centralPoint() << point;
      m_countryDefWidget->centerx->setValue(point.x());
      m_countryDefWidget->centery->setValue(point.y());
      m_onu->itemsMap().insert(item,qMakePair(country,Center));
      break;
    default: ;
  }
  if (item != nullptr && !point.isNull())
  {
    item->setPos(point);
    if (dynamic_cast<PixmapItem*>(item) != nullptr)
    {
      connect(dynamic_cast<PixmapItem*>(item),&PixmapItem::pressed,this, &MainWindow::slotItemPressed);
      connect(dynamic_cast<PixmapItem*>(item),&PixmapItem::placed,this, &MainWindow::slotItemPlaced);
    }
    else if (dynamic_cast<TextItem*>(item) != nullptr)
    {
      connect(dynamic_cast<TextItem*>(item),&TextItem::pressed,this, &MainWindow::slotItemPressed);
      connect(dynamic_cast<TextItem*>(item), &TextItem::placed,this, &MainWindow::slotItemPlaced);
    }
    m_mapScene->addItem(item);
    item->setFlag(QGraphicsItem::ItemIsMovable, true);
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
  }
}

void MainWindow::slotURLSelected(const QUrl &url)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if ( url.isLocalFile() )
  {
    QString path = url.path();
    openSkin(path);
  }
}

void MainWindow::slotSkinNameEdited()
{
  if (m_onu == nullptr) return;
  m_onu->setName(m_skinDefWidget->skinNameLineEdit->text());
}

void MainWindow::slotSkinWidthEdited(int v)
{
  if (m_onu == nullptr) return;
  m_onu->setWidth(v);

  QRectF rect = m_mapScene->sceneRect();
  rect.setWidth(v);
  m_mapScene->setSceneRect(rect);

  if (m_mapItem != nullptr)
  {
    QPixmap mapPixmap(m_onu->pixmapForId(QStringLiteral("map"), m_onu->width(), m_onu->height()));
    m_mapItem->setPixmap(mapPixmap);
  }
}

void MainWindow::slotSkinHeightEdited(int v)
{
  if (m_onu == nullptr) return;
  m_onu->setHeight(v);

  QRectF rect = m_mapScene->sceneRect();
  rect.setHeight(v);
  m_mapScene->setSceneRect(rect);
  
  if (m_mapItem != nullptr)
  {
    QPixmap mapPixmap(m_onu->pixmapForId(QStringLiteral("map"), m_onu->width(), m_onu->height()));
    m_mapItem->setPixmap(mapPixmap);
  }
}

void MainWindow::slotSkinDescriptionEdited()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG) << m_skinDefWidget->descriptionTextEdit->toPlainText();
  m_onu->setDescription(m_skinDefWidget->descriptionTextEdit->toPlainText());
}

void MainWindow::slotSkinWidthDiffEdited()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  bool ok = false;
  int wd = m_spritesDefWidget->widthDiffLineEdit->text().toInt(&ok);
  SkinSpritesData::changeable().intData(QStringLiteral("width-between-flag-and-fighter"), wd);
}

void MainWindow::slotNewCountry()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  QString newCountryName = QInputDialog::getText(this, i18n("New country name"), i18n("Enter the name of the new country"));
  m_onu->createCountry(newCountryName);
  m_skinDefWidget->countrieslist->addItem(newCountryName);
}

void MainWindow::slotDeleteCountry()
{
  if (m_onu == nullptr) return;
  if (!m_skinDefWidget->countrieslist->currentItem()) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int answer = KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete country '%1'?", m_skinDefWidget->countrieslist->currentItem()->text()), i18n("Really delete country?"));
  if (answer == KMessageBox::Cancel)
  {
    return;
  }

  int row = m_skinDefWidget->countrieslist->row(m_skinDefWidget->countrieslist->currentItem());
  QListWidgetItem* item = m_skinDefWidget->countrieslist->takeItem(row);

  Country* country = m_onu->countryNamed(item->text());
  m_onu->deleteCountry(country);
  
  delete item;
}

void MainWindow::slotNewContinent()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  QString newContinentName = QInputDialog::getText(this, i18n("New continent name"), i18n("Enter the name of the new continent"));
  m_onu->createContinent(newContinentName);
  m_skinDefWidget->continentslist->addItem(newContinentName);
}

void MainWindow::slotDeleteContinent()
{
  if (m_onu == nullptr) return;
  if (!m_skinDefWidget->continentslist->currentItem()) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int answer = KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete continent '%1'?", m_skinDefWidget->continentslist->currentItem()->text()), i18n("Really delete continent?"));
  if (answer == KMessageBox::Cancel)
  {
    return;
  }
  
  int row = m_skinDefWidget->continentslist->row(m_skinDefWidget->continentslist->currentItem());
  QListWidgetItem* item = m_skinDefWidget->continentslist->takeItem(row);
  
  Continent* continent = m_onu->continentNamed(item->text());
  m_onu->deleteContinent(continent);
  
  delete item;
}

void MainWindow::slotNeighbours()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_skinDefWidget->countrieslist->currentItem() == nullptr)
  {
    return;
  }

  KsirkSkinEditorCountriesSelectionDialog* dialog = new KsirkSkinEditorCountriesSelectionDialog(this);
  dialog->countriesList->setSortingEnabled(true);
  dialog->countriesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  foreach (Country* country, m_onu->countries())
  {
    if (country->name() != m_skinDefWidget->countrieslist->currentItem()->text())
    {
      dialog->countriesList->addItem(country->name());
    }
  }
  foreach (Country* country, m_onu->countryNamed(m_skinDefWidget->countrieslist->currentItem()->text())->neighbours())
  {
    QList<QListWidgetItem *> list = dialog->countriesList->findItems(country->name(),Qt::MatchExactly);
    foreach (QListWidgetItem* item, list)
    {
      dialog->countriesList->setCurrentItem(item,QItemSelectionModel::Select);
    }
  }
  if (dialog->exec())
  {
    QList<QListWidgetItem *> list = dialog->countriesList->selectedItems();
    Country* country =  m_onu->countryNamed(m_skinDefWidget->countrieslist->currentItem()->text());
    QList<Country*> newNeighbours;
    m_countryDefWidget->neighbourslist->clear();
    foreach (QListWidgetItem* item, list)
    {
      Country* neighbour = m_onu->countryNamed(item->text());
      newNeighbours.push_back(neighbour);
      m_countryDefWidget->neighbourslist->addItem(neighbour->name());
    }
    country->neighbours() = newNeighbours;
  }
  delete dialog;
}

void MainWindow::slotSkinPartTabChanged(int index)
{
  qCDebug(KSIRKSKINEDITOR_LOG);

  switch (index)
  {
    case 0:
      m_spritesDefWidget->show();
      m_nationalityDefWidget->hide();
      m_countryDefWidget->hide();
      m_continentDefWidget->hide();
      m_goalDefWidget->hide();
      break;
    case 1:
      m_spritesDefWidget->hide();
      m_nationalityDefWidget->show();
      m_countryDefWidget->hide();
      m_continentDefWidget->hide();
      m_goalDefWidget->hide();
      break;
    case 2:
      m_spritesDefWidget->hide();
      m_nationalityDefWidget->hide();
      m_countryDefWidget->show();
      m_continentDefWidget->hide();
      m_goalDefWidget->hide();
      break;
    case 3:
      m_spritesDefWidget->hide();
      m_nationalityDefWidget->hide();
      m_countryDefWidget->hide();
      m_continentDefWidget->show();
      m_goalDefWidget->hide();
      break;
    case 4:
      m_spritesDefWidget->hide();
      m_nationalityDefWidget->hide();
      m_countryDefWidget->hide();
      m_continentDefWidget->hide();
      m_goalDefWidget->show();
      break;
    default:
      m_countryDefWidget->hide();
      m_nationalityDefWidget->hide();
      m_continentDefWidget->hide();
      m_goalDefWidget->hide();
      m_spritesDefWidget->hide();
  }
}

void MainWindow::slotContinentCountries()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_skinDefWidget->continentslist->currentItem() == nullptr)
  {
    return;
  }
  
  KsirkSkinEditorCountriesSelectionDialog* dialog = new KsirkSkinEditorCountriesSelectionDialog(this);
  dialog->countriesList->setSortingEnabled(true);
  dialog->countriesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  foreach (Country* country, m_onu->countries())
  {
    dialog->countriesList->addItem(country->name());
  }
  foreach (Country* country, m_onu->continentNamed(m_skinDefWidget->continentslist->currentItem()->text())->members())
  {
    QList<QListWidgetItem*> list = dialog->countriesList->findItems(country->name(),Qt::MatchExactly);
    foreach (QListWidgetItem* item, list)
    {
      dialog->countriesList->setCurrentItem(item,QItemSelectionModel::Select);
    }
  }
  if (dialog->exec())
  {
    QList<QListWidgetItem *> list = dialog->countriesList->selectedItems();
    qCDebug(KSIRKSKINEDITOR_LOG) << list.size();
    Continent* continent =  m_onu->continentNamed(m_skinDefWidget->continentslist->currentItem()->text());
    qCDebug(KSIRKSKINEDITOR_LOG) << (void*)continent;
    qCDebug(KSIRKSKINEDITOR_LOG) << continent->name();
    QList<Country*> newCountries;
    m_continentDefWidget->countrieslist->clear();
    foreach (QListWidgetItem* item, list)
    {
      Country* country = m_onu->countryNamed(item->text());
      qCDebug(KSIRKSKINEDITOR_LOG) << (void*)country;
      qCDebug(KSIRKSKINEDITOR_LOG) << country->name();
      newCountries.push_back(country);
      m_continentDefWidget->countrieslist->addItem(country->name());
    }
    qCDebug(KSIRKSKINEDITOR_LOG) << "set members";
    continent->members() = newCountries;
  }
  delete dialog;
}

void MainWindow::slotContinentBonusEdited(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_onu == nullptr || m_skinDefWidget->continentslist->currentItem() == nullptr) return;
  Continent* continent =  m_onu->continentNamed(m_skinDefWidget->continentslist->currentItem()->text());
  continent->setBonus(v);
  m_onu->setDirty();
}

void MainWindow::slotFlagWidthChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("flag-width"), v);
  updateSprites(Flag);
}

void MainWindow::slotFlagHeightChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("flag-height"), v);
  updateSprites(Flag);
}

void MainWindow::slotFlagFramesChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("flag-frames"), v);
  updateSprites(Flag);
}

void MainWindow::slotFlagVersionsChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("flag-versions"), v);
  updateSprites(Flag);
}


void MainWindow::slotInfantryWidthChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-width"), v);
  updateSprites(Infantry);
}

void MainWindow::slotInfantryHeightChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-height"), v);
  updateSprites(Infantry);
}

void MainWindow::slotInfantryFramesChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-frames"), v);
  updateSprites(Infantry);
}

void MainWindow::slotInfantryVersionsChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("infantry-versions"), v);
  updateSprites(Infantry);
}


void MainWindow::slotCavalryWidthChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-width"), v);
  updateSprites(Cavalry);
}

void MainWindow::slotCavalryHeightChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-height"), v);
  updateSprites(Cavalry);
}

void MainWindow::slotCavalryFramesChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-frames"), v);
  updateSprites(Cavalry);
}

void MainWindow::slotCavalryVersionsChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cavalry-versions"), v);
  updateSprites(Cavalry);
}


void MainWindow::slotCannonWidthChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-width"), v);
  updateSprites(Cannon);
}

void MainWindow::slotCannonHeightChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-height"), v);
  updateSprites(Cannon);
}

void MainWindow::slotCannonFramesChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-frames"), v);
  updateSprites(Cannon);
}

void MainWindow::slotCannonVersionsChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("cannon-versions"), v);
  updateSprites(Cannon);
}

void MainWindow::slotFiringWidthChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("firing-width"), v);
}

void MainWindow::slotFiringHeightChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("firing-height"), v);
}

void MainWindow::slotFiringFramesChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("firing-frames"), v);
}

void MainWindow::slotFiringVersionsChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("firing-versions"), v);
}

void MainWindow::slotExplodingWidthChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-width"), v);
}

void MainWindow::slotExplodingHeightChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-height"), v);
}

void MainWindow::slotExplodingFramesChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-frames"), v);
}

void MainWindow::slotExplodingVersionsChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  SkinSpritesData::changeable().intData(QStringLiteral("exploding-versions"), v);
}

void MainWindow::updateSprites(SpriteType type)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << type;
  QPixmap pix;
  QPixmap alphacopy;
  QPointF point;
  QString fileName;

  m_onu->updateIcon(type);
  switch (type)
  {
    case Flag:
      m_flagButton->setIcon(m_onu->flagIcon());
      break;
    case Infantry:
      m_infantryButton->setIcon(m_onu->infantryIcon());
      break;
    case Cavalry:
      m_cavalryButton->setIcon(m_onu->cavalryIcon());
      break;
    case Cannon:
      m_cannonButton->setIcon(m_onu->cannonIcon());
      break;
    default: ;
  }
  foreach (Country* country, m_onu->countries())
  {
    QGraphicsItem* item = m_onu->itemFor(country, type);
    if (item == nullptr)
    {
      qCCritical(KSIRKSKINEDITOR_LOG) << "item " << type << "not found for" << country->name();
      continue;
    }
    switch (type)
    {
      case Flag:
        pix = m_onu->flagIcon();
        ((PixmapItem*)item)->setPixmap(pix);
        break;
      case Infantry:
        pix = m_onu->infantryIcon();
        ((PixmapItem*)item)->setPixmap(pix);
        break;
      case Cavalry:
        pix = m_onu->cavalryIcon();
        ((PixmapItem*)item)->setPixmap(pix);
        break;
      case Cannon:
        pix = m_onu->cannonIcon();
        ((PixmapItem*)item)->setPixmap(pix);
        break;
      case Anchor:
        ((TextItem*)item)->setFont(m_onu->foregroundFont());
        break;
      default: ;
    }
  }
}

void MainWindow::slotGoalTypeWorldClicked()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_skinDefWidget->goalslist->currentItem() == nullptr)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::NoGoal);
}

void MainWindow::slotGoalTypePlayerClicked()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_skinDefWidget->goalslist->currentItem() == nullptr)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::GoalPlayer);
}

void MainWindow::slotGoalTypeCountriesClicked()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_skinDefWidget->goalslist->currentItem() == nullptr)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::Countries);
}

void MainWindow::slotGoalTypeContinentsClicked()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_skinDefWidget->goalslist->currentItem() == nullptr)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::Continents);
}

void MainWindow::slotGoalDescriptionEdited()
{
  if (m_onu == nullptr || m_skinDefWidget->goalslist->currentItem() == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setDescription(m_goalDefWidget->description->toPlainText());
}

void MainWindow::slotGoalNbCountriesChanged(int)
{
  if (m_onu == nullptr || m_skinDefWidget->goalslist->currentItem() == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setNbCountries(m_goalDefWidget->nbcountries->value());
}

void MainWindow::slotGoalNbArmiesByCountryChanged(int)
{
  if (m_onu == nullptr || m_skinDefWidget->goalslist->currentItem() == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setNbArmiesByCountry(m_goalDefWidget->armiesbycountry->value());
}

void MainWindow::slotGoalAnyContinentChanged(bool checked)
{
  if (m_onu == nullptr || m_skinDefWidget->goalslist->currentItem() == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];

  if (!checked) {
      goal->continents().removeAll(QString());
  } else {
      if (!goal->continents().contains(QString()))
      {
        goal->continents().push_back(QString());
      }
  }
  goal->setNbArmiesByCountry(m_goalDefWidget->armiesbycountry->value());
}

void MainWindow::slotGoalContinents()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (m_skinDefWidget->goalslist->currentItem() == nullptr)
  {
    return;
  }

  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];

  KsirkSkinEditorCountriesSelectionDialog* dialog = new KsirkSkinEditorCountriesSelectionDialog(this);
  dialog->countriesList->setSortingEnabled(true);
  dialog->countriesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  foreach (Continent* continent, m_onu->continents())
  {
    dialog->countriesList->addItem(continent->name());
  }
  foreach (const QString& continentName, goal->continents())
  {
    QList<QListWidgetItem*> list = dialog->countriesList->findItems(continentName,Qt::MatchExactly);
    foreach (QListWidgetItem* item, list)
    {
      dialog->countriesList->setCurrentItem(item,QItemSelectionModel::Select);
    }
  }
  if (dialog->exec())
  {
    QList<QListWidgetItem *> list = dialog->countriesList->selectedItems();
    qCDebug(KSIRKSKINEDITOR_LOG) << list.size();
    QList<QString> newContinents;
    m_goalDefWidget->continentslist->clear();
    foreach (QListWidgetItem* item, list)
    {
      Continent* continent = m_onu->continentNamed(item->text());
      qCDebug(KSIRKSKINEDITOR_LOG) << (void*)continent;
      qCDebug(KSIRKSKINEDITOR_LOG) << continent->name();
      newContinents.push_back(continent->name());
      m_goalDefWidget->continentslist->addItem(continent->name());
    }
    qCDebug(KSIRKSKINEDITOR_LOG) << "set members";
    goal->continents() = newContinents;
  }
  delete dialog;
}

void MainWindow::slotNewGoal()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  QString newGoalName = QStringLiteral("goal") + QString::number(m_skinDefWidget->goalslist->count()+1);
  m_onu->createGoal();
  m_skinDefWidget->goalslist->addItem(newGoalName);
}

void MainWindow::slotDeleteGoal()
{
  if (m_onu == nullptr) return;
  if (!m_skinDefWidget->goalslist->currentItem()) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int answer = KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete goal '%1'?", m_skinDefWidget->goalslist->currentItem()->text()), i18n("Really delete goal?"));
  if (answer == KMessageBox::Cancel)
  {
    return;
  }
  
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  qCDebug(KSIRKSKINEDITOR_LOG) << "row=" << row;
  QListWidgetItem* item = m_skinDefWidget->goalslist->takeItem(m_skinDefWidget->goalslist->count()-1);
  qCDebug(KSIRKSKINEDITOR_LOG) << "item=" << item;
  
  m_onu->deleteGoal(row);
  qCDebug(KSIRKSKINEDITOR_LOG) << "goal deleted";
  
  delete item;
}

void MainWindow::slotNationalityNameEdited()
{
  if (m_onu == nullptr || m_skinDefWidget->nationalitieslist->currentItem() ==nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG) << m_skinDefWidget->nationalitieslist->currentItem()->text();
  Nationality* nationality =  m_onu->nationalityNamed(m_skinDefWidget->nationalitieslist->currentItem()->text());
  nationality->setName(m_nationalityDefWidget->name->text());
  m_skinDefWidget->nationalitieslist->currentItem()->setText(m_nationalityDefWidget->name->text());
}

void MainWindow::slotNationalityLeaderNameEdited()
{
  if (m_onu == nullptr || m_skinDefWidget->nationalitieslist->currentItem() ==nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG) << m_skinDefWidget->nationalitieslist->currentItem()->text();
  qCDebug(KSIRKSKINEDITOR_LOG) << m_nationalityDefWidget->leader->text();
  Nationality* nationality =  m_onu->nationalityNamed(m_skinDefWidget->nationalitieslist->currentItem()->text());
  qCDebug(KSIRKSKINEDITOR_LOG) << nationality;
  nationality->setLeaderName(m_nationalityDefWidget->leader->text());
}

void MainWindow::slotNationalityFlagEdited(int)
{
  if (m_onu == nullptr || m_skinDefWidget->nationalitieslist->currentItem() ==nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG) << m_skinDefWidget->nationalitieslist->currentItem()->text();
  Nationality* nationality =  m_onu->nationalityNamed(m_skinDefWidget->nationalitieslist->currentItem()->text());
  QString previousFlagFileName = nationality->flagFileName();
  if (previousFlagFileName == m_nationalityDefWidget->flag->currentText())
  {
    return;
  }
  nationality->setFlagFileName(m_nationalityDefWidget->flag->currentText());
  m_onu->setDirty();
  
  if (previousFlagFileName.isEmpty() && !nationality->flagFileName().isEmpty())
  {
    updateSprites(Flag);
  }
}

void MainWindow::slotNewNationality()
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  QString newNationalityName = QInputDialog::getText(this, i18n("New nationality name"), i18n("Enter the name of the new nationality"));
  m_onu->createNationality(newNationalityName);
  m_skinDefWidget->nationalitieslist->addItem(newNationalityName);
}

void MainWindow::slotDeleteNationality()
{
  if (m_onu == nullptr) return;
  if (!m_skinDefWidget->nationalitieslist->currentItem()) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  int answer = KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete nationality '%1'?", m_skinDefWidget->nationalitieslist->currentItem()->text()), i18n("Really delete nationality?"));
  if (answer == KMessageBox::Cancel)
  {
    return;
  }
  
  int row = m_skinDefWidget->nationalitieslist->row(m_skinDefWidget->nationalitieslist->currentItem());
  QListWidgetItem* item = m_skinDefWidget->nationalitieslist->takeItem(row);
  
  Nationality* nationality = m_onu->nationalityNamed(item->text());
  m_onu->deleteNationality(nationality);
  
  delete item;
}

void MainWindow::slotFontSelected(const QFont &font)
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  m_onu->setFont(font);
  updateSprites(Anchor);
}

void MainWindow::slotFgSelected(const QColor& color)
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  m_onu->setFontFgColor(color);
  updateSprites(Anchor);
}

void MainWindow::slotBgColorSelected(const QColor& color)
{
  if (m_onu == nullptr) return;
  qCDebug(KSIRKSKINEDITOR_LOG);
  m_onu->setFontBgColor(color);
  updateSprites(Anchor);
}

void MainWindow::initSpritesButtonsWith(const Country* country)
{
  m_flagButton->setEnabled(m_onu->itemFor(country, Flag) == nullptr);
  m_infantryButton->setEnabled(m_onu->itemFor(country, Infantry) == nullptr);
  m_cavalryButton->setEnabled(m_onu->itemFor(country, Cavalry) == nullptr);
  m_cannonButton->setEnabled(m_onu->itemFor(country, Cannon) == nullptr);
  m_anchorButton->setEnabled(m_onu->itemFor(country, Anchor) == nullptr);
  m_centerButton->setEnabled(m_onu->itemFor(country, Center) == nullptr);
}

void MainWindow::slotFLagxValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointFlag(QPointF(v,currentCountry()->pointFlag().y()));
    if (m_onu->itemFor(currentCountry(), Flag) != nullptr)
      m_onu->itemFor(currentCountry(), Flag)->setPos(currentCountry()->pointFlag());
    m_onu->setDirty();
  }
}

void MainWindow::slotFLagyValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointFlag(QPointF(currentCountry()->pointFlag().x(),v));
    if (m_onu->itemFor(currentCountry(), Flag) != nullptr)
      m_onu->itemFor(currentCountry(), Flag)->setPos(currentCountry()->pointFlag());
    m_onu->setDirty();
  }
}

void MainWindow::slotCenterxValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->centralPoint(QPointF(v,currentCountry()->centralPoint().y()));
    if (m_onu->itemFor(currentCountry(), Center) != nullptr)
      m_onu->itemFor(currentCountry(), Center)->setPos(currentCountry()->centralPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotCenteryValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->centralPoint(QPointF(currentCountry()->centralPoint().x(),v));
    if (m_onu->itemFor(currentCountry(), Center) != nullptr)
      m_onu->itemFor(currentCountry(), Center)->setPos(currentCountry()->centralPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotAnchorxValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->anchorPoint(QPointF(v,currentCountry()->anchorPoint().y()));
    if (m_onu->itemFor(currentCountry(), Anchor) != nullptr)
      m_onu->itemFor(currentCountry(), Anchor)->setPos(currentCountry()->anchorPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotAnchoryValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->anchorPoint(QPointF(currentCountry()->anchorPoint().x(),v));
    if (m_onu->itemFor(currentCountry(), Anchor) != nullptr)
      m_onu->itemFor(currentCountry(), Anchor)->setPos(currentCountry()->anchorPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotInfantryxValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointInfantry(QPointF(v,currentCountry()->pointInfantry().y()));
    if (m_onu->itemFor(currentCountry(), Infantry) != nullptr)
      m_onu->itemFor(currentCountry(), Infantry)->setPos(currentCountry()->pointInfantry());
    m_onu->setDirty();
  }
}

void MainWindow::slotInfantryyValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointInfantry(QPointF(currentCountry()->pointInfantry().x(),v));
    if (m_onu->itemFor(currentCountry(), Infantry) != nullptr)
      m_onu->itemFor(currentCountry(), Infantry)->setPos(currentCountry()->pointInfantry());
    m_onu->setDirty();
  }
}

void MainWindow::slotCavalryxValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointCavalry(QPointF(v,currentCountry()->pointCavalry().y()));
    if (m_onu->itemFor(currentCountry(), Cavalry) != nullptr)
      m_onu->itemFor(currentCountry(), Cavalry)->setPos(currentCountry()->pointCavalry());
    m_onu->setDirty();
  }
}
void MainWindow::slotCavalryyValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointCavalry(QPointF(currentCountry()->pointCavalry().x(),v));
    if (m_onu->itemFor(currentCountry(), Cavalry) != nullptr)
      m_onu->itemFor(currentCountry(), Cavalry)->setPos(currentCountry()->pointCavalry());
    m_onu->setDirty();
  }
}

void MainWindow::slotCannonxValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointCannon(QPointF(v,currentCountry()->pointCannon().y()));
    if (m_onu->itemFor(currentCountry(), Cannon) != nullptr)
      m_onu->itemFor(currentCountry(), Cannon)->setPos(currentCountry()->pointCannon());
    m_onu->setDirty();
  }
}

void MainWindow::slotCannonyValueChanged(int v)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
  if (currentCountry() != nullptr)
  {
    currentCountry()->pointCannon(QPointF(currentCountry()->pointCannon().x(),v));
    if (m_onu->itemFor(currentCountry(), Cannon) != nullptr)
      m_onu->itemFor(currentCountry(), Cannon)->setPos(currentCountry()->pointCannon());
    m_onu->setDirty();
  }
}


} // closing namespace

#include "moc_mainwindow.cpp"
