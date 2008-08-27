/* This file is part of KsirK.
   Copyright (C) 2002-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef KSIRKSKINEDITORCONTINENT_H
#define KSIRKSKINEDITORCONTINENT_H

#include "country.h"

namespace KsirkSkinEditor
{

/**
  * This class represents a continent of the world. Each country belongs to 
  * a continent. When a player owns all the countries of a continent, he wins 
  * more armies. It is the basic object on which the strategies are based.
  * @author Gael de Chalendar (aka Kleag)
  */
class Continent
{
public:
  /** 
    * The constructor-initializer.
    * @param myName The name of this continent.
    * @param myCountries The countries that will be member of this continent.
    * @param myBonus The bonus of armies at end of turn for the player owning 
    * all this continent.
    * @param id The unique integer id of this continent.
    */
  Continent (const QString &myName, const QList<Country*>& myCountries,
    const int myBonus);

  /** Default destructor. */
  virtual ~Continent();

  /**
    * Read property of m_members, the countries of this continent.
    */
  inline QList<Country*>& members() {return m_members;}

  /** Return the name of this continent. */
  inline const QString& name() const {return m_name;}

  /** 
    * Return the bonus of armies at end of turn for the player owning all this 
    * continent.
    */
  inline int bonus() const {return m_bonus;}
  inline void setBonus(int b) {m_bonus = b;}
  
  /**
    * Saves a XML representation of this continent for game saving purpose
    * @param xmlStream The stream to write on
    */
//   void saveXml(std::ostream& xmlStream);

  //@{
  /** Accessors to the unique integer identifier of this continent. */
/*  inline unsigned int id() const {return m_id;}
  inline unsigned int id() {return m_id;}
  inline void setId(unsigned int id) {m_id = id;}*/
  //@}

private: // Private attributes

  /** This is the list of the countries that forms this continent. This member
    * is constant as it will not change during the game.
    */
  QList<Country*> m_members;

  /** The name of the continent */
  const QString m_name;

  /** The bonus armies got by a user that owns all this continent */
  unsigned int m_bonus;

  /** The unique integer identifier of this continent. */
//   unsigned int m_id;
};

}
#endif
