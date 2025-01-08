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

#include "animspritesgroup.h"

#include "ksirk_debug.h"

namespace Ksirk
{

AnimSpritesGroup::AnimSpritesGroup(QObject* target, const char* slot, QObject* parent):
  QObject(parent), AnimSpritesList<AnimSprite>(),
  m_numberArrived(0), m_target(target), m_slot(slot)
{
  qCDebug(KSIRK_LOG);
  connect (this,SIGNAL(arrived(AnimSpritesGroup*)),target,slot);
}

AnimSpritesGroup::~AnimSpritesGroup() 
{
}

void AnimSpritesGroup::changeTarget(QObject* target, const char* slot)
{
  qCDebug(KSIRK_LOG) << (void*)target << slot;
  if (m_target != nullptr)
  {
    disconnect(this,SIGNAL(arrived(AnimSpritesGroup*)),m_target,m_slot);
  }
  m_target = target;
  m_slot = slot;
  connect (this,SIGNAL(arrived(AnimSpritesGroup*)),target,slot);
}

void AnimSpritesGroup::clear()
{
  qCDebug(KSIRK_LOG) << size();
  disconnect(this,SIGNAL(arrived(AnimSpritesGroup*)),m_target,m_slot);
  m_target = nullptr; 
  m_slot = nullptr;
  m_numberArrived = 0;
  hideAndRemoveAll();
  AnimSpritesList<AnimSprite>::clear();
}


void AnimSpritesGroup::addSprite(AnimSprite* sprite)
{
  push_back(sprite);
  qCDebug(KSIRK_LOG) << "now" << size();
  connect(sprite, &AnimSprite::atDestination,this,&AnimSpritesGroup::oneArrived);
  connect(sprite, &AnimSprite::animationFinished,this,&AnimSpritesGroup::oneArrived);
}

void AnimSpritesGroup::oneArrived(AnimSprite* sprite)
{
  m_numberArrived++;
  qCDebug(KSIRK_LOG) << (void*)sprite << ":" << m_numberArrived << " on " << AnimSpritesList<AnimSprite>::size();
  // if 0 is given, then one is count as arrived whithout action. Useful for 
  // non-animated sprites of the group, but ugly solution...
  if (sprite != nullptr)
  {
    sprite->arrival();
  }
  if (m_numberArrived == (unsigned int)AnimSpritesList<AnimSprite>::size())
  {
    Q_EMIT arrived(this);
    m_numberArrived = 0;
  }
}

}

#include "moc_animspritesgroup.cpp"
