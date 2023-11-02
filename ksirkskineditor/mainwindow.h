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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ksirkskineditorwidget.h"
#include "onu.h"
#include "spritetype.h"

// include files for Qt

#include <QPointF>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox>
#include <QSplitter>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUrl>
// include files for KDE
#include <KSharedConfig>
#include <KXmlGuiWindow>
// include files for kde games

class QAction;
class KRecentFilesAction;

class QGraphicsView;
class QListWidgetItem;
class QGraphicsPixmapItem;

class KSirkSkinDefinitionWidget;
class KsirkCountryDefinitionWidget;
class KsirkContinentDefinitionWidget;
class KsirkGoalDefinitionWidget;
class KsirkSpritesDefinitionWidget;
class KsirkNationalityDefinitionWidget;

namespace KsirkSkinEditor
{
  class PixmapItem;
  class Goal;
  class Scene;
  
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
  explicit MainWindow(QWidget* parent=nullptr);
  
  /**
    * Deletes the background and the pool
    */
  ~MainWindow() override;
    
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
  bool queryClose() override;

public Q_SLOTS:

  /**
    * The standard slotShowAboutApplication slot
    */
  void slotShowAboutApplication();

//   void slotZoomIn();
//   void slotZoomOut();

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
  
  void slotItemPressed(QGraphicsItem*, const QPointF&);
  void slotItemPlaced(QGraphicsItem*, const QPointF&);
  
  void slotOpenSkin();
  void slotSaveSkin();

  void slotURLSelected(const QUrl&);

  void slotSkinNameEdited();
  void slotSkinWidthEdited(int);
  void slotSkinHeightEdited(int);
  void slotSkinDescriptionEdited();

  void slotSkinWidthDiffEdited();

  void slotNewCountry();
  void slotDeleteCountry();

  void slotNeighbours();
  void slotSkinPartTabChanged(int index);
  void slotContinentCountries();
  void slotContinentBonusEdited(int);

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

  void slotNewContinent();
  void slotDeleteContinent();

  void slotGoalSelected(QListWidgetItem* item);

  void slotGoalTypeWorldClicked();
  void slotGoalTypePlayerClicked();
  void slotGoalTypeCountriesClicked();
  void slotGoalTypeContinentsClicked();
  void slotGoalDescriptionEdited();
  void slotGoalNbCountriesChanged(int);
  void slotGoalNbArmiesByCountryChanged(int);
  void slotGoalAnyContinentChanged(int);
  void slotGoalContinents();
  void slotNewGoal();
  void slotDeleteGoal();

  void slotNationalitySelected(QListWidgetItem* item);
  void slotNationalityNameEdited();
  void slotNationalityLeaderNameEdited();
  void slotNationalityFlagEdited(int);
  void slotNewNationality();
  void slotDeleteNationality();
  
  void slotFontSelected(const QFont& font);
  void slotFgSelected(const QColor& color);
  void slotBgColorSelected(const QColor& color);
  void slotReleasePosition(const QPointF&);

  void slotFLagxValueChanged(int);
  void slotFLagyValueChanged(int v);
  void slotCenterxValueChanged(int);
  void slotCenteryValueChanged(int v);
  void slotAnchorxValueChanged(int);
  void slotAnchoryValueChanged(int v);
  void slotCavalryxValueChanged(int);
  void slotCavalryyValueChanged(int v);
  void slotCannonxValueChanged(int);
  void slotCannonyValueChanged(int v);
  void slotInfantryxValueChanged(int);
  void slotInfantryyValueChanged(int v);
  
private:
  void initSpritesButtonsWith(const Country* country);
    
  void openSkin(const QString& skinDir);
  void initContinentWidgetWith(Continent* continent);
  void initGoalWidgetWith(Goal* continent);
  void initNationalityWidgetWith(Nationality* nationality);
  void updateSprites(SpriteType type);
  
  Country* currentCountry();
  void createPixmapFor(Country* country, SpriteType type);

  // the current saved game file name
  QString m_fileName;

  QAction * m_saveGameAction;

  QGraphicsView* m_mapView;
  Scene* m_mapScene;
  
  SpriteType m_selectedSprite;

  ONU* m_onu;

  KSirkSkinDefinitionWidget* m_skinDefWidget;
  KsirkCountryDefinitionWidget* m_countryDefWidget;
  KsirkContinentDefinitionWidget* m_continentDefWidget;
  KsirkGoalDefinitionWidget* m_goalDefWidget;
  KsirkSpritesDefinitionWidget* m_spritesDefWidget;
  KsirkNationalityDefinitionWidget* m_nationalityDefWidget;
  
  QPushButton* m_flagButton;
  QPushButton* m_infantryButton;
  QPushButton* m_cavalryButton;
  QPushButton* m_cannonButton;
  QPushButton* m_anchorButton;
  QPushButton* m_centerButton;
  
  KRecentFilesAction* m_rfa;

  QGraphicsPixmapItem* m_mapItem;

  bool m_updateHighlightPosition;
};

} // closing namespace KsirkSkinEditor

#endif 

