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
#include "ksirkspritesdefinition.h"
#include "ksirkskineditorpixmapitem.h"
#include "ksirkskineditortextitem.h"
#include "ksirkskineditorcountriesselectiondialog.h"
#include "skinSpritesData.h"

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
  m_rfa(0)
{
  kDebug() << "MainWindow constructor begin";
  KSirkSkinEditorWidget* mainWidget = new KSirkSkinEditorWidget(this);
  setCentralWidget(mainWidget);
  
  m_mapScene = new Scene(0,0,1024,768,this);
  connect(m_mapScene,SIGNAL(position(const QPointF&)),this,SLOT(slotPosition(const QPointF&)));
  connect(m_mapScene,SIGNAL(pressPosition(const QPointF&)),this,SLOT(slotPressPosition(const QPointF&)));
  m_mapView = new QGraphicsView(m_mapScene,mainWidget->mapScrollArea);
  m_mapView->setInteractive(true);

  mainWidget->mapScrollArea->setBackgroundRole(QPalette::Dark);
  mainWidget->mapScrollArea->setWidget(m_mapView);
  
  QHBoxLayout* layout = new QHBoxLayout;
  mainWidget->buttonsScrollArea->setLayout(layout);

  m_flagButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_flagButton);
  m_flagButton->setCheckable(true);
  connect(m_flagButton,SIGNAL(clicked()),this,SLOT(slotFlagButtonClicked()));

  m_infantryButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_infantryButton);
  m_infantryButton->setCheckable(true);
  connect(m_infantryButton,SIGNAL(clicked()),this,SLOT(slotInfantryButtonClicked()));

  m_cavalryButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_cavalryButton);
  m_cavalryButton->setCheckable(true);
  connect(m_cavalryButton,SIGNAL(clicked()),this,SLOT(slotCavalryButtonClicked()));

  m_cannonButton = new QPushButton(mainWidget->buttonsScrollArea);
  layout->addWidget(m_cannonButton);
  m_cannonButton->setCheckable(true);
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
  m_skinDefWidget->countrieslist->setSortingEnabled (true);
  addDockWidget ( Qt::LeftDockWidgetArea, m_skinDefWidget);
  
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
  
  connect(m_skinDefWidget->countrieslist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotCountrySelected(QListWidgetItem*)));

  connect (m_countryDefWidget->neighboursbutton, SIGNAL(clicked()), this, SLOT(slotNeighbours()));

  connect (m_skinDefWidget->skinNameLineEdit, SIGNAL(editingFinished()), this, SLOT(slotSkinNameEdited()));

  QIntValidator* skinWidthValidator = new QIntValidator(this);
  m_skinDefWidget->widthLineEdit->setValidator(skinWidthValidator);
  connect (m_skinDefWidget->widthLineEdit, SIGNAL(editingFinished()), this, SLOT(slotSkinWidthEdited()));

  QIntValidator* skinHeightValidator = new QIntValidator(this);
  m_skinDefWidget->heightLineEdit->setValidator(skinHeightValidator);
  connect (m_skinDefWidget->heightLineEdit, SIGNAL(editingFinished()), this, SLOT(slotSkinHeightEdited()));

  connect (m_skinDefWidget->descriptionTextEdit,SIGNAL(textChanged()), this, SLOT(slotSkinDescriptionEdited()));

  QIntValidator* heightDiffValidator = new QIntValidator(this);
  m_skinDefWidget->heightDiffLineEdit->setValidator(heightDiffValidator);
  connect (m_skinDefWidget->heightDiffLineEdit, SIGNAL(editingFinished()), this, SLOT(slotSkinHeightDiffEdited()));

  QIntValidator* widthDiffValidator = new QIntValidator(this);
  m_skinDefWidget->widthDiffLineEdit->setValidator(widthDiffValidator);
  connect (m_skinDefWidget->widthDiffLineEdit, SIGNAL(editingFinished()), this, SLOT(slotSkinWidthDiffEdited()));

  connect(m_skinDefWidget->newCountryButton, SIGNAL(clicked()), this, SLOT(slotNewCountry()));
  connect(m_skinDefWidget->deleteCountryButton, SIGNAL(clicked()), this, SLOT(slotDeleteCountry()));    

  connect(m_skinDefWidget->continentslist, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotContinentSelected(QListWidgetItem*)));

  connect (m_continentDefWidget->selectcountriesbutton, SIGNAL(clicked()), this, SLOT(slotContinentCountries()));
  
  connect (m_continentDefWidget->bonus, SIGNAL(editingFinished()), this, SLOT(slotContinentBonusEdited()));
  
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

  m_rfa = KStandardGameAction::loadRecent (this, SLOT(slotURLSelected(const KUrl&)), this);
  actionCollection()->addAction(m_rfa->objectName(), m_rfa);
  KSharedConfig::Ptr config = KGlobal::config();
  kDebug() << "loading recent files";
  m_rfa->loadEntries(KGlobal::config()->group("ksirkskineditor"));
  
  m_saveGameAction = KStandardGameAction::save(this, SLOT(slotSaveSkin()), this);
  actionCollection()->addAction(m_saveGameAction->objectName(), m_saveGameAction);
  
  action = KStandardGameAction::quit(this, SLOT(close()), this);
  actionCollection()->addAction(action->objectName(), action);
  
