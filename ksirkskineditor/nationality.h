/***************************************************************************
                          nationality.h  -  description
                             -------------------
    begin                : sat aug 31 2002
    copyright            : (C) 2002-2007 by Gael de Chalendar
    email                : Kleag@free.fr
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

#ifndef KSIRKSKINEDITORNATIONALITY_H
#define KSIRKSKINEDITORNATIONALITY_H

#include <QString>
#include <QTextStream>

namespace KsirkSkinEditor
{

/**
  * The Nationality class stores all what represents a nation identity : name, 
  * flag, etc.
  * Nations in KsirK are not bound to a country, they just represent a player
  * identity.
  * @author Gael de Chalendar (aka Kleag)
  */
class Nationality
{
public:
  /**
    * Constructor
    * @param myName The name of the nationality.
    * @param myFlag The file name of the flag.
    * @param leaderName The name of the nation's leader. Will be used as the 
    * displayed player's name. 
    */
  Nationality(const QString &myName, const QString &myFlag, const QString& leaderName);

  /** Default destructor */
  virtual ~Nationality() {}
  
  /** Read property of QString m_flagFileName. */
  inline const QString& flagFileName() const {return m_flagFileName;}
  inline void setFlagFileName(const QString& f)
  {m_flagFileName = f;}
  
  /** Read property of QString m_name. */
  inline const QString& name() const {return m_name;}
  inline void setName(const QString& n) {m_name = n;}
  
  /** Read property of QString m_leaderName. */
  inline const QString& leaderName() const {return m_leaderName;}
  inline void setLeaderName(const QString& l) {m_leaderName = l;}
  
  /**
    * Saves a XML representation of the nationality for game saving purpose
    * @param xmlStream The stream to write on
    */
  void saveXml(QTextStream& xmlStream);

private: // Private attributes

  /** The nation's name. The name of its associated country in the real world. */
  QString m_name;

  /** The default name given to a player of the nationality */
  QString m_leaderName;

  /** The name of the file containing this nation's flag  */
  QString m_flagFileName;
};

}

#endif // NATIONALITY_H
