/***************************************************************************
                          armysprite.h  -  
                          A sprite representing armies
                             -------------------
    begin                : 
    copyright            : (C) 2003-2007 by Gael de Chalendar
    email                : kleag@free.fr
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
 
#ifndef KSIRKARMYSPRITE_H
#define KSIRKARMYSPRITE_H

#include "animsprite.h"

namespace Ksirk {

/**
  * An ArmySprite is a sprite that represents a certain number of armies
  * @author Gaël de Chalendar (aka Kleag)
  */
class ArmySprite : public AnimSprite
{
public:
  /**
    * This constructor allows to create a new @ref ArmySprite whose images are
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
  ArmySprite(const QString &svgid,
              unsigned int width,
              unsigned int height,
              unsigned int nbFrames,
              unsigned int nbDirs, double zoom,
              BackGnd* aBackGnd,
              unsigned int visibility = 200) :
              AnimSprite(svgid, width, height, nbFrames, nbDirs, zoom, aBackGnd, visibility)
  {
    setStatic();
  }

  /** The default destructor */
  virtual ~ArmySprite() {};

  /**
    * Gets the number of armies represented by this sprite. Concrete subclasses
    * should implement this abstract method.
    */
  virtual unsigned int nbArmies() const = 0 ;
};

}

#endif