//   action = KStandardGameAction::gameNew(this, SLOT(slotNewGame()), this);
//   actionCollection()->addAction(action->objectName(), action);

  action = KStandardAction::zoomIn(this, SLOT(slotZoomIn()), this);
  actionCollection()->addAction(action->objectName(), action);

  action = KStandardAction::zoomOut(this, SLOT(slotZoomOut()), this);
  actionCollection()->addAction(action->objectName(), action);

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
  
  m_skinDefWidget->skinNameLineEdit->setText(m_onu->name());
  m_skinDefWidget->widthLineEdit->setText(QString::number(m_onu->width()));
  m_skinDefWidget->heightLineEdit->setText(QString::number(m_onu->height()));
  m_skinDefWidget->descriptionTextEdit->setText(m_onu->description());
  m_skinDefWidget->heightDiffLineEdit->setText(QString::number(SkinSpritesData::changeable().intData("fighters-flag-y-diff")));
  m_skinDefWidget->widthDiffLineEdit->setText(QString::number(SkinSpritesData::changeable().intData("width-between-flag-and-fighter")));

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
    kDebug() << "Adding "<<continent->name()<<" items";
    m_skinDefWidget->continentslist->addItem(continent->name());
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
    // TODO : Test si jeu en cours
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

void MainWindow::slotZoomIn()
{
  kDebug();
}

void MainWindow::slotZoomOut()
{
  kDebug();
}

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
  kDebug() << point;
  QString message("");
  QTextStream ts( &message );
  ts << point.x() << " x " << point.y();
  statusBar()->showMessage(message);
}

void MainWindow::slotPressPosition(const QPointF& point)
{
  kDebug() << point;
  QPixmap pix;
  QPixmap alphacopy;
  QString fileName;
  
  if (currentCountry() == 0
    || m_onu->itemFor(currentCountry(), m_selectedSprite) != 0)
  {
    kDebug() << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    return;
  }
  PixmapItem* item = new PixmapItem();
  item->setZValue(10);
  connect(item, SIGNAL(pressed(QGraphicsItem*)),this, SLOT(slotItemPressed(QGraphicsItem*)));
  connect(item, SIGNAL(placed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPlaced(QGraphicsItem*, const QPointF&)));
  
  switch (m_selectedSprite)
  {
    case Flag:
      pix = m_onu->flagIcon();
//       pix = pix.scaled(20,20);
      m_countryDefWidget->flagx->setText(QString::number(point.x()));
      m_countryDefWidget->flagy->setText(QString::number(point.y()));
      currentCountry()->pointFlag(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Flag));
      break;
    case Infantry:
      pix = m_onu->infantryIcon();
//       pix = pix.scaled(23,32);
      m_countryDefWidget->infantryx->setText(QString::number(point.x()));
      m_countryDefWidget->infantryy->setText(QString::number(point.y()));
      currentCountry()->pointInfantry(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Infantry));
      break;
    case Cavalry:
      pix = m_onu->cavalryIcon();
//       pix = pix.scaled(32,32);
      m_countryDefWidget->cavalryx->setText(QString::number(point.x()));
      m_countryDefWidget->cavalryy->setText(QString::number(point.y()));
      currentCountry()->pointCavalry(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Cavalry));
      break;
    case Cannon:
      kDebug() << "Adding cannon";
      pix = m_onu->cannonIcon();
//       pix = pix.scaled(32,32);
      m_countryDefWidget->cannonx->setText(QString::number(point.x()));
      m_countryDefWidget->cannony->setText(QString::number(point.y()));
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
      m_countryDefWidget->anchorx->setText(QString::number(point.x()));
      m_countryDefWidget->anchory->setText(QString::number(point.y()));
      currentCountry()->anchorPoint(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Anchor));
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
      m_countryDefWidget->centerx->setText(QString::number(point.x()));
      m_countryDefWidget->centery->setText(QString::number(point.y()));
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

