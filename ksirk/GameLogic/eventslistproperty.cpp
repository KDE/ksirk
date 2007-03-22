/**
 C++ Implementation: eventslistproperty

 Description:


 Author: Gael de Chalendar (aka Kleag) <kleag@free.fr>, (C) 2005

 Copyright: See COPYING file that comes with this distribution

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA
 */

#include "eventslistproperty.h"

namespace Ksirk {

namespace GameLogic {

// EventsListProperty::EventsListProperty() : KGamePropertyList< QPair< QString, QPoint > >()
EventsListProperty::EventsListProperty() : QList< QPair< QString, QPointF > >()
{
}


EventsListProperty::~EventsListProperty()
{
}

// void EventsListProperty::load(QDataStream& s)
// {
//   QValueList< QPair< QString, QPoint> > list;
//   while (!s.atEnd())
//   {
//     QString str; QPoint p;
//     s >> str >> p;
//     list.push_back(qMakePair(str,p));
//   }
// //   setValue(list);
// }

  /**
    * Write the value into a stream. MUST be overwritten
    **/
// void EventsListProperty::save(QDataStream& s)
// {
//   QValueList< QPair< QString, QPoint> >::const_iterator it, it_end;
//   it = value().begin(); it_end = value().end();
//   for (; it != it_end; it++)
//   {
//     s << (*it).first << (*it).second;
//   }
// }

  /**
    * send a command to advanced properties like arrays
    * @param stream The stream containing the data of the comand
    * @param msgid The ID of the command - see PropertyCommandIds
    * @param isSender whether this client is also the sender of the command
    **/
// void EventsListProperty::command(QDataStream &stream, int msgid, bool isSender)
// {
//   kDebug() << "EventsListProperty::command " << endl;
//   
//   QString commandName;
//   stream >> commandName;
//   
//   kDebug() << "    command name is " << commandName << endl;
//   if (commandName == "pop_front")
//   {
//     QValueList< QPair< QString, QPoint> > list = value();
//     list.pop_front();
//     setValue(list);
//   }
//   else if (commandName == "push_back")
//   {
//     QString str; QPoint point;
//     stream >> str >> point;
//     QValueList< QPair< QString, QPoint> > list = value();
//     list.push_back(qMakePair(str,point));
//     setValue(list);
//   }
// }

void EventsListProperty::pop_front()
{
//   kDebug() << "EventsListProperty::pop_front " << endl;
QList< QPair< QString, QPointF > >::pop_front();
/*  if (!isEmpty())
  {
    EventsListProperty::iterator it = begin();
    remove(it);
  }*/
//   QByteArray buffer;
/*  QDataStream stream(buffer, QIODevice::ReadWrite);
  stream << QString("pop_front");
  command(stream, 1,true);*/
}

void EventsListProperty::push_back(const QPair<QString, QPointF>& pair)
{
//   kDebug() << "EventsListProperty::push_back " << pair.first << " / " << pair.second << endl;
QList< QPair< QString, QPointF > >::push_back(pair);
//   append(pair);
//   QByteArray buffer;
/*  QDataStream stream(buffer, QIODevice::ReadWrite);
  kDebug() << "1" << endl;
  stream << QString("push_back");
  kDebug() << "2" << endl;
  stream << pair.first;
  kDebug() << "3" << endl;
  stream << pair.second;
  kDebug() << "4" << endl;
  command(stream, 2, true);*/
}

}

}
