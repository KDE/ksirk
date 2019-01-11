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
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "nationality.h"

namespace KsirkSkinEditor
{

Nationality::Nationality(const QString &myName, const QString &myFlag,
    const QString& leaderName) :
  m_name(myName), m_leaderName(leaderName), m_flagFileName(myFlag)
{
}

void Nationality::saveXml(QTextStream& xmlStream)
{
  QString name = m_name;
  name = name.replace(QLatin1String("&"),QLatin1String("&amp;"));
  name = name.replace(QLatin1String("<"),QLatin1String("&lt;"));
  name = name.replace(QLatin1String(">"),QLatin1String("&gt;"));
  xmlStream << "<nationality name=\"" << name << "\" flag=\"" << m_flagFileName << "\" />";
}

}
