/* This file is part of KsirK.
   Copyright (C) 2008 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

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
#include <QDockWidget>
#include <QTreeView>
#include <QPushButton>
#include <QGridLayout>
#include <QString>
#include <QVBoxLayout>
#include <QMovie>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QBitmap>
#include <QInputDialog>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardgameaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <phonon/mediaobject.h>
#include <KPushButton>
#include <kglobal.h>
#include <KStatusBar>
#include <KToolBar>
#include <KAction>
#include <KSvgRenderer>
#include <KDialog>
#include <KAboutApplicationDialog>
#include <KRecentFilesAction>


#include <assert.h>


namespace KsirkSkinEditor
{

MainWindow::MainWindow(QWidget* parent) :
  KXmlGuiWindow(parent),
  m_selectedSprite(None),
  m_onu(0),
  m_rfa(0),
  m_mapItem(0),
  m_updateHighlightPosition(false)
{
  kDebug() << "MainWindow constructor begin";
  KSirkSkinEditorWidget* mainWidget = new KSirkSkinEditorWidget(this);
  setCentralWidget(mainWidget);
  
  m_mapScene = new Scene(0,0,1024,768,this);
  connect(m_mapScene,SIGNAL(position(const QPointF&)),this,SLOT(slotPosition(const QPointF&)));
  connect(m_mapScene,SIGNAL(pressPosition(const QPointF&)),this,SLOT(slotPressPosition(const QPointF&)));
  connect(m_mapScene,SIGNAL(releasePosition(const QPointF&)),this,SLOT(slotReleasePosition(const QPointF&)));
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
  connect(m_flagButton,SIGNAL(clicked()),this,SLOT(slotFlagButtonClicked()));

  m_infantryButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_infantryButton);
  m_infantryButton->setCheckable(true);
  m_infantryButton->setEnabled(false);
  connect(m_infantryButton,SIGNAL(clicked()),this,SLOT(slotInfantryButtonClicked()));

  m_cavalryButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_cavalryButton);
  m_cavalryButton->setCheckable(true);
  m_cavalryButton->setEnabled(false);
  connect(m_cavalryButton,SIGNAL(clicked()),this,SLOT(slotCavalryButtonClicked()));

  m_cannonButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_cannonButton);
  m_cannonButton->setCheckable(true);
  m_cannonButton->setEnabled(false);
  connect(m_cannonButton,SIGNAL(clicked()),this,SLOT(slotCannonButtonClicked()));

  m_dirs = KGlobal::dirs();

  QString anchorFileName = m_dirs-> findResource("appdata", "cross.png");
  if (anchorFileName.isNull())
  {
    KMessageBox::error(0, i18n("Cannot load anchor icon<br>Program cannot continue"), i18n("Error !"));
    exit(2);
  }
  QPixmap anchorPix = QPixmap(anchorFileName);
  m_anchorButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_anchorButton->setIcon(anchorPix);
  layout->addWidget(m_anchorButton);
  m_anchorButton->setCheckable(true);
  m_anchorButton->setEnabled(false);
  connect(m_anchorButton,SIGNAL(clicked()),this,SLOT(slotAnchorButtonClicked()));
  
  QString centerFileName = m_dirs-> findResource("appdata", "target.png");
  if (centerFileName.isNull())
  {
    KMessageBox::error(0, i18n("Cannot load center icon<br>Program cannot continue"), i18n("Error !"));
    exit(2);
  }
  QPixmap centerPix = QPixmap(centerFileName);
  m_centerButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_centerButton->setIcon(centerPix);
  layout->addWidget(m_centerButton);
  m_centerButton->setCheckable(true);
  m_centerButton->setEnabled(false);
  connect(m_centerButton,SIGNAL(clicked()),this,SLOT(slotCenterButtonClicked()));

  
  //   m_accels.setEnabled(true);
  
  QString iconFileName = m_dirs-> findResource("appdata", "ksirkskineditor.png");
/*  if (iconFileName.isNull())
  {
      KMessageBox::error(0, i18n("Cannot load icon<br>Program cannot continue"), i18n("Error !"));
      exit(2);
  }*/
  QPixmap icon(iconFileName);

