/* This file is part of KsirK.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef KSIRK_GAMELOGICEVENTSLISTPROPERTY_H
#define KSIRK_GAMELOGICEVENTSLISTPROPERTY_H

#include<qstring.h>
#include<qpoint.h>
#include<qpair.h>
#include <QList>
namespace Ksirk {

namespace GameLogic {

/**
  * Currently unused. But could be rebirthed in the future.
  * @author Gaël de Chalendar
  * 
  */
// class EventsListProperty : public KGamePropertyList< QPair< QString, QPoint > >
class EventsListProperty : public QList< QPair< QString, QPointF > >
{
public:
  EventsListProperty();

  virtual ~EventsListProperty();

  /**
    * This will read the value of this property from the stream. You MUST
    * overwrite this method in order to use this class
    * @param s The stream to read from
    **/
//   virtual void load(QDataStream& s);
  
  /**
    * Write the value into a stream. MUST be overwritten
    **/
//   virtual void save(QDataStream& s);
  
  /**
    * send a command to advanced properties like arrays
    * @param stream The stream containing the data of the comand
    * @param msgid The ID of the command - see PropertyCommandIds
    * @param isSender whether this client is also the sender of the command
    **/
//   virtual void command(QDataStream &stream, int msgid, bool isSender=false);
  
  void pop_front();
  
  void push_back(const QPair<QString, QPointF>& pair);

private:
//   QByteArray buffer;
  
};

}

}

#endif
