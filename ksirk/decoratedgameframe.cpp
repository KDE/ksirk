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

#include <qmessagebox.h>
#include <qcursor.h>
#include <qsize.h>
#include <QEvent>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>

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
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setCacheMode(QGraphicsView::CacheBackground);
  setMinimumSize(200,100);
  setMaximumSize(mapW,mapH);
  updateGeometry(); 
  setMouseTracking(true);
}

DecoratedGameFrame::~DecoratedGameFrame()
{
}

void DecoratedGameFrame::mouseMoveEvent ( QMouseEvent * event )
{
//   kDebug() << "DecoratedGameFrame::mouseMoveEvent" << endl;
  event->ignore();
}

QSize DecoratedGameFrame::sizeHint() const
{
  kDebug() << k_funcinfo << " " << m_mapW << "/" << m_mapH << endl;
  return QSize(m_mapW, m_mapH);
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

}

#include "decoratedgameframe.moc"
