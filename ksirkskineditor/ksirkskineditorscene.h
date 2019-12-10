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

#ifndef KSIRKSKINEDITORSCENE_H
#define KSIRKSKINEDITORSCENE_H

#include <QGraphicsScene>

class QGraphicsSceneMouseEvent;

namespace KsirkSkinEditor
{

/**
  * This is the main window.
  *
  * @author Gael de Chalendar (aka Kleag)
  * @version $Id: $
  */
class Scene : public QGraphicsScene
{
  Q_OBJECT

public:
  /**
    * 
    */
  explicit Scene(QObject* parent=0);
  Scene (qreal x, qreal y, qreal width, qreal height, QObject* parent=0);
  /**
    *
    */
  ~Scene() override;
    
protected:
  void mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent ) override;
  void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent ) override;
  void mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent ) override;
  void dropEvent ( QGraphicsSceneDragDropEvent * event ) override;
  
Q_SIGNALS:
  void position(const QPointF&);
  void pressPosition(const QPointF&);
  void releasePosition(const QPointF&);
  
public Q_SLOTS:

private Q_SLOTS:

private:
};

} // closing namespace KsirkSkinEditor

#endif 

