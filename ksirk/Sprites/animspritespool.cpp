/* This file is part of KsirK.
   Copyright (C) 2007 Gael de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/*  begin                : Thu Feb 22 2007  */

#include "animspritespool.h"
#include "animsprite.h"

#include "ksirk_debug.h"

namespace Ksirk
{


AnimSpritePool* AnimSpritePool::m_pool = nullptr;

AnimSpritePool::AnimSpritePool() 
{
  connect(&m_timer,&QTimer::timeout,this,&AnimSpritePool::update);
  m_timer.setSingleShot(true);
  m_timer.start(200);
}

const AnimSpritePool& AnimSpritePool::single()
{
  if (m_pool == nullptr)
  {
    m_pool = new AnimSpritePool();
  }
  return *m_pool;
}

AnimSpritePool& AnimSpritePool::changeable() 
{
  if (m_pool == nullptr)
  {
    m_pool = new AnimSpritePool();
  }
  return *m_pool;
}

void AnimSpritePool::addSprite(AnimSprite* sprite)
{
  int index = m_sprites.indexOf(sprite);
  if (index == -1)
  {
    m_sprites.push_back(sprite);
  }
}

void AnimSpritePool::removeSprite(AnimSprite* sprite)
{
  int index = m_sprites.indexOf(sprite);
  if (index != -1)
  {
    m_sprites.removeAt(index);
  }
}

void AnimSpritePool::update() 
{
  qCDebug(KSIRK_LOG) << "AnimSpritePool::update";
  QList<AnimSprite*>::iterator it, it_end;
  it = m_sprites.begin(); it_end = m_sprites.end();
  for (; it != it_end; it++)
  {
    (*it)->animate();
  }
  m_timer.start(200);
}


} // closing namespace Ksirk

#include "moc_animspritespool.cpp"
