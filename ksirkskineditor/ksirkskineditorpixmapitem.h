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

#ifndef KSIRKSKINEDITORPIXMAPITEM_H
#define KSIRKSKINEDITORPIXMAPITEM_H

// include files for Qt
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QPointF>

// include files for KDE

// include files for kde games

class QGraphicsSceneMouseEvent;

namespace KsirkSkinEditor
{
class PixmapItem : public QObject, public QGraphicsPixmapItem
{
Q_OBJECT

public:
  /**
    * Create the window and initializes its members
    */
  explicit PixmapItem(QGraphicsItem* parent=0);
  
  /**
    * Deletes the background and the pool
    */
  ~PixmapItem() override;

Q_SIGNALS:
  void pressed(QGraphicsItem*, const QPointF&);
  void placed(QGraphicsItem*, const QPointF&);
  
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseReleaseEvent (QGraphicsSceneMouseEvent* event) override;
  
};

} // closing namespace KsirkSkinEditor

#endif 

