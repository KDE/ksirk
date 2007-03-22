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

#include "player.h"

#include "kdebug.h"

namespace Ksirk
{

namespace GameLogic
{

/** The constructor-initializer */
Continent::Continent (const QString &myName, const std::vector<Country*>& myCountries, int myBonus,
                     unsigned int id) :
  m_members(myCountries), m_name(myName), bonus(myBonus), m_id(id)
{
  for ( uint i = 0; i < myCountries.size(); ++i )
  {    
    myCountries.at(i)->setContinent(this);
  }
}

Continent::~Continent()
{
}

/** Read property of QList<Country> m_members. */
const std::vector<Country*>& Continent::getMembers() const
{
//   kDebug() << "There is " << m_members.size() << " countries in " << name() << endl;
  return m_members;
}

/** Read property of QString name. */
const QString& Continent::name() const
{
    return m_name;
}

/** Read property of int bonus. */
const int& Continent::getBonus() const
{
    return bonus;
}

/**
  * Returns the player that owns all the countries of this continent. 0 if none
  */
const Player* Continent::owner() const
{
//    kDebug() << "Continent::owner for "  << m_name;
    /** The owner of the first country is the owner if there is any one*/
  std::vector<Country*>::const_iterator it = m_members.begin();
  const Country* firstOne = *(it);
  const Player* owner = firstOne-> owner();
//    kDebug() << "\t"  << firstOne-> name()  << " is owned by "  << owner-> name();

  for (it++; it != m_members.end(); it++)
  {
//        kDebug() << "\t"  << c-> name() << " is owned by "  << c-> owner()-> name();
    /** if the owner of the current country is not the owner of th first
      * one, then there is two different owners and the function should
      * return 0
      */
    if ((*it)-> owner() != owner)
    {
//            kDebug() << "Nobody owns " << m_name << endl;
      return 0;
    }
  }
    /** There is only one owner for all hte countries ; lets return it */
//    kDebug() << "The owner of " << m_name << " is "  << owner-> name();
  return owner;
}

void Continent::saveXml(std::ostream& xmlStream)
{
  QString name = m_name.toUtf8();
  name = name.replace("&","&amp;");
  name = name.replace("<","&lt;");
  name = name.replace(">","&gt;");
  xmlStream << "<continent name=\""<<name.toUtf8().data()<<"\" bonus=\""<<bonus<<"\" >" << std::endl;
  std::vector< Country* >::const_iterator it, it_end;
  it = m_members.begin(); it_end = m_members.end();
  for (; it != it_end; it++)
  {
    (*it)->saveXml(xmlStream);
  }

  xmlStream << "</continent>" << std::endl;
}

/** Returns the list of countries owned by @ref player */
std::vector<Country*> Continent::countriesOwnedBy(const Player* player) 
{
  std::vector<Country*> res;
  for ( uint i = 0; i < m_members.size(); ++i )
  {    
    if ( m_members.at(i)->owner() == player )
    {
      res.push_back(m_members.at(i));
    }
  }
  kDebug() << player->name() << " owns " << res.size() << " in " << name() << endl;
  return res;
}

} // closing namespace GameLogic
} // closing namespace Ksirk