void MainWindow::slotCountrySelected(QListWidgetItem* item)
{
  kDebug();
  Country* country = m_onu->countryNamed(item->text());
  if (country != 0)
  {
    initCountryWidgetWith(country);
  }
}

void MainWindow::initCountryWidgetWith(Country* country)
{
  kDebug();
  m_countryDefWidget->flagx->setText(QString::number(country->pointFlag().x()));
  m_countryDefWidget->flagy->setText(QString::number(country->pointFlag().y()));

  m_countryDefWidget->anchorx->setText(QString::number(country->anchorPoint().x()));
  m_countryDefWidget->anchory->setText(QString::number(country->anchorPoint().y()));

  m_countryDefWidget->centerx->setText(QString::number(country->centralPoint().x()));
  m_countryDefWidget->centery->setText(QString::number(country->centralPoint().y()));

  m_countryDefWidget->infantryx->setText(QString::number(country->pointInfantry().x()));
  m_countryDefWidget->infantryy->setText(QString::number(country->pointInfantry().y()));

  m_countryDefWidget->cavalryx->setText(QString::number(country->pointCavalry().x()));
  m_countryDefWidget->cavalryy->setText(QString::number(country->pointCavalry().y()));

  m_countryDefWidget->cannonx->setText(QString::number(country->pointCannon().x()));
  m_countryDefWidget->cannony->setText(QString::number(country->pointCannon().y()));

  m_countryDefWidget->anchorx->setText(QString::number(country->anchorPoint().x()));
  m_countryDefWidget->anchory->setText(QString::number(country->anchorPoint().y()));

  m_countryDefWidget->centerx->setText(QString::number(country->centralPoint().x()));
  m_countryDefWidget->centery->setText(QString::number(country->centralPoint().y()));

  m_countryDefWidget->neighbourslist->clear();
  foreach(Country* neighbour, country->neighbours())
  {
    m_countryDefWidget->neighbourslist->addItem(neighbour->name());
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
  m_continentDefWidget->bonus->setText(QString::number(continent->bonus()));

    m_continentDefWidget->countrieslist->clear();
  foreach(Country* country, continent->members())
  {
    m_continentDefWidget->countrieslist->addItem(country->name());
  }
}

Country* MainWindow::currentCountry()
{
  if (m_skinDefWidget->countrieslist->currentItem() == 0)
    return 0;
  kDebug() << m_skinDefWidget->countrieslist->currentItem()->text();
  return m_onu->countryNamed(m_skinDefWidget->countrieslist->currentItem()->text());
}

void MainWindow::slotItemPlaced(QGraphicsItem* item, const QPointF&)
{
  kDebug();
  if (m_onu->itemsMap().contains(item))
  {
    Country* country = m_onu->itemsMap()[item].first;
    SpriteType type = m_onu->itemsMap()[item].second;
    initCountryWidgetWith(country);
    for (int i = 0; i != m_skinDefWidget->countrieslist->count(); i++)
    {
      if (m_skinDefWidget->countrieslist->item(i)->text() == country->name())
      {
        m_skinDefWidget->countrieslist->setCurrentRow (i, QItemSelectionModel::ClearAndSelect);
        break;
      }
    }
    QPointF anchorPos;
    switch (type)
    {
      case Flag:
        country->pointFlag(item->scenePos());
        m_countryDefWidget->flagx->setText(QString::number(item->scenePos().x()));
        m_countryDefWidget->flagy->setText(QString::number(item->scenePos().y()));
        break;
      case Infantry:
        country->pointInfantry(item->scenePos());
        m_countryDefWidget->infantryx->setText(QString::number(item->scenePos().x()));
        m_countryDefWidget->infantryy->setText(QString::number(item->scenePos().y()));
        break;
      case Cavalry:
        country->pointCavalry(item->scenePos());
        m_countryDefWidget->cavalryx->setText(QString::number(item->scenePos().x()));
        m_countryDefWidget->cavalryy->setText(QString::number(item->scenePos().y()));
        break;
      case Cannon:
        country->pointCannon(item->scenePos());
        m_countryDefWidget->cannonx->setText(QString::number(item->scenePos().x()));
        m_countryDefWidget->cannony->setText(QString::number(item->scenePos().y()));
        break;
      case Anchor:
        anchorPos = QPointF(item->scenePos().x()+(item->boundingRect().width()/2),item->scenePos().y()+(item->boundingRect().height()/2));
        country->anchorPoint(anchorPos);
        m_countryDefWidget->anchorx->setText(QString::number(anchorPos.x()));
        m_countryDefWidget->anchory->setText(QString::number(anchorPos.y()));
        break;
      case Center:
        country->centralPoint(item->scenePos());
        m_countryDefWidget->centerx->setText(QString::number(item->scenePos().x()));
        m_countryDefWidget->centery->setText(QString::number(item->scenePos().y()));
        break;
      default:;
    }
  }
}

void MainWindow::slotItemPressed(QGraphicsItem* item)
{
  kDebug();
  if (m_onu->itemsMap().contains(item))
  {
    Country* country = m_onu->itemsMap()[item].first;
    initCountryWidgetWith(country);
    for (int i = 0; i != m_skinDefWidget->countrieslist->count(); i++)
    {
      if (m_skinDefWidget->countrieslist->item(i)->text() == country->name())
      {
        m_skinDefWidget->countrieslist->setCurrentRow (i, QItemSelectionModel::ClearAndSelect);
        break;
      }
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
      pix = m_onu->flagIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      //       pix = pix.scaled(20,20);
      point = country->pointFlag();
      m_countryDefWidget->flagx->setText(QString::number(point.x()));
      m_countryDefWidget->flagy->setText(QString::number(point.y()));
      m_onu->itemsMap().insert(item,qMakePair(country,Flag));
      break;
    case Infantry:
      item = new PixmapItem();
      pix = m_onu->infantryIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      point = country->pointInfantry();
      //       pix = pix.scaled(23,32);
      m_countryDefWidget->infantryx->setText(QString::number(point.x()));
      m_countryDefWidget->infantryy->setText(QString::number(point.y()));
      m_onu->itemsMap().insert(item,qMakePair(country,Infantry));
      break;
    case Cavalry:
      item = new PixmapItem();
      pix = m_onu->cavalryIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      point = country->pointCavalry();
      //       pix = pix.scaled(32,32);
      m_countryDefWidget->cavalryx->setText(QString::number(point.x()));
      m_countryDefWidget->cavalryy->setText(QString::number(point.y()));
      m_onu->itemsMap().insert(item,qMakePair(country,Cavalry));
      break;
    case Cannon:
      item = new PixmapItem();
      kDebug() << "Adding cannon";
      pix = m_onu->cannonIcon();
      ((PixmapItem*)item)->setPixmap(pix);
      point = country->pointCannon();
      //       pix = pix.scaled(32,32);
      m_countryDefWidget->cannonx->setText(QString::number(point.x()));
      m_countryDefWidget->cannony->setText(QString::number(point.y()));
      m_onu->itemsMap().insert(item,qMakePair(country,Cannon));
      break;
    case Anchor:
      item = new TextItem();
      ((TextItem*)item)->setFont(m_onu->foregroundFont());
      kDebug() << "Adding anchor";
      fileName = m_dirs-> findResource("appdata", "cross.png");
      if (fileName.isNull())
      {
        KMessageBox::error(0, i18n("Cannot load anchor icon<br>Program cannot continue"), i18n("Error !"));
        exit(2);
      }
      pix = QPixmap(fileName);
      point = country->anchorPoint();
      pix = pix.scaled(16,16);
      ((TextItem*)item)->setPlainText(country->name());
      m_countryDefWidget->anchorx->setText(QString::number(point.x()));
      m_countryDefWidget->anchory->setText(QString::number(point.y()));
      m_onu->itemsMap().insert(item,qMakePair(country,Anchor));
      break;
    case Center:
      item = new PixmapItem();
      kDebug() << "Adding center";
      fileName = m_dirs-> findResource("appdata", "target.png");
      if (fileName.isNull())
      {
        KMessageBox::error(0, i18n("Cannot load center icon<br>Program cannot continue"), i18n("Error !"));
        exit(2);
      }
      pix = QPixmap(fileName);
      point = country->centralPoint();
      pix = pix.scaled(16,16);
      ((PixmapItem*)item)->setPixmap(pix);
      m_countryDefWidget->anchorx->setText(QString::number(point.x()));
      m_countryDefWidget->anchory->setText(QString::number(point.y()));
      m_onu->itemsMap().insert(item,qMakePair(country,Center));
      break;
    default: ;
  }
  if (item != 0 && !point.isNull())
  {
    if (dynamic_cast<PixmapItem*>(item) != 0)
    {
      connect(dynamic_cast<PixmapItem*>(item),SIGNAL(pressed(QGraphicsItem*)),this, SLOT(slotItemPressed(QGraphicsItem*)));
      connect(dynamic_cast<PixmapItem*>(item),SIGNAL(placed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPlaced(QGraphicsItem*, const QPointF&)));
      item->setPos(point);
    }
    else if (dynamic_cast<TextItem*>(item) != 0)
    {
      connect(dynamic_cast<TextItem*>(item),SIGNAL(pressed(QGraphicsItem*)),this, SLOT(slotItemPressed(QGraphicsItem*)));
      connect(dynamic_cast<TextItem*>(item), SIGNAL(placed(QGraphicsItem*, const QPointF&)),this, SLOT(slotItemPlaced(QGraphicsItem*, const QPointF&)));
      QPointF pos(point.x()-(item->boundingRect().width()/2),
                   point.y()-(item->boundingRect().height()/2));
      item->setPos(pos);
    }
    item->setZValue(10);
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

void MainWindow::slotSkinWidthEdited()
{
  if (m_onu == 0) return;
  bool ok = false;
  int w = m_skinDefWidget->widthLineEdit->text().toInt(&ok);
  m_onu->setWidth(w);

  QPixmap mapPixmap(m_onu->pixmapForId("map", m_onu->width(), m_onu->height()));
  m_mapItem->setPixmap(mapPixmap);
}

void MainWindow::slotSkinHeightEdited()
{
  if (m_onu == 0) return;
  bool ok = false;
  int h = m_skinDefWidget->heightLineEdit->text().toInt(&ok);
  m_onu->setHeight(h);

  QPixmap mapPixmap(m_onu->pixmapForId("map", m_onu->width(), m_onu->height()));
  m_mapItem->setPixmap(mapPixmap);
}

void MainWindow::slotSkinDescriptionEdited()
{
  if (m_onu == 0) return;
  kDebug() << m_skinDefWidget->descriptionTextEdit->toPlainText();
  m_onu->setDescription(m_skinDefWidget->descriptionTextEdit->toPlainText());
}

void MainWindow::slotSkinHeightDiffEdited()
{
  if (m_onu == 0) return;
  kDebug();
  bool ok = false;
  int wd = m_skinDefWidget->heightDiffLineEdit->text().toInt(&ok);
  SkinSpritesData::changeable().intData("fighters-flag-y-diff", wd);
}

void MainWindow::slotSkinWidthDiffEdited()
{
  if (m_onu == 0) return;
  kDebug();
  bool ok = false;
  int wd = m_skinDefWidget->widthDiffLineEdit->text().toInt(&ok);
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
      m_countryDefWidget->hide();
      m_continentDefWidget->hide();
      m_goalDefWidget->hide();
      break;
    case 1:
      m_spritesDefWidget->hide();
      m_countryDefWidget->show();
      m_continentDefWidget->hide();
      m_goalDefWidget->hide();
      break;
    case 2:
      m_spritesDefWidget->hide();
      m_countryDefWidget->hide();
      m_continentDefWidget->show();
      m_goalDefWidget->hide();
      break;
    case 3:
      m_spritesDefWidget->hide();
      m_countryDefWidget->hide();
      m_continentDefWidget->hide();
      m_goalDefWidget->show();
      break;
    default:
      m_countryDefWidget->hide();
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

void MainWindow::slotContinentBonusEdited()
{
  kDebug();
  Continent* continent =  m_onu->continentNamed(m_skinDefWidget->continentslist->currentItem()->text());
  bool ok;
  continent->setBonus(m_continentDefWidget->bonus->text().toUInt(&ok));
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
        m_flagButton->setIcon(m_onu->flagIcon());
        break;
      case Infantry:
        pix = m_onu->infantryIcon();
        ((PixmapItem*)item)->setPixmap(pix);
        m_infantryButton->setIcon(m_onu->infantryIcon());
        break;
      case Cavalry:
        pix = m_onu->cavalryIcon();
        ((PixmapItem*)item)->setPixmap(pix);
        m_cavalryButton->setIcon(m_onu->cavalryIcon());
        break;
      case Cannon:
        pix = m_onu->cannonIcon();
        ((PixmapItem*)item)->setPixmap(pix);
        m_cannonButton->setIcon(m_onu->cannonIcon());
        break;
      default: ;
    }
  }
}

} // closing namespace

#include "mainwindow.moc"
