/* This file is part of KsirK.
 *   Copyright (C) 2005-2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>
 *
 *   KsirK is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 */

#include "KMessageParts.h"
#include "kdebug.h"

#include <stdexcept>
#include <QPixmap>

namespace Ksirk {

namespace GameLogic {

KMessageParts::KMessageParts()
{
}


KMessageParts::~KMessageParts()
{
}

KMessageParts& KMessageParts::operator<<(const QString& text)
{
  m_strings.push_back(text);
  m_pixmaps.push_back(QPixmap());
  m_order.push_back(Text);
  return *this;
}

KMessageParts& KMessageParts::operator<<(const QPixmap& pixmap)
{
  m_strings.push_back("");
  m_pixmaps.push_back(pixmap);
  m_order.push_back(Pixmap);
  return *this;
}

KMessageParts& KMessageParts::operator>>(QString& s)
{
  if (nextIsText())
  {
    s = m_strings.front();
    m_strings.pop_front();
    m_pixmaps.pop_front();
    m_order.pop_front();
    return *this;
  }
  else
  {
    throw std::runtime_error("Next is not text");
  }
  return *this;
}

KMessageParts& KMessageParts::operator>>(QPixmap& p)
{
  if (nextIsPixmap())
  {
    p = m_pixmaps.front();
    m_pixmaps.pop_front();
    m_strings.pop_front();
    m_order.pop_front();
    return *this;
  }
  else
  {
    throw std::runtime_error("Next is not pixmap");
  }
  return *this;
}

bool KMessageParts::nextIsText()
{
  return (m_order.front() == Text);
}

bool KMessageParts::nextIsPixmap()
{
  return (m_order.front() == Pixmap);
}

bool KMessageParts::empty()
{
  return m_order.empty();
}

unsigned int KMessageParts::size()
{
  return m_order.size();
}
  
void KMessageParts::clear()
{
  m_strings.clear();
  m_pixmaps.clear();
  m_order.clear();
}

KMessageParts::iterator KMessageParts::begin()
{
  iterator it;
  it.m_pixmaps_it = m_pixmaps.begin();
  it.m_strings_it = m_strings.begin();
  it.m_order_it =  m_order.begin();
  return it;
}

KMessageParts::iterator KMessageParts::end()
{
  iterator it;
  it.m_pixmaps_it = m_pixmaps.end();
  it.m_strings_it = m_strings.end();
  it.m_order_it =  m_order.end();
  return it;
}


}

}