//    kDebug() << "Before initActions";
  initActions();

  m_skinDefWidget = new KSirkSkinDefinitionWidget(this);
  addDockWidget ( Qt::LeftDockWidgetArea, m_skinDefWidget);
  
  connect(m_skinDefWidget->fontrequester, SIGNAL(fontSelected(const QFont &)), this, SLOT(slotFontSelected(const QFont&)));
  connect(m_skinDefWidget->fgcolorbutton, SIGNAL(changed(const QColor&)), this, SLOT(slotFgSelected(const QColor&)));
  connect(m_skinDefWidget->bgcolorbutton, SIGNAL(changed(const QColor&)), this, SLOT(slotBgColorSelected(const QColor&)));

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
  connect(m_goalDefWidget->worldtype,SIGNAL(clicked()),this,SLOT(slotGoalTypeWorldClicked()));
  connect(m_goalDefWidget->playertype,SIGNAL(clicked()),this,SLOT(slotGoalTypePlayerClicked()));
  connect(m_goalDefWidget->countriestype,SIGNAL(clicked()),this,SLOT(slotGoalTypeCountriesClicked()));
  connect(m_goalDefWidget->continentstype,SIGNAL(clicked()),this,SLOT(slotGoalTypeContinentsClicked()));
  connect (m_goalDefWidget->description,SIGNAL(textChanged()), this, SLOT(slotGoalDescriptionEdited()));
  connect (m_goalDefWidget->nbcountries,SIGNAL(valueChanged(int)), this, SLOT(slotGoalNbCountriesChanged(int)));
  connect (m_goalDefWidget->armiesbycountry,SIGNAL(valueChanged(int)), this, SLOT(slotGoalNbArmiesByCountryChanged(int)));
  connect (m_goalDefWidget->anycontinent,SIGNAL(stateChanged(int)), this, SLOT(slotGoalAnyContinentChanged(int)));
  connect (m_goalDefWidget->selectcontinentsbutton, SIGNAL(clicked()), this, SLOT(slotGoalContinents()));
  connect(m_skinDefWidget->newGoalButton, SIGNAL(clicked()), this, SLOT(slotNewGoal()));
  connect(m_skinDefWidget->deleteGoalButton, SIGNAL(clicked()), this, SLOT(slotDeleteGoal()));
  
  m_nationalityDefWidget = new KsirkNationalityDefinitionWidget(this);
  addDockWidget ( Qt::RightDockWidgetArea, m_nationalityDefWidget);
  m_nationalityDefWidget->hide();
  
  m_spritesDefWidget = new KsirkSpritesDefinitionWidget(this);
  addDockWidget ( Qt::RightDockWidgetArea, m_spritesDefWidget);
  m_spritesDefWidget->hide();
  connect(m_spritesDefWidget->flagw, SIGNAL(valueChanged(int)), this, SLOT(slotFlagWidthChanged(int)));
  connect(m_spritesDefWidget->flagh, SIGNAL(valueChanged(int)), this, SLOT(slotFlagHeightChanged(int)));
  connect(m_spritesDefWidget->flagf, SIGNAL(valueChanged(int)), this, SLOT(slotFlagFramesChanged(int)));
  connect(m_spritesDefWidget->flagv, SIGNAL(valueChanged(int)), this, SLOT(slotFlagVersionsChanged(int)));

  connect(m_spritesDefWidget->infantryw, SIGNAL(valueChanged(int)), this, SLOT(slotInfantryWidthChanged(int)));
  connect(m_spritesDefWidget->infantryh, SIGNAL(valueChanged(int)), this, SLOT(slotInfantryHeightChanged(int)));
  connect(m_spritesDefWidget->infantryf, SIGNAL(valueChanged(int)), this, SLOT(slotInfantryFramesChanged(int)));
  connect(m_spritesDefWidget->infantryv, SIGNAL(valueChanged(int)), this, SLOT(slotInfantryVersionsChanged(int)));
  
  connect(m_spritesDefWidget->cavalryw, SIGNAL(valueChanged(int)), this, SLOT(slotCavalryWidthChanged(int)));
  connect(m_spritesDefWidget->cavalryh, SIGNAL(valueChanged(int)), this, SLOT(slotCavalryHeightChanged(int)));
  connect(m_spritesDefWidget->cavalryf, SIGNAL(valueChanged(int)), this, SLOT(slotCavalryFramesChanged(int)));
  connect(m_spritesDefWidget->cavalryv, SIGNAL(valueChanged(int)), this, SLOT(slotCavalryVersionsChanged(int)));
  
  connect(m_spritesDefWidget->cannonw, SIGNAL(valueChanged(int)), this, SLOT(slotCannonWidthChanged(int)));
  connect(m_spritesDefWidget->cannonh, SIGNAL(valueChanged(int)), this, SLOT(slotCannonHeightChanged(int)));
  connect(m_spritesDefWidget->cannonf, SIGNAL(valueChanged(int)), this, SLOT(slotCannonFramesChanged(int)));
  connect(m_spritesDefWidget->cannonv, SIGNAL(valueChanged(int)), this, SLOT(slotCannonVersionsChanged(int)));
  
  connect(m_spritesDefWidget->firingw, SIGNAL(valueChanged(int)), this, SLOT(slotFiringWidthChanged(int)));
  connect(m_spritesDefWidget->firingh, SIGNAL(valueChanged(int)), this, SLOT(slotFiringHeightChanged(int)));
  connect(m_spritesDefWidget->firingf, SIGNAL(valueChanged(int)), this, SLOT(slotFiringFramesChanged(int)));
  connect(m_spritesDefWidget->firingv, SIGNAL(valueChanged(int)), this, SLOT(slotFiringVersionsChanged(int)));
  
  connect(m_spritesDefWidget->explodingw, SIGNAL(valueChanged(int)), this, SLOT(slotExplodingWidthChanged(int)));
  connect(m_spritesDefWidget->explodingh, SIGNAL(valueChanged(int)), this, SLOT(slotExplodingHeightChanged(int)));
  connect(m_spritesDefWidget->explodingf, SIGNAL(valueChanged(int)), this, SLOT(slotExplodingFramesChanged(int)));
  connect(m_spritesDefWidget->explodingv, SIGNAL(valueChanged(int)), this, SLOT(slotExplodingVersionsChanged(int)));
  
  
  connect(m_skinDefWidget->ktabwidget, SIGNAL(currentChanged (int)), this, SLOT(slotSkinPartTabChanged(int)));
  m_skinDefWidget->ktabwidget-> setCurrentIndex(0);
  
  m_skinDefWidget->countrieslist->setSortingEnabled (true);
  connect(m_skinDefWidget->countrieslist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotCountrySelected(QListWidgetItem*)));

  connect(m_skinDefWidget->goalslist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotGoalSelected(QListWidgetItem*)));
  
  connect (m_countryDefWidget->neighboursbutton, SIGNAL(clicked()), this, SLOT(slotNeighbours()));
  connect (m_countryDefWidget->flagx, SIGNAL(valueChanged(int)), this, SLOT(slotFLagxValueChanged(int)));
  connect (m_countryDefWidget->flagy, SIGNAL(valueChanged(int)), this, SLOT(slotFLagyValueChanged(int)));
  connect (m_countryDefWidget->centerx, SIGNAL(valueChanged(int)), this, SLOT(slotCenterxValueChanged(int)));
  connect (m_countryDefWidget->centery, SIGNAL(valueChanged(int)), this, SLOT(slotCenteryValueChanged(int)));
  connect (m_countryDefWidget->anchorx, SIGNAL(valueChanged(int)), this, SLOT(slotAnchorxValueChanged(int)));
  connect (m_countryDefWidget->anchory, SIGNAL(valueChanged(int)), this, SLOT(slotAnchoryValueChanged(int)));
  connect (m_countryDefWidget->infantryx, SIGNAL(valueChanged(int)), this, SLOT(slotInfantryxValueChanged(int)));
  connect (m_countryDefWidget->infantryy, SIGNAL(valueChanged(int)), this, SLOT(slotInfantryyValueChanged(int)));
  connect (m_countryDefWidget->cavalryx, SIGNAL(valueChanged(int)), this, SLOT(slotCavalryxValueChanged(int)));
  connect (m_countryDefWidget->cavalryy, SIGNAL(valueChanged(int)), this, SLOT(slotCavalryyValueChanged(int)));
  connect (m_countryDefWidget->cannonx, SIGNAL(valueChanged(int)), this, SLOT(slotCannonxValueChanged(int)));
  connect (m_countryDefWidget->cannony, SIGNAL(valueChanged(int)), this, SLOT(slotCannonyValueChanged(int)));
  

  connect (m_skinDefWidget->skinNameLineEdit, SIGNAL(editingFinished()), this, SLOT(slotSkinNameEdited()));

  connect (m_skinDefWidget->widthLineEdit, SIGNAL(valueChanged(int)), this, SLOT(slotSkinWidthEdited(int)));
  connect (m_skinDefWidget->heightLineEdit, SIGNAL(valueChanged(int)), this, SLOT(slotSkinHeightEdited(int)));

  connect (m_skinDefWidget->descriptionTextEdit,SIGNAL(textChanged()), this, SLOT(slotSkinDescriptionEdited()));

  QIntValidator* widthDiffValidator = new QIntValidator(this);
  m_spritesDefWidget->widthDiffLineEdit->setValidator(widthDiffValidator);
  connect (m_spritesDefWidget->widthDiffLineEdit, SIGNAL(editingFinished()), this, SLOT(slotSkinWidthDiffEdited()));

  m_skinDefWidget->nationalitieslist->setSortingEnabled (true);
  connect(m_skinDefWidget->nationalitieslist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotNationalitySelected(QListWidgetItem*)));
  connect(m_skinDefWidget->newNationalityButton, SIGNAL(clicked()), this, SLOT(slotNewNationality()));
  connect(m_skinDefWidget->deleteNationalityButton, SIGNAL(clicked()), this, SLOT(slotDeleteNationality()));

  connect(m_nationalityDefWidget->name,SIGNAL(editingFinished()), this, SLOT(slotNationalityNameEdited()));
  connect(m_nationalityDefWidget->leader,SIGNAL(editingFinished()), this, SLOT(slotNationalityLeaderNameEdited()));
  connect(m_nationalityDefWidget->flag,SIGNAL(currentIndexChanged(int)), this, SLOT(slotNationalityFlagEdited(int)));
  
  
  connect(m_skinDefWidget->newCountryButton, SIGNAL(clicked()), this, SLOT(slotNewCountry()));
  connect(m_skinDefWidget->deleteCountryButton, SIGNAL(clicked()), this, SLOT(slotDeleteCountry()));    

  m_skinDefWidget->continentslist->setSortingEnabled (true);
  connect(m_skinDefWidget->continentslist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotContinentSelected(QListWidgetItem*)));

  connect(m_skinDefWidget->newContinentButton, SIGNAL(clicked()), this, SLOT(slotNewContinent()));
  connect(m_skinDefWidget->deleteContinentButton, SIGNAL(clicked()), this, SLOT(slotDeleteContinent()));
  
  connect (m_continentDefWidget->selectcountriesbutton, SIGNAL(clicked()), this, SLOT(slotContinentCountries()));
  
  connect (m_continentDefWidget->bonus, SIGNAL(valueChanged(int)), this, SLOT(slotContinentBonusEdited(int)));
  
  kDebug() << "Setting up GUI";
  setupGUI();
  
  //    kDebug() << "Before initStatusBar";
  initStatusBar();
  
  menuBar()-> show();
  
  setMouseTracking(true);
  
}

