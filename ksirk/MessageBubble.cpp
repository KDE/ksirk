/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar <kleag@free.fr>

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
#include "MessageBubble.h"

#include <QLinearGradient>
#include <QPen>

namespace Ksirk
{
MessageBubble::MessageBubble(
    const QString& message,
    QGraphicsItem * parent,
    quint32 delay) :
  QGraphicsPathItem(parent),
  m_message(message),
  m_delay(delay)
{
  QGraphicsTextItem* textItem = new QGraphicsTextItem(this);
  textItem->setHtml(m_message);
  QRectF textRect = textItem->boundingRect();
  qreal w = textRect.width();
  qreal h = textRect.height();
  
  QPainterPath roundRectPath;
  roundRectPath.moveTo(w, 5.0);
  roundRectPath.arcTo(w-10, 0.0,10.0, 10.0, 0.0, 90.0);
  roundRectPath.lineTo(5.0, 0.0);
  roundRectPath.arcTo(0.0, 0.0, 10.0, 10.0, 90.0, 90.0);
  roundRectPath.lineTo(0.0, h-5);
  roundRectPath.arcTo(0.0, h-10, 10.0, 10.0, 180.0, 90.0);
  roundRectPath.lineTo(w-5, h);
  roundRectPath.arcTo(w-10, h-10, 10.0, 10.0, 270.0, 90.0);
  roundRectPath.closeSubpath();
  setPath(roundRectPath);

  QLinearGradient myGradient;
  QPen myPen;
  setBrush(myGradient);
  setPen(myPen);
}

MessageBubble::~MessageBubble()
{
}


} // closing namespace Ksirk
