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
#include "skinSpritesData.h"

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
    * This simplified constructor allows to create a new @ref FlagSprite with
    * default values for skin elements names
    * @param svgid The id of the SVG element from which to load images, usually
    * the flag's country id
    * @param zoom The current zoom factor
    * @param aBackGnd The background giving info about the world geometry and
    * access to the underlying QGraphicsScene
    */
  FlagSprite(const QString &svgid,
                double zoom,
                BackGnd* aBackGnd) :
              AnimSprite(svgid,
                  Sprites::SkinSpritesData::single().intData("flag-width"),
                  Sprites::SkinSpritesData::single().intData("flag-height"),
                  Sprites::SkinSpritesData::single().intData("flag-frames"),
                  Sprites::SkinSpritesData::single().intData("flag-versions"),
                  zoom, aBackGnd)
  {
    setAnimated();
  }

  /**
    * This constructor allows to create a new @ref FlagSprite whose images are
    * taken from the given file name with the given number of frames and
    * number of look directions
    * @param svgid The id of the SVG element from which to load images
    * @param aBackGnd The background giving info about the world geometry and
    * access to the underlying QGraphicsScene
    * @param nbFrames The number of different frames in this sprite animation, 
    * thus the number of columns in the sprite image
    * @param nbDirs The number of different views on the sprite, 
    * thus the number of rows in the sprite image
    * @param visibility Measures how much this sprite is visible. It gives its
    * Z value on the graphics scene.
    */
  FlagSprite(const QString &svgid,
              unsigned int width,
              unsigned int height,
              unsigned int nbFrames,
              unsigned int nbDirs,
              double zoom,
              BackGnd* aBackGnd) :
              AnimSprite(svgid, width, height, nbFrames, nbDirs, zoom, aBackGnd)
  {
    setAnimated();
  }

  /** The default destructor */
  virtual ~FlagSprite() {}
};

}

#endif