MainWindow::~MainWindow()
{
  kDebug();
  m_dirs = 0;
  KSharedConfig::Ptr config = KGlobal::config();
  if (m_rfa != 0)
  {
    kDebug() << "saving recent files";
    m_rfa->saveEntries(KGlobal::config()->group("ksirkskineditor"));
  }
}

void MainWindow::initActions()
{
  KAction *action;
  // standard game actions
  action = KStandardGameAction::load(this, SLOT(slotOpenSkin()), this);
  actionCollection()->addAction(action->objectName(), action);
  action->setToolTip(i18n("Open a saved skin..."));

  m_rfa = KStandardGameAction::loadRecent (this, SLOT(slotURLSelected(const KUrl&)), this);
  actionCollection()->addAction(m_rfa->objectName(), m_rfa);
  m_rfa->setText(i18n("Load &Recent"));
  m_rfa->setToolTip(i18n("Open a recently saved skin..."));

  KSharedConfig::Ptr config = KGlobal::config();
  kDebug() << "loading recent files";
  m_rfa->loadEntries(KGlobal::config()->group("ksirkskineditor"));
  
  m_saveGameAction = KStandardGameAction::save(this, SLOT(slotSaveSkin()), this);
  actionCollection()->addAction(m_saveGameAction->objectName(), m_saveGameAction);
  m_saveGameAction->setToolTip(i18n("Save the current skin"));

  action = KStandardGameAction::quit(this, SLOT(close()), this);
  actionCollection()->addAction(action->objectName(), action);

//   action = KStandardGameAction::gameNew(this, SLOT(slotNewGame()), this);
//   actionCollection()->addAction(action->objectName(), action);

//   action = KStandardAction::zoomIn(this, SLOT(slotZoomIn()), this);
//   actionCollection()->addAction(action->objectName(), action);

//   action = KStandardAction::zoomOut(this, SLOT(slotZoomOut()), this);
//   actionCollection()->addAction(action->objectName(), action);

  KStandardAction::preferences( this, SLOT( optionsConfigure() ), actionCollection() );

}

void MainWindow::initStatusBar()
{
  statusBar()-> setSizeGripEnabled(true);
}

