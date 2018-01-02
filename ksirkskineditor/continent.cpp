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
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "continent.h"


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

} // closing namespace KsirkSkinEditor
