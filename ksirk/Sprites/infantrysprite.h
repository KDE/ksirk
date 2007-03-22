/***************************************************************************
                          infantrysprite.h  -  description
                             -------------------
    begin                : 
    copyright            : (C) 2003-2007 by Gaël de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSIRKINFANTRYSPRITE_H
#define KSIRKINFANTRYSPRITE_H

#include "armysprite.h"

namespace Ksirk {

namespace GameLogic
{
  class Country;
}

/**
  * An InfantrySprite is an army sprite that represents 1 army
  * @author Gaël de Chalendar
  */
class InfantrySprite : public ArmySprite
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
  InfantrySprite(const QString &imgPath, 
    BackGnd* aBackGnd, unsigned int nbFrames, unsigned int nbDirs, 
    unsigned int visibility = 200);

  /** The default destructor */
  virtual ~InfantrySprite() {}

  /**
    * This function chooses the approach mode of an infantry sprite towards its
    * destination:
    * if the distance between the origin and the destination is higher than half
    * the size of the map and if the origin and destination countries comunicate,
    * then the sprite should choose an approach by left or right, through the
    * edge of the map.
    */
  void setupTravel(GameLogic::Country* src, GameLogic::Country* dest, 
    const QPointF* dpi=0);

  /**
    * Gets the number of armies represented by an infantry: 1
    * @return the number of armies represented by an infantry: 1
    */
  inline virtual unsigned int nbArmies() const {return m_nbArmies;}

private:
  static const unsigned int m_nbArmies = 1;
};

}

#endif