void MainWindow::slotOpenSkin(const QString& dir)
{
  kDebug() << dir;
  QString skinDir = dir;
  if (dir.isNull())
  {
    skinDir = KFileDialog::getExistingDirectory(
    KUrl(), this, i18n("Choose the root folder of the skin to open"));
  }
  if (skinDir.isEmpty())
  {
    return;
  }

  if (m_rfa != 0)
  {
    kDebug() << "Adding" << skinDir << "to recent files";
    m_rfa->addUrl(KUrl::fromPath(skinDir));
  }
  
  m_mapScene->clear();
  m_skinDefWidget->countrieslist->clear();
  m_skinDefWidget->continentslist->clear();
  m_skinDefWidget->goalslist->clear();
  
  if (m_onu != 0)
  {
    delete m_onu;
  }
  m_onu = new ONU(skinDir);

  m_skinDefWidget->fontrequester->setFont(m_onu->foregroundFont());
  m_skinDefWidget->fgcolorbutton->setColor(m_onu->foregroundColor());
  m_skinDefWidget->bgcolorbutton->setColor(m_onu->backgroundColor());
  
  m_skinDefWidget->skinNameLineEdit->setText(m_onu->name());
  m_skinDefWidget->widthLineEdit->setValue(m_onu->width());
  m_skinDefWidget->heightLineEdit->setValue(m_onu->height());
  m_skinDefWidget->descriptionTextEdit->setText(m_onu->description());
  m_spritesDefWidget->widthDiffLineEdit->setText(QString::number(SkinSpritesData::changeable().intData("width-between-flag-and-fighter")));

  m_spritesDefWidget->flagw->setValue(SkinSpritesData::changeable().intData("flag-width"));
  m_spritesDefWidget->flagh->setValue(SkinSpritesData::changeable().intData("flag-height"));
  m_spritesDefWidget->flagf->setValue(SkinSpritesData::changeable().intData("flag-frames"));
  m_spritesDefWidget->flagv->setValue(SkinSpritesData::changeable().intData("flag-versions"));
  
  m_spritesDefWidget->infantryw->setValue(SkinSpritesData::changeable().intData("infantry-width"));
  m_spritesDefWidget->infantryh->setValue(SkinSpritesData::changeable().intData("infantry-height"));
  m_spritesDefWidget->infantryf->setValue(SkinSpritesData::changeable().intData("infantry-frames"));
  m_spritesDefWidget->infantryv->setValue(SkinSpritesData::changeable().intData("infantry-versions"));
  
  m_spritesDefWidget->cavalryw->setValue(SkinSpritesData::changeable().intData("cavalry-width"));
  m_spritesDefWidget->cavalryh->setValue(SkinSpritesData::changeable().intData("cavalry-height"));
  m_spritesDefWidget->cavalryf->setValue(SkinSpritesData::changeable().intData("cavalry-frames"));
  m_spritesDefWidget->cavalryv->setValue(SkinSpritesData::changeable().intData("cavalry-versions"));
  
  m_spritesDefWidget->cannonw->setValue(SkinSpritesData::changeable().intData("cannon-width"));
  m_spritesDefWidget->cannonh->setValue(SkinSpritesData::changeable().intData("cannon-height"));
  m_spritesDefWidget->cannonf->setValue(SkinSpritesData::changeable().intData("cannon-frames"));
  m_spritesDefWidget->cannonv->setValue(SkinSpritesData::changeable().intData("cannon-versions"));
  
  m_spritesDefWidget->firingw->setValue(SkinSpritesData::changeable().intData("firing-width"));
  m_spritesDefWidget->firingh->setValue(SkinSpritesData::changeable().intData("firing-height"));
  m_spritesDefWidget->firingf->setValue(SkinSpritesData::changeable().intData("firing-frames"));
  m_spritesDefWidget->firingv->setValue(SkinSpritesData::changeable().intData("firing-versions"));
  
  m_spritesDefWidget->explodingw->setValue(SkinSpritesData::changeable().intData("exploding-width"));
  m_spritesDefWidget->explodingh->setValue(SkinSpritesData::changeable().intData("exploding-height"));
  m_spritesDefWidget->explodingf->setValue(SkinSpritesData::changeable().intData("exploding-frames"));
  m_spritesDefWidget->explodingv->setValue(SkinSpritesData::changeable().intData("exploding-versions"));

  m_nationalityDefWidget->flag->clear();
  foreach(const QString& key, m_onu->poolIds())
  {
    int flagWidth = SkinSpritesData::changeable().intData("flag-width");
    int flagHeight = SkinSpritesData::changeable().intData("flag-height");
    int flagFrames = SkinSpritesData::changeable().intData("flag-frames");
    int flagVersions = SkinSpritesData::changeable().intData("flag-versions");
    QPixmap flagIcon = m_onu->pixmapForId(key,flagWidth*flagFrames,flagHeight*flagVersions).copy(0,0,flagWidth,flagHeight);
    m_nationalityDefWidget->flag->insertItem(m_nationalityDefWidget->flag->count(),QIcon(flagIcon),key);
  }

  
  QPixmap mapPixmap(m_onu->pixmapForId("map", m_onu->width(), m_onu->height()));
  m_mapItem = m_mapScene->addPixmap(mapPixmap);
  
  m_flagButton->setIcon(m_onu->flagIcon());
  //   m_flagButton->setIconSize(QSize(flagWidth,flagHeight));
  
  m_infantryButton->setIcon(m_onu->infantryIcon());
  //   m_infantryButton->setIconSize(QSize(infantryWidth,infantryHeight));
  
  m_cavalryButton->setIcon(m_onu->cavalryIcon());
  //   m_cavalryButton->setIconSize(QSize(cavalryWidth,cavalryHeight));
  
  m_cannonButton->setIcon(m_onu->cannonIcon());
  //   m_cannonButton->setIconSize(QSize(cannonWidth,cannonHeight));
  
  kDebug() << "Adding nationalities items";
  foreach (Nationality* nationality, m_onu->nationalities())
  {
    kDebug() << "Adding "<<nationality->name()<<" items";
    m_skinDefWidget->nationalitieslist->addItem(nationality->name());
  }
  
  kDebug() << "Adding countries items";
  foreach (Country* country, m_onu->countries())
  {
    kDebug() << "Adding "<<country->name()<<" items";
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

  kDebug() << "Adding continents items";
  foreach (Continent* continent, m_onu->continents())
  {
    kDebug() << "Adding "<<continent<<" items";
    kDebug() << "Adding "<<continent->name()<<" items";
    m_skinDefWidget->continentslist->addItem(continent->name());
  }

  kDebug() << "Adding goals items";
  for (int i = 1; i <= m_onu->goals().size(); i++)
  {
    kDebug() << "Adding goal"<<i<<" items";
    m_skinDefWidget->goalslist->addItem("goal" + QString::number(i));
  }
}

void MainWindow::slotSaveSkin()
{
  kDebug();
  m_onu->saveConfig();
}

/**
  * Reimplementation of the inherited function called when a window close event arise
  */
bool MainWindow::queryClose()
{
  kDebug();
  // TODO : Test si jeu en cours
  if (m_onu && m_onu->dirty())
  {
    switch (KMessageBox::warningYesNoCancel(this,i18n("There is unsaved changes. What do you want to do ?"),
      i18n("Exit Anyway ?"),
                                             KGuiItem(i18n("Quit without saving")),
                                             KGuiItem(i18n("Save then quit")),
                                             KGuiItem(i18n("Don't quit"))))
    {
      case KMessageBox::Yes:
        return true;
        break;
      case KMessageBox::No:
        m_onu->saveConfig();
        return true;
        break;
      default:
        return false;
    }
  }
  return true;
}

void MainWindow::optionsConfigure()
{
  //An instance of your dialog could be already created and could be cached, 
  //in which case you want to display the cached dialog instead of creating 
  //another one 
  if ( KsirkSkinEditorConfigurationDialog::showDialog( "settings" ) ) 
    return;
 
  //KConfigDialog didn't find an instance of this dialog, so lets create it : 
  KsirkSkinEditorConfigurationDialog* dialog = new KsirkSkinEditorConfigurationDialog(
      this, "settings",
      KsirkSkinEditorSettings::self() ); 

  dialog->show();
}

// void MainWindow::slotZoomIn()
// {
//   kDebug();
// }

// void MainWindow::slotZoomOut()
// {
//   kDebug();
// }

void MainWindow::slotShowAboutApplication()
{
  KAboutApplicationDialog dialog(KGlobal::mainComponent().aboutData(), this);
  dialog.exec();
}

bool MainWindow::queryExit()
{
//   kDebug() << "Writing skin m_config: " << m_automaton->skin();
  KGlobal::config()->sync();
  return true;
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
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
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
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
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
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
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
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
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
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
  {
    QString anchorFileName = m_dirs-> findResource("appdata", "cross.png");
    if (anchorFileName.isNull())
    {
      KMessageBox::error(0, i18n("Cannot load anchor icon<br>Program cannot continue"), i18n("Error !"));
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
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
  {
    QString centerFileName = m_dirs-> findResource("appdata", "target.png");
    if (centerFileName.isNull())
    {
      KMessageBox::error(0, i18n("Cannot load center icon<br>Program cannot continue"), i18n("Error !"));
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
  QString message("");
  QTextStream ts( &message );
  ts << point.x() << " x " << point.y();
  statusBar()->showMessage(message);
//   kDebug() << "selected sprite:" << m_selectedSprite;
  if (currentCountry() != 0 && m_selectedSprite == Anchor
    && m_onu->itemFor(currentCountry(), m_selectedSprite) != 0 && m_updateHighlightPosition
    && currentCountry()->highlighting() != 0)
  {
    kDebug() << point << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    QGraphicsItem* anchorItem = m_onu->itemFor(currentCountry(), Anchor);
    kDebug() << "anchorItem=" << anchorItem;
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
  kDebug() << clickedPoint << (void*)currentCountry() << m_selectedSprite;
  QPixmap pix;
  QPixmap alphacopy;
  QString fileName;
  QPointF point = clickedPoint;
  m_updateHighlightPosition = true;
  if (currentCountry() == 0
    || m_onu->itemFor(currentCountry(), m_selectedSprite) != 0)
  {
    kDebug() << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    if (currentCountry() != 0)
    {
      currentCountry()->clearHighlighting();
    }
    return;
  }
  PixmapItem* item = new PixmapItem();
  item->setZValue(10);
  connect(item, SIGNAL(pressed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPressed(QGraphicsItem*, const QPointF&)));
  connect(item, SIGNAL(placed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPlaced(QGraphicsItem*, const QPointF&)));
  
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
      kDebug() << "Adding cannon";
      pix = m_onu->cannonIcon();
//       pix = pix.scaled(32,32);
      m_countryDefWidget->cannonx->setValue(point.x());
      m_countryDefWidget->cannony->setValue(point.y());
      currentCountry()->pointCannon(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Cannon));
      break;
    case Anchor:
      kDebug() << "Adding anchor";
      fileName = m_dirs-> findResource("appdata", "cross.png");
      if (fileName.isNull())
      {
        KMessageBox::error(0, i18n("Cannot load anchor icon<br>Program cannot continue"), i18n("Error !"));
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
      kDebug() << "Adding center";
      fileName = m_dirs-> findResource("appdata", "target.png");
      if (fileName.isNull())
      {
        KMessageBox::error(0, i18n("Cannot load center icon<br>Program cannot continue"), i18n("Error !"));
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
  if (item != 0)
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
  kDebug() << point;
  m_updateHighlightPosition = false;

  kDebug() << "selected sprite:" << m_selectedSprite;
  if (currentCountry() != 0 && m_selectedSprite == Anchor
    && m_onu->itemFor(currentCountry(), m_selectedSprite) != 0 && m_updateHighlightPosition
    && currentCountry()->highlighting() != 0)
  {
    kDebug() << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    currentCountry()->anchorPoint(point);
    currentCountry()->highlighting()->setPos((currentCountry()->anchorPoint().x()-currentCountry()->highlighting()->boundingRect().width()/2),(currentCountry()->anchorPoint().y()-currentCountry()->highlighting()->boundingRect().height()/2));
    
    return;
  }
}

void MainWindow::slotNationalitySelected(QListWidgetItem* item)
{
  kDebug();
  Nationality* nationality = m_onu->nationalityNamed(item->text());
  if (nationality != 0)
  {
    initNationalityWidgetWith(nationality);
  }
}

void MainWindow::initNationalityWidgetWith(Nationality* nationality)
{
  kDebug();
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
  kDebug();
  Country* country = m_onu->countryNamed(item->text());
  if (country != 0)
  {
    m_countryDefWidget->initWith(country);
    initSpritesButtonsWith(country);
  }
}

void MainWindow::slotContinentSelected(QListWidgetItem* item)
{
  kDebug();
  Continent* continent = m_onu->continentNamed(item->text());
  if (continent != 0)
  {
    initContinentWidgetWith(continent);
  }
}

void MainWindow::initContinentWidgetWith(Continent* continent)
{
  kDebug();
  m_continentDefWidget->bonus->setValue(continent->bonus());

  m_continentDefWidget->countrieslist->clear();
  foreach(Country* country, continent->members())
  {
    m_continentDefWidget->countrieslist->addItem(country->name());
  }
}

void MainWindow::slotGoalSelected(QListWidgetItem* item)
{
  kDebug();
  int row = m_skinDefWidget->goalslist->row(item);
  Goal* goal = m_onu->goals()[row];
  if (goal != 0)
  {
    initGoalWidgetWith(goal);
  }
}

void MainWindow::initGoalWidgetWith(Goal* goal)
{
  kDebug();
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
    kDebug() << "continent" << id;
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
  if (m_skinDefWidget->countrieslist->currentItem() == 0)
    return 0;
//   kDebug() << m_skinDefWidget->countrieslist->currentItem()->text();
  return m_onu->countryNamed(m_skinDefWidget->countrieslist->currentItem()->text());
}

void MainWindow::slotItemPlaced(QGraphicsItem* item, const QPointF&)
{
  kDebug();
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
  kDebug();
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
        if (item == 0)
        {
          kError() << "item " << Anchor << "not found for" << country->name();
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
  kDebug() << country->name() << type;
  QPixmap pix;
  QPixmap alphacopy;
  QPointF point;
  QString fileName;
  if (country == 0
    || m_onu->itemFor(country, type) != 0)
  {
    kDebug() << (void*)country << (void*)m_onu->itemFor(country, type);
    return;
  }
  QGraphicsItem* item = 0;
                    
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
      kDebug() << "Adding cannon";
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
      kDebug() << "Adding anchor";
      fileName = m_dirs-> findResource("appdata", "cross.png");
      if (fileName.isNull())
      {
        KMessageBox::error(0, i18n("Cannot load anchor icon<br>Program cannot continue"), i18n("Error !"));
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
      kDebug() << "Adding center";
      item = new TextItem();
      item->setZValue(2);
      ((TextItem*)item)->setFont(m_onu->foregroundFont());
      ((TextItem*)item)->setPlainText(country->name());
      point = country->centralPoint() - QPointF(item->boundingRect().width()/2, item->boundingRect().height()/2);
      kDebug() << "Adding center" << country->centralPoint() << point;
      m_countryDefWidget->centerx->setValue(point.x());
      m_countryDefWidget->centery->setValue(point.y());
      m_onu->itemsMap().insert(item,qMakePair(country,Center));
      break;
    default: ;
  }
  if (item != 0 && !point.isNull())
  {
    item->setPos(point);
    if (dynamic_cast<PixmapItem*>(item) != 0)
    {
      connect(dynamic_cast<PixmapItem*>(item),SIGNAL(pressed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPressed(QGraphicsItem*, const QPointF&)));
      connect(dynamic_cast<PixmapItem*>(item),SIGNAL(placed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPlaced(QGraphicsItem*, const QPointF&)));
    }
    else if (dynamic_cast<TextItem*>(item) != 0)
    {
      connect(dynamic_cast<TextItem*>(item),SIGNAL(pressed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPressed(QGraphicsItem*, const QPointF&)));
      connect(dynamic_cast<TextItem*>(item), SIGNAL(placed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPlaced(QGraphicsItem*, const QPointF&)));
    }
    m_mapScene->addItem(item);
    item->setFlag(QGraphicsItem::ItemIsMovable, true);
    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
  }
}

void MainWindow::slotURLSelected(const KUrl& url)
{
  kDebug();
  if ( url.isLocalFile() )
  {
    QString path = url.path();
    slotOpenSkin(path);
  }
}

void MainWindow::slotSkinNameEdited()
{
  if (m_onu == 0) return;
  m_onu->setName(m_skinDefWidget->skinNameLineEdit->text());
}

void MainWindow::slotSkinWidthEdited(int v)
{
  if (m_onu == 0) return;
  m_onu->setWidth(v);

  if (m_mapItem != 0)
  {
    QPixmap mapPixmap(m_onu->pixmapForId("map", m_onu->width(), m_onu->height()));
    m_mapItem->setPixmap(mapPixmap);
  }
}

void MainWindow::slotSkinHeightEdited(int v)
{
  if (m_onu == 0) return;
  m_onu->setHeight(v);

  if (m_mapItem != 0)
  {
    QPixmap mapPixmap(m_onu->pixmapForId("map", m_onu->width(), m_onu->height()));
    m_mapItem->setPixmap(mapPixmap);
  }
}

void MainWindow::slotSkinDescriptionEdited()
{
  if (m_onu == 0) return;
  kDebug() << m_skinDefWidget->descriptionTextEdit->toPlainText();
  m_onu->setDescription(m_skinDefWidget->descriptionTextEdit->toPlainText());
}

void MainWindow::slotSkinWidthDiffEdited()
{
  if (m_onu == 0) return;
  kDebug();
  bool ok = false;
  int wd = m_spritesDefWidget->widthDiffLineEdit->text().toInt(&ok);
  SkinSpritesData::changeable().intData("width-between-flag-and-fighter", wd);
}

void MainWindow::slotNewCountry()
{
  if (m_onu == 0) return;
  kDebug();
  QString newCountryName = QInputDialog::getText(this, i18n("New country name"), i18n("Enter the name of the new country"));
  m_onu->createCountry(newCountryName);
  m_skinDefWidget->countrieslist->addItem(newCountryName);
}

void MainWindow::slotDeleteCountry()
{
  if (m_onu == 0) return;
  kDebug();
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
  if (m_onu == 0) return;
  kDebug();
  QString newContinentName = QInputDialog::getText(this, i18n("New continent name"), i18n("Enter the name of the new continent"));
  m_onu->createContinent(newContinentName);
  m_skinDefWidget->continentslist->addItem(newContinentName);
}

void MainWindow::slotDeleteContinent()
{
  if (m_onu == 0) return;
  kDebug();
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
  if (m_onu == 0) return;
  kDebug();
  if (m_skinDefWidget->countrieslist->currentItem() == 0)
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
  kDebug();

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
  if (m_onu == 0) return;
  kDebug();
  if (m_skinDefWidget->continentslist->currentItem() == 0)
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
    kDebug() << list.size();
    Continent* continent =  m_onu->continentNamed(m_skinDefWidget->continentslist->currentItem()->text());
    kDebug() << (void*)continent;
    kDebug() << continent->name();
    QList<Country*> newCountries;
    m_continentDefWidget->countrieslist->clear();
    foreach (QListWidgetItem* item, list)
    {
      Country* country = m_onu->countryNamed(item->text());
      kDebug() << (void*)country;
      kDebug() << country->name();
      newCountries.push_back(country);
      m_continentDefWidget->countrieslist->addItem(country->name());
    }
    kDebug() << "set members";
    continent->members() = newCountries;
  }
  delete dialog;
}

void MainWindow::slotContinentBonusEdited(int v)
{
  kDebug();
  if (m_onu == 0 || m_skinDefWidget->continentslist->currentItem() == 0) return;
  Continent* continent =  m_onu->continentNamed(m_skinDefWidget->continentslist->currentItem()->text());
  continent->setBonus(v);
  m_onu->setDirty();
}

void MainWindow::slotFlagWidthChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("flag-width", v);
  updateSprites(Flag);
}

void MainWindow::slotFlagHeightChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("flag-height", v);
  updateSprites(Flag);
}

void MainWindow::slotFlagFramesChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("flag-frames", v);
  updateSprites(Flag);
}

void MainWindow::slotFlagVersionsChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("flag-versions", v);
  updateSprites(Flag);
}


void MainWindow::slotInfantryWidthChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("infantry-width", v);
  updateSprites(Infantry);
}

void MainWindow::slotInfantryHeightChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("infantry-height", v);
  updateSprites(Infantry);
}

void MainWindow::slotInfantryFramesChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("infantry-frames", v);
  updateSprites(Infantry);
}

