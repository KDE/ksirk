/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar <kleag@free.fr>

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

/*  begin                : Thu Jul 19 2001  */

#include "decoratedgameframe.h"
#include "GameLogic/player.h"

#include <QSize>
#include <QEvent>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>

#include <klocale.h>
#include <kdebug.h>
#include <kgame/kgameio.h>
#include <kgame/kplayer.h>

namespace Ksirk
{
using namespace GameLogic;

DecoratedGameFrame::DecoratedGameFrame(QWidget* parent, 
      unsigned int mapW, unsigned int mapH)
  : QGraphicsView(parent), m_mapW(mapW), m_mapH(mapH)
{
  kDebug() << "("<<mapW<<"x"<<mapH<<")" << endl;
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setCacheMode(QGraphicsView::CacheBackground);
  setMinimumSize(200,100);
  setMaximumSize(mapW,mapH);
  updateGeometry(); 
  setMouseTracking(true);

  this->m_parent = parent;
  initMenu ();

  // redirect the mouse move event to the main windows
  connect(this, SIGNAL(mouseMoveEventReceived(QMouseEvent *)), parent, SLOT(mouseMoveEvent(QMouseEvent *)));
}

DecoratedGameFrame::~DecoratedGameFrame()
{
}

void DecoratedGameFrame::mouseMoveEvent ( QMouseEvent * event )
{
//   kDebug() << "DecoratedGameFrame::mouseMoveEvent" << endl;
  emit (mouseMoveEventReceived(event));
  event->ignore();
}

QSize DecoratedGameFrame::sizeHint() const
{
//   kDebug() << " " << m_mapW << "/" << m_mapH << endl;
  return QSize(m_mapW, m_mapH);
}

void DecoratedGameFrame::initMenu ()
{
    this->menu = new QMenu(this);
    
    //QAction* action = KStandardGameAction::quit(this, SLOT(close()), this);
    //m_frame->addAction(action);

    ArenaAction = new QAction(i18n("Arena"), this);
    ArenaAction->setCheckable(true);
    connect(ArenaAction, SIGNAL(triggered()), this, SLOT(arenaState()));
    connect(this, SIGNAL(arenaStateSignal(bool)), this->m_parent, SLOT(slotArena(bool)));

    QAction* Attack1Action = new QAction(i18n("Attack1"), this);
    connect(Attack1Action, SIGNAL(triggered()), this->m_parent, SLOT(slotAttack1()));

    QAction* Attack2Action = new QAction(i18n("Attack2"), this);
    connect(Attack2Action, SIGNAL(triggered()), this->m_parent, SLOT(slotAttack2()));

    QAction* Attack3Action = new QAction(i18n("Attack3"), this);
    connect(Attack3Action, SIGNAL(triggered()), this->m_parent, SLOT(slotAttack3()));

    QAction* QuitAction = new QAction(i18n("Quit Game"), this);
    connect(QuitAction, SIGNAL(triggered()),this->m_parent, SLOT(close()));
		
    menu->addAction(ArenaAction);
    menu->addSeparator();
    menu->addAction(Attack1Action);
    menu->addAction(Attack2Action);
    menu->addAction(Attack3Action);
    menu->addSeparator();
    menu->addAction(QuitAction);
}

void DecoratedGameFrame::contextMenuEvent( QContextMenuEvent * )
{
    menuPoint = QCursor::pos();
    menu->exec(menuPoint);
}

/**
 * This slot is called when a mouse key is pressed. As the mouse is used as
 * input for all players
 * this slot is called to generate a player move out of a mouse input, i.e.
 * it converts a QMouseEvent into a move for the game. We do here some
 * simple nonsense and use the position of the mouse to generate
 * moves containing the keycodes
 */
void DecoratedGameFrame::slotMouseInput(KGameIO *input,QDataStream &stream,QMouseEvent *e,bool *eatevent)
{
  kDebug() << "DecoratedGameFrame::slotMouseInput()" << endl;
  
  KPlayer *player=input->player();
  if (!player->myTurn())
  {
    kDebug() << "Player " << dynamic_cast<Player*>(player)->name() << ": not my turn!" << endl;
    *eatevent=false;
    e->setAccepted(false);
    return;
  }
  kDebug() << "Player " << dynamic_cast<Player*>(player)->name() << " " << e->type() << endl;
  if (e->type() == QEvent::GraphicsSceneMousePress)
  {
    kDebug() << "\tQEvent::MouseButtonPress" << endl;
    if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::LeftButton)
    {
      kDebug() << "\tLeft" << endl;
      stream << QString("actionLButtonDown");
    }
    else if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::RightButton)
    {
      kDebug() << "\tRight" << endl;

      


      stream << QString("actionRButtonDown");
    }
    else
    {
      *eatevent=false;
      e->setAccepted(false);
      return;
    }
  }
  else if (e->type() == QEvent::GraphicsSceneMouseRelease)
  {
    kDebug() << "\tQEvent::MouseButtonRelease" << endl;
    if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::LeftButton)
    {
      kDebug() << "\tLeft" << endl;
      stream << QString("actionLButtonUp");
    }
    else if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::RightButton)
    {
      kDebug() << "\tRight" << endl;
      stream << QString("actionRButtonUp");
    }
    else
    {
      kDebug() << "\tOther:" << e->button() << endl;
      *eatevent=false;
      e->setAccepted(false);
      return;
    }
  }
  else
  {
    *eatevent=false;
    e->setAccepted(false);
    return;
  }
  QPointF newPoint = ((QGraphicsSceneMouseEvent*)e)->scenePos();
  kDebug() << "\tPosition: " << newPoint << endl;
  stream << newPoint;
  *eatevent=true;
  kDebug() << "Mouse input done... eatevent=true" << endl;
}

void DecoratedGameFrame::arenaState()
{
	if (ArenaAction->isChecked())
	{
		ArenaAction->setChecked(true);
		emit(arenaStateSignal(true));
	}
	else
	{
		ArenaAction->setChecked(false);
		emit(arenaStateSignal(false));
	}

	menu->exec(menuPoint);
}

QMenu * DecoratedGameFrame::getContextMenu()
{
	return this->menu;
}
}

#include "decoratedgameframe.moc"
