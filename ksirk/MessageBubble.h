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

#ifndef KSIRK_MESSAGEBUBBLE_H
#define KSIRK_MESSAGEBUBBLE_H

// include files for Qt
#include <QGraphicsPathItem>

namespace Ksirk
{

/**
  *
  * @author Gael de Chalendar (aka Kleag)
  * @version $Id:  $
  */
class MessageBubble: public QGraphicsPathItem
{

public:
    
  /**
    * Create the window and initializes its members
    */
  MessageBubble(
    const QString& message,
    QGraphicsItem * parent = 0,
    quint32 delay = 5);
  
  /**
    * Deletes the background and the pool
    */
  virtual ~MessageBubble();
    
private: // Private methods
  QString m_message;
  quint32 m_delay;
};

} // closing namespace Ksirk

#endif // KSIRK_MESSAGEBUBBLE_H

