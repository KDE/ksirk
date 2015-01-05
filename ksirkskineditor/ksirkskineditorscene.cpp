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
#include "ksirkskineditorscene.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <KDebug>

namespace KsirkSkinEditor
{

Scene::Scene(QObject* parent) :
  QGraphicsScene(parent)
{
  kDebug();
}

Scene::Scene(qreal x, qreal y, qreal width, qreal height, QObject* parent) :
  QGraphicsScene(x, y, width, height, parent)
{
}

Scene::~Scene()
{
  kDebug();
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
//   kDebug() << mouseEvent->scenePos();
  emit position(mouseEvent->scenePos());
  QGraphicsScene::mouseMoveEvent( mouseEvent );
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
//   kDebug() << mouseEvent->scenePos();
  emit pressPosition(mouseEvent->scenePos());
  QGraphicsScene::mousePressEvent( mouseEvent );
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
//   kDebug() << mouseEvent->scenePos();
  emit releasePosition(mouseEvent->scenePos());
  QGraphicsScene::mouseReleaseEvent( mouseEvent );
}

void Scene::dropEvent ( QGraphicsSceneDragDropEvent * event )
{
  kDebug() << event->scenePos();
  QGraphicsScene::dropEvent(event);
}

} // closing namespace


