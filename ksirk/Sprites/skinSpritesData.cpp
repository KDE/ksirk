/***************************************************************************
 *    copyright            : (C) 2004-2007 by Gael de Chalendar
 *    email                : kleag@free.fr
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

#include "skinSpritesData.h"
#include "kdebug.h"

#include <QTextStream>

#include <KLocale>
#include <KMessageBox>

namespace Ksirk{
namespace Sprites {



SkinSpritesData* SkinSpritesData::m_singleton = 0;

SkinSpritesData::SkinSpritesData()
{
}

SkinSpritesData::~SkinSpritesData()
{
  delete m_singleton;
  m_singleton = 0;
}

const SkinSpritesData& SkinSpritesData::single()
{
  if (m_singleton == 0)
  {
    m_singleton = new SkinSpritesData();
    m_singleton->init();
  }
  return *m_singleton;
}

SkinSpritesData& SkinSpritesData::changeable()
{
  if (m_singleton == 0)
  {
    m_singleton = new SkinSpritesData();
    m_singleton->init();
  }
  return *m_singleton;
}

/** setup default values for default skin sprites */
void SkinSpritesData::init()
{
  m_intDatas.clear();
  m_strDatas.clear();
}


const QString& SkinSpritesData::skin() const
{
  return m_skin;
}

void SkinSpritesData::skin(const QString& newSkin)
{
  m_skin = newSkin;
}

int SkinSpritesData::intData(const QString& name) const
{
  QMap<QString, int>::const_iterator it = m_intDatas.find(name);
  if (it == m_intDatas.end())
  {
    QString msg;
    QTextStream(&msg) << i18n("Error - Unknown skin int data: ") << name;
    KMessageBox::error(0, msg, i18n("Fatal Error"));
    exit(1);    
  }
  else
  {
    return *it;
  }
}

const QString& SkinSpritesData::strData(const QString& name) const
{
  QMap<QString, QString>::const_iterator it = m_strDatas.find(name);
  if (it == m_strDatas.end())
  {
    QString msg;
    QTextStream(&msg) << i18n("Error - Unknown skin string data: ") << name;
    KMessageBox::error(0, msg, i18n("Fatal Error"));
    exit(1);    
  }
  else
  {
    return *it;
  }
}
    
void SkinSpritesData::strData(const QString& name, const QString& data)
{
  m_strDatas[name] = data;
}

void SkinSpritesData::intData(const QString& name, int data)
{
  m_intDatas[name] = data;
}
    

} // closing namespace GameLogic
} // closing namespace Ksirk