void MainWindow::slotInfantryVersionsChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("infantry-versions", v);
  updateSprites(Infantry);
}


void MainWindow::slotCavalryWidthChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cavalry-width", v);
  updateSprites(Cavalry);
}

void MainWindow::slotCavalryHeightChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cavalry-height", v);
  updateSprites(Cavalry);
}

void MainWindow::slotCavalryFramesChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cavalry-frames", v);
  updateSprites(Cavalry);
}

void MainWindow::slotCavalryVersionsChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cavalry-versions", v);
  updateSprites(Cavalry);
}


void MainWindow::slotCannonWidthChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cannon-width", v);
  updateSprites(Cannon);
}

void MainWindow::slotCannonHeightChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cannon-height", v);
  updateSprites(Cannon);
}

void MainWindow::slotCannonFramesChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cannon-frames", v);
  updateSprites(Cannon);
}

void MainWindow::slotCannonVersionsChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("cannon-versions", v);
  updateSprites(Cannon);
}

void MainWindow::slotFiringWidthChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("firing-width", v);
}

void MainWindow::slotFiringHeightChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("firing-height", v);
}

void MainWindow::slotFiringFramesChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("firing-frames", v);
}

void MainWindow::slotFiringVersionsChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("firing-versions", v);
}

