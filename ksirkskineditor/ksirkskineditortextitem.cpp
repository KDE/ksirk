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
#include "ksirkskineditortextitem.h"

#include<QGraphicsSceneMouseEvent>

#include<KDebug>

namespace KsirkSkinEditor
{

TextItem::TextItem(QGraphicsItem* parent) :
      QGraphicsTextItem(parent)
{
  kDebug();
}

TextItem::~TextItem()
{
  kDebug();
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  kDebug();
//   setPosition(event->scenePos().x()-width()/2,event->scenePos().y()-height()/2)
  emit pressed(this, event->scenePos());
  QGraphicsItem::mousePressEvent(event);
}

void TextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  kDebug() << event->scenePos();
  emit placed(this, event->scenePos());
  QGraphicsItem::mouseReleaseEvent(event);
}

} // closing namespace

#include "ksirkskineditortextitem.moc"
