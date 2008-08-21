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
#include "ksirkskineditorpixmapitem.h"

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
#include <QBitmap>

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


#include <assert.h>


namespace KsirkSkinEditor
{

MainWindow::MainWindow(QWidget* parent) :
  KXmlGuiWindow(parent),
  m_selectedSprite(None)
{
  kDebug() << "MainWindow constructor begin";
  m_onu = new ONU("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default");
  KSirkSkinEditorWidget* mainWidget = new KSirkSkinEditorWidget(this);
  setCentralWidget(mainWidget);

  m_mapScene = new Scene(0,0,1024,768,this);
  connect(m_mapScene,SIGNAL(position(const QPointF&)),this,SLOT(slotPosition(const QPointF&)));
  m_mapView = new QGraphicsView(m_mapScene,this);
  m_mapView->setInteractive(true);
  connect(m_mapScene,SIGNAL(pressPosition(const QPointF&)),this,SLOT(slotPressPosition(const QPointF&)));
  m_mapView = new QGraphicsView(m_mapScene,this);

  QPixmap mapPixmap("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/map.png");
  m_mapScene->addPixmap(mapPixmap);

  mainWidget->mapScrollArea->setBackgroundRole(QPalette::Dark);
  mainWidget->mapScrollArea->setWidget(m_mapView);
  
  QHBoxLayout* layout = new QHBoxLayout;
  mainWidget->buttonsScrollArea->setLayout(layout);

  QPixmap flagIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/flag.png");
  m_flagButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_flagButton->setIcon(flagIcon);
  m_flagButton->setIconSize(QSize(20,20));
  layout->addWidget(m_flagButton);
  m_flagButton->setCheckable(true);
  connect(m_flagButton,SIGNAL(clicked()),this,SLOT(slotFlagButtonClicked()));

  QPixmap infantryIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/infantry.png");
  m_infantryButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_infantryButton->setIcon(infantryIcon);
  m_infantryButton->setIconSize(QSize(23,32));
  layout->addWidget(m_infantryButton);
  m_infantryButton->setCheckable(true);
  connect(m_infantryButton,SIGNAL(clicked()),this,SLOT(slotInfantryButtonClicked()));

  QPixmap cavalryIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/cavalry.png");
  m_cavalryButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_cavalryButton->setIcon(cavalryIcon);
  m_cavalryButton->setIconSize(QSize(32,32));
  layout->addWidget(m_cavalryButton);
  m_cavalryButton->setCheckable(true);
  connect(m_cavalryButton,SIGNAL(clicked()),this,SLOT(slotCavalryButtonClicked()));

  QPixmap cannonIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/cannon.png");
  m_cannonButton = new QPushButton(mainWidget->buttonsScrollArea);
  m_cannonButton->setIcon(cannonIcon);
  m_cannonButton->setIconSize(QSize(32,32));
  layout->addWidget(m_cannonButton);
  m_cannonButton->setCheckable(true);
  connect(m_cannonButton,SIGNAL(clicked()),this,SLOT(slotCannonButtonClicked()));

  m_dirs = KGlobal::dirs();
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

  kDebug() << "Setting up GUI";
  setupGUI();

//    kDebug() << "Before initStatusBar";
  initStatusBar();
  
  menuBar()-> show();
  
  setMouseTracking(true);

  m_skinDefWidget = new KSirkSkinDefinitionWidget(this);
  addDockWidget ( Qt::LeftDockWidgetArea, m_skinDefWidget);
  
  m_countryDefWidget = new KsirkCountryDefinitionWidget(this);
  addDockWidget ( Qt::LeftDockWidgetArea, m_countryDefWidget);

  m_skinDefWidget->countrieslist->setSortingEnabled (true);
  foreach (Country* country, m_onu->getCountries())
  {
    m_skinDefWidget->countrieslist->addItem(country->name());
  }

  connect(
    m_skinDefWidget->countrieslist,
    SIGNAL(itemClicked(QListWidgetItem*)),
    this,
    SLOT(slotCountrySelected(QListWidgetItem*)));

}

MainWindow::~MainWindow()
{
  kDebug();
  m_dirs = 0;
}

void MainWindow::initActions()
{
  QAction *action;

  // standard game actions
//   action = KStandardGameAction::gameNew(this, SLOT(slotNewGame()), this);
//   actionCollection()->addAction(action->objectName(), action);
//   action = KStandardGameAction::load(this, SLOT(slotOpenGame()), this);
//   actionCollection()->addAction(action->objectName(), action);
//   m_saveGameAction = KStandardGameAction::save(this, SLOT(slotSaveGame()), this);
//   actionCollection()->addAction(m_saveGameAction->objectName(), m_saveGameAction);
//   action = KStandardGameAction::quit(this, SLOT(close()), this);
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

/**
  * Reimplementation of the inherited function called when a window close event arise
  */
bool MainWindow::queryClose()
{
    // TODO : Test si jeu en cours
  return true;
}

void MainWindow::saveXml(std::ostream& xmlStream)
{
  xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  xmlStream << "<ksirkskin formatVersion=\"2.0\" >" << std::endl;
  xmlStream << "</ksirkskin>" << std::endl;
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
  if (m_selectedSprite == Flag)
  {
    m_flagButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
  {
    QPixmap flagIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/flag.png");
    m_mapView->setCursor(QCursor(flagIcon.scaled(20,20),0,0));
    m_selectedSprite = Flag;
    m_flagButton->setChecked(true);
  }
}

void MainWindow::slotInfantryButtonClicked()
{
  if (m_selectedSprite == Infantry)
  {
    m_infantryButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
  {
    QPixmap infantryIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/infantry.png");
    m_mapView->setCursor(QCursor(infantryIcon.scaled(23,32),0,0));
    m_selectedSprite = Infantry;
    m_infantryButton->setChecked(true);
  }
}

void MainWindow::slotCavalryButtonClicked()
{
  if (m_selectedSprite == Cavalry)
  {
    m_cavalryButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
  {
    QPixmap cavalryIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/cavalry.png");
    m_mapView->setCursor(QCursor(cavalryIcon.scaled(32,32),0,0));
    m_selectedSprite = Cavalry;
    m_cavalryButton->setChecked(true);
  }
}

void MainWindow::slotCannonButtonClicked()
{
  if (m_selectedSprite == Cannon)
  {
    m_cannonButton->setChecked(false);
    m_mapView->unsetCursor();
  }
  else if (m_skinDefWidget->countrieslist->currentItem()!=0)
  {
    QPixmap cannonIcon("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/cannon.png");
    m_mapView->setCursor(QCursor(cannonIcon.scaled(32,32),0,0));
    m_selectedSprite = Cannon;
    m_cannonButton->setChecked(true);
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
  if (currentCountry() == 0
    || m_onu->itemFor(currentCountry(), m_selectedSprite) != 0)
  {
    kDebug() << (void*)currentCountry() << (void*)m_onu->itemFor(currentCountry(), m_selectedSprite);
    return;
  }
  PixmapItem* item = new PixmapItem();
  connect(item, SIGNAL(placed(PixmapItem*, const QPointF&)),
           this, SLOT(slotPixmapPlaced(PixmapItem*, const QPointF&)));
  
  switch (m_selectedSprite)
  {
    case Flag:
      pix = QPixmap("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/flag.png");
      pix = pix.scaled(20,20);
      m_countryDefWidget->flagx->setText(QString::number(point.x()));
      m_countryDefWidget->flagy->setText(QString::number(point.y()));
      currentCountry()->pointFlag(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Flag));
      break;
    case Infantry:
      pix = QPixmap("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/infantry.png");
      pix = pix.scaled(23,32);
      m_countryDefWidget->infantryx->setText(QString::number(point.x()));
      m_countryDefWidget->infantryy->setText(QString::number(point.y()));
      currentCountry()->pointInfantry(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Infantry));
      break;
    case Cavalry:
      pix = QPixmap("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/cavalry.png");
      pix = pix.scaled(32,32);
      m_countryDefWidget->cavalryx->setText(QString::number(point.x()));
      m_countryDefWidget->cavalryy->setText(QString::number(point.y()));
      currentCountry()->pointCavalry(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Cavalry));
      break;
    case Cannon:
      kDebug() << "Adding cannon";
      pix = QPixmap("/home/kleag/Developpements/C++/kde-kdegames-trunk/ksirk/ksirk/skins/default/Images/cannon.png");
      pix = pix.scaled(32,32);
      m_countryDefWidget->cannonx->setText(QString::number(point.x()));
      m_countryDefWidget->cannony->setText(QString::number(point.y()));
      currentCountry()->pointCannon(point);
      m_onu->itemsMap().insert(item,qMakePair(currentCountry(),Cannon));
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
}

Country* MainWindow::currentCountry()
{
  if (m_skinDefWidget->countrieslist->currentItem() == 0)
    return 0;
  kDebug() << m_skinDefWidget->countrieslist->currentItem()->text();
  return m_onu->countryNamed(m_skinDefWidget->countrieslist->currentItem()->text());
}

void MainWindow::slotPixmapPlaced(PixmapItem* item, const QPointF& point)
{
  kDebug();
  if (m_onu->itemsMap().contains(item))
  {
    Country* country = m_onu->itemsMap()[item].first;
    SpriteType type = m_onu->itemsMap()[item].second;
    switch (type)
    {
      case Flag:
        country->pointFlag(point);
        m_countryDefWidget->flagx->setText(QString::number(point.x()));
        m_countryDefWidget->flagy->setText(QString::number(point.y()));
        break;
      case Infantry:
        country->pointInfantry(point);
        m_countryDefWidget->infantryx->setText(QString::number(point.x()));
        m_countryDefWidget->infantryy->setText(QString::number(point.y()));
        break;
      case Cavalry:
        country->pointCavalry(point);
        m_countryDefWidget->cavalryx->setText(QString::number(point.x()));
        m_countryDefWidget->cavalryy->setText(QString::number(point.y()));
        break;
      case Cannon:
        country->pointCannon(point);
        m_countryDefWidget->cannonx->setText(QString::number(point.x()));
        m_countryDefWidget->cannony->setText(QString::number(point.y()));
        break;
      default:;
    }
  }
}

} // closing namespace

#include "mainwindow.moc"
