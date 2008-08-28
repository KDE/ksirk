/* This file is part of KsirK.
   Copyright (C) 2001-2008 Gael de Chalendar <kleag@free.fr>

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

/*  begin                : Sat May 17 2008  */

#include "arrowsprite.h"

#include <kdebug.h>

#include <QBrush>

namespace Ksirk
{

namespace Sprites
{

ArrowSprite::ArrowSprite(Qt::ArrowType type, QGraphicsItem * parent) :
    QGraphicsPolygonItem(parent)
{
  QPolygonF polygon;
  switch (type)
  {
  case Qt::UpArrow:
    polygon << QPointF(0,0) << QPointF(80,0) << QPointF(40,-20) << QPointF(0,0);
  break;
  case Qt::DownArrow:
    polygon << QPointF(0,0) << QPointF(80,0) << QPointF(40,20) << QPointF(0,0);
  break;
  case Qt::LeftArrow:
    polygon << QPointF(0,0) << QPointF(0,80) << QPointF(-20,40) << QPointF(0,0);
  break;
  case Qt::RightArrow:
    polygon << QPointF(0,0) << QPointF(0,80) << QPointF(20,40) << QPointF(0,0);
  break;
  default: ;
  }
  setBrush(Qt::black);
  setActive(false);
  setPolygon(polygon);
}

ArrowSprite::~ArrowSprite()
{
}

void ArrowSprite::setActive(bool value)
{
  int alpha = (value?128:64);
  kDebug() << value << alpha;
  QBrush b = brush();
  QColor color = b.color();
  color.setAlpha(alpha);
  kDebug() << color.alpha();
  b.setColor(color);
  setBrush(b);
}

} // closing namespace Sprites
} // closing namespace Ksirk
