/* This file is part of KsirK.
   Copyright (C) 2007 Gaël de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*  begin                : Thu Feb 22 2007  */

#include "animspritesgroup.h"

#include <kdebug.h>

namespace Ksirk
{

AnimSpritesGroup::AnimSpritesGroup(QObject* target, char* slot):
  m_numberArrived(0), m_target(target), m_slot(slot)
{
  kDebug() << "AnimSpritesGroup::AnimSpritesGroup" << endl;
  connect (this,SIGNAL(arrived(AnimSpritesGroup*)),target,slot);
}

AnimSpritesGroup::~AnimSpritesGroup() 
{
}

void AnimSpritesGroup::changeTarget(QObject* target, char* slot)
{
  kDebug() << "AnimSpritesGroup::changeTarget" << endl;
  if (m_target != 0)
  {
    disconnect(this,SIGNAL(arrived(AnimSpritesGroup*)),m_target,m_slot);
  }
  m_target = target;
  m_slot = slot;
  connect (this,SIGNAL(arrived(AnimSpritesGroup*)),target,slot);
}

void AnimSpritesGroup::clear()
{
  kDebug() << "AnimSpritesGroup::clear" << endl;
  disconnect(this,SIGNAL(arrived(AnimSpritesGroup*)),m_target,m_slot);
  m_target = 0; 
  m_slot = 0;
  m_numberArrived = 0;
  AnimSpritesList<AnimSprite>::clear();
}


void AnimSpritesGroup::addSprite(AnimSprite* sprite)
{
  push_back(sprite);
  connect(sprite, SIGNAL(atDestination(AnimSprite*)),this,SLOT(oneArrived(AnimSprite*)));
  connect(sprite, SIGNAL(animationFinished(AnimSprite*)),this,SLOT(oneArrived(AnimSprite*)));
}

void AnimSpritesGroup::oneArrived(AnimSprite* sprite)
{
  m_numberArrived++;
  kDebug() << "AnimSpritesGroup::oneArrived: " << m_numberArrived 
    << " on " << size() << endl;
  // if 0 is given, then one is count as arrived whithout action. Useful for 
  // non-animated sprites of the group, but ugly solution...
  if (sprite != 0)
  {
    sprite->arrival();
  }
  if (m_numberArrived == size())
  {
    emit arrived(this);
    m_numberArrived = 0;
  }
}

}

#include "animspritesgroup.moc"
