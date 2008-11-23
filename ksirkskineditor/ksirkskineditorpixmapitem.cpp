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

// application specific includes
#include "ksirkskineditorpixmapitem.h"

#include<QGraphicsSceneMouseEvent>

#include<KDebug>

namespace KsirkSkinEditor
{

PixmapItem::PixmapItem(QGraphicsItem* parent) :
      QGraphicsPixmapItem(parent)
{
  kDebug();
}

PixmapItem::~PixmapItem()
{
  kDebug();
}

void PixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  kDebug() << event->scenePos();
  QGraphicsItem::mousePressEvent(event);
  emit pressed(this,event->scenePos());
}

void PixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  kDebug() << event->scenePos();
  QGraphicsItem::mouseReleaseEvent(event);
  emit placed(this, event->scenePos());
}

} // closing namespace

#include "ksirkskineditorpixmapitem.moc"