void MainWindow::slotExplodingWidthChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("exploding-width", v);
}

void MainWindow::slotExplodingHeightChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("exploding-height", v);
}

void MainWindow::slotExplodingFramesChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("exploding-frames", v);
}

void MainWindow::slotExplodingVersionsChanged(int v)
{
  kDebug();
  SkinSpritesData::changeable().intData("exploding-versions", v);
}

void MainWindow::updateSprites(SpriteType type)
{
  kDebug() << type;
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
    if (item == 0)
    {
      kError() << "item " << type << "not found for" << country->name();
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
  if (m_onu == 0) return;
  kDebug();
  if (m_skinDefWidget->goalslist->currentItem() == 0)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::NoGoal);
}

void MainWindow::slotGoalTypePlayerClicked()
{
  if (m_onu == 0) return;
  kDebug();
  if (m_skinDefWidget->goalslist->currentItem() == 0)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::GoalPlayer);
}

void MainWindow::slotGoalTypeCountriesClicked()
{
  if (m_onu == 0) return;
  kDebug();
  if (m_skinDefWidget->goalslist->currentItem() == 0)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::Countries);
}

void MainWindow::slotGoalTypeContinentsClicked()
{
  if (m_onu == 0) return;
  kDebug();
  if (m_skinDefWidget->goalslist->currentItem() == 0)
  {
    return;
  }
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setType(Goal::Continents);
}

