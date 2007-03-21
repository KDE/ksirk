/***************************************************************************
                          backgnd.cpp  -  description
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001-2006 by Gaël de Chalendar (aka Kleag)
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#define KDE_NO_COMPAT
#include "backgnd.h"
#include "GameLogic/onu.h"

#include <QPixmap>

#include <kdebug.h>

namespace Ksirk
{

using namespace GameLogic;

BackGnd::BackGnd(QGraphicsScene *scene, const GameLogic::ONU* theWorld) :
    QGraphicsPixmapItem(0, scene)
{
  kDebug() << "BackGnd constructor" << endl;
  setPixmap(theWorld->map());
  setZValue(1);
  show();
};

BackGnd::~BackGnd()
{
//   m_frame = 0;
};



}
