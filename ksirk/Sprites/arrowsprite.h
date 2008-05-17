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


#ifndef ARROWSPRITE_H
#define ARROWSPRITE_H

#include "KsirkGlobalDefinitions.h"

#include <QGraphicsPolygonItem>

namespace Ksirk
{

namespace Sprites
{

/**
 * The ArrowSprite objects are displayed at the visible surface boundaries of
 * the map when this one can scroll in this direction
 *
 */
class ArrowSprite : public QGraphicsPolygonItem
{
public:
  ArrowSprite(Qt::ArrowType type, QGraphicsItem * parent = 0);

  /** The default destructor */
  virtual ~ArrowSprite();
};

} // closing namespace Sprites
} // closing namespace Ksirk

#endif // ARROWSPRITE_H