void MainWindow::slotGoalDescriptionEdited()
{
  if (m_onu == 0 || m_skinDefWidget->goalslist->currentItem() == 0) return;
  kDebug();
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setDescription(m_goalDefWidget->description->toPlainText());
}

void MainWindow::slotGoalNbCountriesChanged(int)
{
  if (m_onu == 0 || m_skinDefWidget->goalslist->currentItem() == 0) return;
  kDebug();
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setNbCountries(m_goalDefWidget->nbcountries->value());
}

void MainWindow::slotGoalNbArmiesByCountryChanged(int)
{
  if (m_onu == 0 || m_skinDefWidget->goalslist->currentItem() == 0) return;
  kDebug();
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  goal->setNbArmiesByCountry(m_goalDefWidget->armiesbycountry->value());
}

void MainWindow::slotGoalAnyContinentChanged(int state)
{
  if (m_onu == 0 || m_skinDefWidget->goalslist->currentItem() == 0) return;
  kDebug();
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  Goal* goal = m_onu->goals()[row];
  switch (state)
  {
    case Qt::Unchecked:
      goal->continents().removeAll(QString());
      break;
    case Qt::Checked:
      if (!goal->continents().contains(QString()))
      {
        goal->continents().push_back(QString());
      }
      break;
    default:;
  }
  goal->setNbArmiesByCountry(m_goalDefWidget->armiesbycountry->value());
}

void MainWindow::slotGoalContinents()
{
  if (m_onu == 0) return;
  kDebug();
  if (m_skinDefWidget->goalslist->currentItem() == 0)
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
    kDebug() << list.size();
    QList<QString> newContinents;
    m_goalDefWidget->continentslist->clear();
    foreach (QListWidgetItem* item, list)
    {
      Continent* continent = m_onu->continentNamed(item->text());
      kDebug() << (void*)continent;
      kDebug() << continent->name();
      newContinents.push_back(continent->name());
      m_goalDefWidget->continentslist->addItem(continent->name());
    }
    kDebug() << "set members";
    goal->continents() = newContinents;
  }
  delete dialog;
}

void MainWindow::slotNewGoal()
{
  if (m_onu == 0) return;
  kDebug();
  QString newGoalName = QString("goal") + QString::number(m_skinDefWidget->goalslist->count()+1);
  m_onu->createGoal();
  m_skinDefWidget->goalslist->addItem(newGoalName);
}

void MainWindow::slotDeleteGoal()
{
  if (m_onu == 0) return;
  kDebug();
  int answer = KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete goal '%1'?", m_skinDefWidget->goalslist->currentItem()->text()), i18n("Really delete goal?"));
  if (answer == KMessageBox::Cancel)
  {
    return;
  }
  
  int row = m_skinDefWidget->goalslist->row(m_skinDefWidget->goalslist->currentItem());
  kDebug() << "row=" << row;
  QListWidgetItem* item = m_skinDefWidget->goalslist->takeItem(m_skinDefWidget->goalslist->count()-1);
  kDebug() << "item=" << item;
  
  m_onu->deleteGoal(row);
  kDebug() << "goal deleted";
  
  delete item;
}

void MainWindow::slotNationalityNameEdited()
{
  if (m_onu == 0 || m_skinDefWidget->nationalitieslist->currentItem() ==0) return;
  kDebug() << m_skinDefWidget->nationalitieslist->currentItem()->text();
  Nationality* nationality =  m_onu->nationalityNamed(m_skinDefWidget->nationalitieslist->currentItem()->text());
  nationality->setName(m_nationalityDefWidget->name->text());
  m_skinDefWidget->nationalitieslist->currentItem()->setText(m_nationalityDefWidget->name->text());
}

void MainWindow::slotNationalityLeaderNameEdited()
{
  if (m_onu == 0 || m_skinDefWidget->nationalitieslist->currentItem() ==0) return;
  kDebug() << m_skinDefWidget->nationalitieslist->currentItem()->text();
  kDebug() << m_nationalityDefWidget->leader->text();
  Nationality* nationality =  m_onu->nationalityNamed(m_skinDefWidget->nationalitieslist->currentItem()->text());
  kDebug() << nationality;
  nationality->setLeaderName(m_nationalityDefWidget->leader->text());
}

