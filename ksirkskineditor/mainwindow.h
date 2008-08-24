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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ksirkskineditorwidget.h"
#include "onu.h"
#include "spritetype.h"

// include files for Qt

#include <QPointF>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox>
#include <QSplitter>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
// include files for KDE
#include <ksharedconfig.h>
#include <KXmlGuiWindow>
#include <KStandardDirs>
#include <KShortcut>
#include <KUrl>
// include files for kde games

class KAction;
class KToolBar;
class KDialog;
class KRecentFilesAction;

class QEvent;
class QDockWidget;
class QGraphicsScene;
class QGraphicsView;
class QListWidgetItem;
class QPushButton;
class QGraphicsPixmapItem;

class KSirkSkinDefinitionWidget;
class KsirkCountryDefinitionWidget;
class KsirkContinentDefinitionWidget;
class KsirkGoalDefinitionWidget;
class KsirkSpritesDefinitionWidget;

namespace Phonon
{
  class MediaObject;
}

namespace KsirkSkinEditor
{
class PixmapItem;

/**
  * This is the main window.
  *
  * @author Gael de Chalendar (aka Kleag)
  * @version $Id: $
  */
class MainWindow : public KXmlGuiWindow
{
  Q_OBJECT

public:
  /**
    * Create the window and initializes its members
    */
  MainWindow(QWidget* parent=0);
  
  /**
    * Deletes the background and the pool
    */
  ~MainWindow();
    
protected:
  /**
    * Add the main toolbar buttons
    */
  void initActions();

  /**
    * Prepares the status bar
    */
  void initStatusBar();

  /**
    * creates and display the main frame with the background
    */
  void initView();

  /**
    * Reimplementation of the inherited function called when a window close event arise
    */
  bool queryClose();
  bool queryExit();
    

public Q_SLOTS:

  /**
    * The standard slotShowAboutApplication slot
    */
  void slotShowAboutApplication();

  void slotZoomIn();
  void slotZoomOut();

private Q_SLOTS:
  void optionsConfigure();

  void slotFlagButtonClicked();
  void slotInfantryButtonClicked();
  void slotCavalryButtonClicked();
  void slotCannonButtonClicked();
  void slotAnchorButtonClicked();
  void slotCenterButtonClicked();
  
  void slotPosition(const QPointF&);
  void slotPressPosition(const QPointF&);

  void slotCountrySelected(QListWidgetItem* item);
  void slotContinentSelected(QListWidgetItem* item);
  
  void slotItemPressed(QGraphicsItem*);
  void slotItemPlaced(QGraphicsItem*, const QPointF&);
  
  void slotOpenSkin(const QString& dir = QString());
  void slotSaveSkin();

  void slotURLSelected(const KUrl&);

  void slotSkinNameEdited();
  void slotSkinWidthEdited();
  void slotSkinHeightEdited();
  void slotSkinDescriptionEdited();

  void slotSkinHeightDiffEdited();
  void slotSkinWidthDiffEdited();

  void slotNewCountry();
  void slotDeleteCountry();

  void slotNeighbours();
  void slotSkinPartTabChanged(int index);
  void slotContinentCountries();
  void slotContinentBonusEdited();

  void slotFlagWidthChanged(int);
  void slotFlagHeightChanged(int);
  void slotFlagFramesChanged(int);
  void slotFlagVersionsChanged(int);
  
  void slotInfantryWidthChanged(int);
  void slotInfantryHeightChanged(int);
  void slotInfantryFramesChanged(int);
  void slotInfantryVersionsChanged(int);
  
  void slotCavalryWidthChanged(int);
  void slotCavalryHeightChanged(int);
  void slotCavalryFramesChanged(int);
  void slotCavalryVersionsChanged(int);
  
  void slotCannonWidthChanged(int);
  void slotCannonHeightChanged(int);
  void slotCannonFramesChanged(int);
  void slotCannonVersionsChanged(int);
  
  void slotFiringWidthChanged(int);
  void slotFiringHeightChanged(int);
  void slotFiringFramesChanged(int);
  void slotFiringVersionsChanged(int);
  
  void slotExplodingWidthChanged(int);
  void slotExplodingHeightChanged(int);
  void slotExplodingFramesChanged(int);
  void slotExplodingVersionsChanged(int);
  
  private:
  void initCountryWidgetWith(Country* country);
  void initContinentWidgetWith(Continent* continent);
  void updateSprites(SpriteType type);
  
  Country* currentCountry();
  void createPixmapFor(Country* country, SpriteType type);
  
  /**
    * a shortcut to the standard dirs object.
    */
  KStandardDirs* m_dirs;

  /**
    * Audio player object: play all the sounds of the game.
    */
  Phonon::MediaObject* m_audioPlayer;

  // the current saved game file name
  QString m_fileName;

  KAction* m_saveGameAction;

  QGraphicsView* m_mapView;
  QGraphicsScene* m_mapScene;
  
  SpriteType m_selectedSprite;

  ONU* m_onu;

  KSirkSkinDefinitionWidget* m_skinDefWidget;
  KsirkCountryDefinitionWidget* m_countryDefWidget;
  KsirkContinentDefinitionWidget* m_continentDefWidget;
  KsirkGoalDefinitionWidget* m_goalDefWidget;
  KsirkSpritesDefinitionWidget* m_spritesDefWidget;
  
  QPushButton* m_flagButton;
  QPushButton* m_infantryButton;
  QPushButton* m_cavalryButton;
  QPushButton* m_cannonButton;
  QPushButton* m_anchorButton;
  QPushButton* m_centerButton;
  
  KRecentFilesAction* m_rfa;

  QGraphicsPixmapItem* m_mapItem;
};

} // closing namespace KsirkSkinEditor

#endif 

