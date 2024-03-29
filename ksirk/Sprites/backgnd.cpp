/***************************************************************************
                          backgnd.cpp  -  description
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001-2006 by Gael de Chalendar (aka Kleag)
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/
 
#include "backgnd.h"
#include "GameLogic/onu.h"

#include <QPixmap>

#include "ksirk_debug.h"

namespace Ksirk
{

using namespace GameLogic;

BackGnd::BackGnd(QGraphicsScene *scene, const GameLogic::ONU* theWorld, bool arena) :
    QGraphicsPixmapItem(nullptr), m_theWorld(theWorld), m_bgIsArena(arena)
{
  scene->addItem(this);
  qCDebug(KSIRK_LOG) << "BackGnd constructor";

  QPixmap pix;
  if (arena == false) {
    pix = theWorld->map();
    // put the image
    setPixmap(pix);
  }

  setZValue(1);
  show();
}

BackGnd::~BackGnd()
{
//   m_frame = 0;
}



}
