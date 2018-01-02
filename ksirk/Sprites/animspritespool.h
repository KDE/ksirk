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

*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
*   02110-1301, USA
*/

/*  begin                : Thu Feb 22 2007  */


#ifndef ANIMSPRITESPOOL_H
#define ANIMSPRITESPOOL_H

#include "KsirkGlobalDefinitions.h"

#include <QTimer>

namespace Ksirk
{

class AnimSprite;

class AnimSpritePool : public QObject
{
  Q_OBJECT
public:
  void addSprite(AnimSprite* sprite);
  void removeSprite(AnimSprite* sprite);

  /** static method to retrive the singleton as const */
  static const AnimSpritePool& single();

  /** static method to retrive the singleton as variable */
  static AnimSpritePool& changeable();

private slots:
  void update();

private:
  AnimSpritePool();
  virtual ~AnimSpritePool() {}

  static AnimSpritePool* m_pool;
  /** All sprites are regularly updated */
  QTimer m_timer;
  QList<AnimSprite*> m_sprites;
};


} // closing namespace Ksirk

#endif // ANIMSPRITESPOOL_H
