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
// include files for kde games

class QAction;
class KToolBar;
class KDialog;

class QEvent;
class QDockWidget;
class QGraphicsScene;
class QGraphicsView;

namespace Phonon
{
  class MediaObject;
}

namespace KsirkSkinEditor
{

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
  enum SpriteType
  {
    None,
    Flag,
    Infantry,
    Cavalry,
    Cannon
  };
  
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

  void slotPosition(const QPointF&);
  void slotPressPosition(const QPointF&);
  
private:
  void saveXml(std::ostream& xmlStream);

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

  QAction* m_saveGameAction;

  QGraphicsView* m_mapView;
  QGraphicsScene* m_mapScene;
  
  SpriteType m_selectedSprite;

  ONU* m_onu;
};

} // closing namespace KsirkSkinEditor

#endif 