void MainWindow::slotNationalityFlagEdited(int)
{
  if (m_onu == 0 || m_skinDefWidget->nationalitieslist->currentItem() ==0) return;
  kDebug() << m_skinDefWidget->nationalitieslist->currentItem()->text();
  Nationality* nationality =  m_onu->nationalityNamed(m_skinDefWidget->nationalitieslist->currentItem()->text());
  QString previousFlagFileName = nationality->flagFileName();
  if (previousFlagFileName == m_nationalityDefWidget->flag->currentText())
  {
    return;
  }
  nationality->setFlagFileName(m_nationalityDefWidget->flag->currentText());
  m_onu->setDirty();
  
  if (previousFlagFileName == "" && nationality->flagFileName() != "")
  {
    updateSprites(Flag);
  }
}

void MainWindow::slotNewNationality()
{
  if (m_onu == 0) return;
  kDebug();
  QString newNationalityName = QInputDialog::getText(this, i18n("New nationality name"), i18n("Enter the name of the new nationality"));
  m_onu->createNationality(newNationalityName);
  m_skinDefWidget->nationalitieslist->addItem(newNationalityName);
}

void MainWindow::slotDeleteNationality()
{
  if (m_onu == 0) return;
  kDebug();
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
  if (m_onu == 0) return;
  kDebug();
  m_onu->setFont(font);
  updateSprites(Anchor);
}

void MainWindow::slotFgSelected(const QColor& color)
{
  if (m_onu == 0) return;
  kDebug();
  m_onu->setFontFgColor(color);
  updateSprites(Anchor);
}

void MainWindow::slotBgColorSelected(const QColor& color)
{
  if (m_onu == 0) return;
  kDebug();
  m_onu->setFontBgColor(color);
  updateSprites(Anchor);
}

void MainWindow::initSpritesButtonsWith(const Country* country)
{
  m_flagButton->setEnabled(m_onu->itemFor(country, Flag) == 0);
  m_infantryButton->setEnabled(m_onu->itemFor(country, Infantry) == 0);
  m_cavalryButton->setEnabled(m_onu->itemFor(country, Cavalry) == 0);
  m_cannonButton->setEnabled(m_onu->itemFor(country, Cannon) == 0);
  m_anchorButton->setEnabled(m_onu->itemFor(country, Anchor) == 0);
  m_centerButton->setEnabled(m_onu->itemFor(country, Center) == 0);
}

void MainWindow::slotFLagxValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointFlag(QPointF(v,currentCountry()->pointFlag().y()));
    if (m_onu->itemFor(currentCountry(), Flag) != 0)
      m_onu->itemFor(currentCountry(), Flag)->setPos(currentCountry()->pointFlag());
    m_onu->setDirty();
  }
}

void MainWindow::slotFLagyValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointFlag(QPointF(currentCountry()->pointFlag().x(),v));
    if (m_onu->itemFor(currentCountry(), Flag) != 0)
      m_onu->itemFor(currentCountry(), Flag)->setPos(currentCountry()->pointFlag());
    m_onu->setDirty();
  }
}

void MainWindow::slotCenterxValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->centralPoint(QPointF(v,currentCountry()->centralPoint().y()));
    if (m_onu->itemFor(currentCountry(), Center) != 0)
      m_onu->itemFor(currentCountry(), Center)->setPos(currentCountry()->centralPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotCenteryValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->centralPoint(QPointF(currentCountry()->centralPoint().x(),v));
    if (m_onu->itemFor(currentCountry(), Center) != 0)
      m_onu->itemFor(currentCountry(), Center)->setPos(currentCountry()->centralPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotAnchorxValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->anchorPoint(QPointF(v,currentCountry()->anchorPoint().y()));
    if (m_onu->itemFor(currentCountry(), Anchor) != 0)
      m_onu->itemFor(currentCountry(), Anchor)->setPos(currentCountry()->anchorPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotAnchoryValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->anchorPoint(QPointF(currentCountry()->anchorPoint().x(),v));
    if (m_onu->itemFor(currentCountry(), Anchor) != 0)
      m_onu->itemFor(currentCountry(), Anchor)->setPos(currentCountry()->anchorPoint());
    m_onu->setDirty();
  }
}

void MainWindow::slotInfantryxValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointInfantry(QPointF(v,currentCountry()->pointInfantry().y()));
    if (m_onu->itemFor(currentCountry(), Infantry) != 0)
      m_onu->itemFor(currentCountry(), Infantry)->setPos(currentCountry()->pointInfantry());
    m_onu->setDirty();
  }
}

void MainWindow::slotInfantryyValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointInfantry(QPointF(currentCountry()->pointInfantry().x(),v));
    if (m_onu->itemFor(currentCountry(), Infantry) != 0)
      m_onu->itemFor(currentCountry(), Infantry)->setPos(currentCountry()->pointInfantry());
    m_onu->setDirty();
  }
}

void MainWindow::slotCavalryxValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointCavalry(QPointF(v,currentCountry()->pointCavalry().y()));
    if (m_onu->itemFor(currentCountry(), Cavalry) != 0)
      m_onu->itemFor(currentCountry(), Cavalry)->setPos(currentCountry()->pointCavalry());
    m_onu->setDirty();
  }
}

void MainWindow::slotCavalryyValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointCavalry(QPointF(currentCountry()->pointCavalry().x(),v));
    if (m_onu->itemFor(currentCountry(), Cavalry) != 0)
      m_onu->itemFor(currentCountry(), Cavalry)->setPos(currentCountry()->pointCavalry());
    m_onu->setDirty();
  }
}

void MainWindow::slotCannonxValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointCannon(QPointF(v,currentCountry()->pointCannon().y()));
    if (m_onu->itemFor(currentCountry(), Cannon) != 0)
      m_onu->itemFor(currentCountry(), Cannon)->setPos(currentCountry()->pointCannon());
    m_onu->setDirty();
  }
}

void MainWindow::slotCannonyValueChanged(int v)
{
  kDebug();
  if (currentCountry() != 0)
  {
    currentCountry()->pointCannon(QPointF(currentCountry()->pointCannon().x(),v));
    if (m_onu->itemFor(currentCountry(), Cannon) != 0)
      m_onu->itemFor(currentCountry(), Cannon)->setPos(currentCountry()->pointCannon());
    m_onu->setDirty();
  }
}


} // closing namespace

#include "mainwindow.moc"
