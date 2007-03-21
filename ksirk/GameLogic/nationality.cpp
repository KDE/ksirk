/***************************************************************************
                          nationality.cpp  -  description
                             -------------------
    begin                : sam ao 31 2002
    copyright            : (C) 2002 by Gael de Chalendar
    email                : Gael.de.Chalendar@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "nationality.h"

#include <iostream>

namespace Ksirk
{

namespace GameLogic
{

Nationality::Nationality(const QString &myName, const QString &myFlag, 
    const QString& leaderName) :
  m_name(myName), m_flagFileName(myFlag), m_leaderName(leaderName)
{
}


/** Read property of QString flagFileName. */
const QString& Nationality::flagFileName() const
{
    return m_flagFileName;
}

void Nationality::saveXml(std::ostream& xmlStream)
{
  QString name = m_name.toUtf8();
  name = name.replace("&","&amp;");
  name = name.replace("<","&lt;");
  name = name.replace(">","&gt;");
  xmlStream << "<nationality name=\"" << m_name.toUtf8().data() << "\" flag=\"" << m_flagFileName.toUtf8().data() << "\" />" << std::endl;  
}

const QString& Nationality::name() const
{
  return m_name;
}

const QString& Nationality::leaderName() const
{
  return m_leaderName;
}

} // closing namespace GameLogic
} // closing namespace Ksirk
