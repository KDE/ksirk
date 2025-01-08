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

// application specific includes
#include "ksirkskineditorpixmapitem.h"

#include<QGraphicsSceneMouseEvent>

#include "ksirkskineditor_debug.h"

namespace KsirkSkinEditor
{

PixmapItem::PixmapItem(QGraphicsItem* parent) :
      QGraphicsPixmapItem(parent)
{
  qCDebug(KSIRKSKINEDITOR_LOG);
}

PixmapItem::~PixmapItem()
{
  qCDebug(KSIRKSKINEDITOR_LOG);
}

void PixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << event->scenePos();
  QGraphicsItem::mousePressEvent(event);
  Q_EMIT pressed(this,event->scenePos());
}

void PixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << event->scenePos();
  QGraphicsItem::mouseReleaseEvent(event);
  Q_EMIT placed(this, event->scenePos());
}

} // closing namespace

#include "moc_ksirkskineditorpixmapitem.cpp"
