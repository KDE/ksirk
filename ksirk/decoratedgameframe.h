/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar <kleag@free.fr>

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

/* begin                : Thu Jul 19 2001 */

#ifndef DECORATEDGAMEFRAME_H
#define DECORATEDGAMEFRAME_H

#include "KsirkGlobalDefinitions.h"

#include "GameLogic/gameautomaton.h"

#include <stdlib.h>
#include <qtimer.h>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QMenu>

class KGameIO;

namespace Ksirk
{

class ArenaAction : public QAction
{
  Q_OBJECT
  public:
    ArenaAction(QObject* parent = 0) : QAction(parent), m_arenaEnabled(false) {}
    ArenaAction(const QString& text, QObject* parent = 0) : QAction(text, parent), m_arenaEnabled(false) {}
    inline bool isArenaEnabled() {return m_arenaEnabled;}
    inline void setArenaEnabled(bool value) {m_arenaEnabled = value;}
  private:
    bool m_arenaEnabled;
};


/**
  * The DecoratedGameFrame class is the central widget of the application where
  * all the sprites are displayed. It is linked to its parent widget (the main 
  * window)
 */
class DecoratedGameFrame: public QGraphicsView
{
    Q_OBJECT
    
public:
  /**
    * Creates the frame, its timer and set some parameters
    * @param parent The parent widget, the main window
    * @param mapW The width of the map. Will be the width given in size hint.
    * @param mapH The height of the map. Will be the height given in size hint.
    */
  DecoratedGameFrame(QWidget* parent, unsigned int mapW, unsigned int mapH, GameLogic::GameAutomaton* m_automaton);

  /**
    * Destroy the frame : stops and deletes the timer
    */
  virtual ~DecoratedGameFrame();

  /**
    * Returns the size given to the constructor.
    * @return The size given to the constructor.
    */
  virtual QSize sizeHint() const;

  /**
    * initialisation of the contextual menu
    */
  void initMenu ();
  void initAttackMenu ();
  void initMoveMenu ();

  /**
    * Redefinition of the contextMenuEvent function
    */
  void contextMenuEvent( QContextMenuEvent * );

  /**
    * Getter to the context menu
    */
  QMenu * getContextMenu();
  QMenu * getAttackContextMenu();
  QMenu * getMoveContextMenu();

  /**
    * Getter to the action of the context menu
    */
  QAction* getAttack1Action();
  QAction* getAttack2Action();
  QAction* getAttack3Action();
  QAction* getMove1Action();
  QAction* getMove5Action();
  QAction* getMove10Action();

  void setMenuPoint(QPoint);

  void setIcon();

  void setArenaOptionEnabled(bool option);

public slots:
  /** 
    * Slot connected in the game/document object to catch and
    * process mouse events. This is a KGame function which
    * ends up generating the move for the game.
    * @param input is the KGameIO input object
    * @param stream is the output command stream
    * @param mouse is a mouse event
    * @param eatevent is to be set to true if you processed the event
    *
    */
  void slotMouseInput(KGameIO *input,QDataStream &stream,
			QMouseEvent *mouse, bool *eatevent);
  
  void arenaState();

  void slotDetails();

  void attackAuto();

signals:
  /**
    * Signal that redirect the mouse event to the kmainwindows in
    * order to process the event.
    * @param event is the event received
    */
  void mouseMoveEventReceived(QMouseEvent * event);

  void arenaStateSignal(bool);

protected:

  void mouseMoveEvent (QMouseEvent* event);
  bool viewportEvent (QEvent* event);
  
private:
  unsigned int m_mapW;
  unsigned int m_mapH;
  QWidget* m_parent;
  GameLogic::GameAutomaton* m_automaton;
  QMenu* menu;
  QMenu* attackMenu;
  QMenu* moveMenu;
  ArenaAction* m_arenaAction;
  QAction* AutoAction;
  QAction* detailsAction;
  QAction* goalAction;
  QAction* nextPlayer;
  QAction* Attack1Action;
  QAction* Attack2Action;
  QAction* Attack3Action;
  QAction* Move1Action;
  QAction* Move5Action;
  QAction* Move10Action;
  QAction* QuitAction;
  QPoint menuPoint;
  QPoint detailPoint;
};

}

#endif // DECORATEDGAMEFRAME_H

