/***************************************************************************
                          continent.cpp  -  description
                             -------------------
    begin                : sam sep 7 2002
    copyright            : (C) 2002 by Gael de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "continent.h"


#include "kdebug.h"

namespace KsirkSkinEditor
{

/** The constructor-initializer */
Continent::Continent (const QString &myName, const QList<Country*>& myCountries, const int myBonus/*,
                     unsigned int id*/) :
  m_members(myCountries), m_name(myName), m_bonus(myBonus)/*, m_id(id)*/
{
  for ( int i = 0; i < myCountries.size(); ++i )
  {    
    myCountries.at(i)->setContinent(this);
  }
}

Continent::~Continent()
{
}

// void Continent::saveXml(std::ostream& xmlStream)
// {
//   QString name = m_name.toUtf8();
//   name = name.replace("&","&amp;");
//   name = name.replace("<","&lt;");
//   name = name.replace(">","&gt;");
//   xmlStream << "<continent name=\""<<name.toUtf8().data()<<"\" bonus=\""<<m_bonus<<"\" >" << std::endl;
//   QList< Country* >::const_iterator it, it_end;
//   it = m_members.begin(); it_end = m_members.end();
//   for (; it != it_end; it++)
//   {
//     (*it)->saveXml(xmlStream);
//   }
// 
//   xmlStream << "</continent>" << std::endl;
// }

} // closing namespace KsirkSkinEditor
