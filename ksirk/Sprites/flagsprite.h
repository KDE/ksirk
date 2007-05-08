/* This file is part of KsirK.
   Copyright (C) 2003-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef KSIRKFLAGSPRITE_H
#define KSIRKFLAGSPRITE_H

#include "animsprite.h"

namespace Ksirk {

class AnimSprite;
/**
  * A FlagSprite is a sprite that represents a flag. It adds no new member but
  * is defined for semantic reasons
  * @author Gaël de Chalendar (aka Kleag)
  */
class FlagSprite : public AnimSprite
{
public:
  /**
    * This constructor allows to create a new @ref AnimSprite whose images are
    * taken from the given file name with the given number of frames and
    * number of look directions
    * @param imgPath The (SVG) file name from which to load images
    * @param aBackGnd The background giving info about the world geometry and 
    * access to the underlying QGraphicsScene
    * @param nbFrames The number of different frames in this sprite animation, 
    * thus the number of columns in the sprite image
    * @param nbDirs The number of different views on the sprite, 
    * thus the number of rows in the sprite image
    * @param visibility Measures how much this sprite is visible. It gives its
    * Z value on the graphics scene.
    */
  FlagSprite(const QString &imgPath, BackGnd* aBackGnd,
              unsigned int nbFrames, unsigned int nbDirs,
              double zoom) :
      AnimSprite(imgPath, aBackGnd, nbFrames, nbDirs, zoom)
  {
    setAnimated();
  }

  /** The default destructor */
  virtual ~FlagSprite() {}
};

}

#endif
