/***************************************************************************
                          cavalrysprite.h  -  description
                             -------------------
    begin                : 
    copyright            : (C) 2003-2007 by Gael de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either either version 2
   of the License, or (at your option) any later version.of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/


#ifndef KSIRKCAVALRYSPRITE_H
#define KSIRKCAVALRYSPRITE_H

#include "armysprite.h"

namespace Ksirk {

namespace GameLogic
{
  class Country;
}

/**
  * A CavalrySpritei is an army sprite that represents 5 armies
  * @author Gaël de Chalendar
  */
class CavalrySprite : public ArmySprite
{
public:
  /**
    * This simplified constructor allows to create a new @ref CavalrySprite with
    * default values for svg pool id and skin elements names
    * @param zoom The current zoom factor
    * @param aBackGnd The background giving info about the world geometry and
    * access to the underlying QGraphicsScene
    * @param visibility Measures how much this sprite is visible. It gives its
    * Z value on the graphics scene.
    */
  CavalrySprite(double zoom,
                BackGnd* aBackGnd,
                unsigned int visibility = 200);

  /**
    * This constructor allows to create a new @ref AnimSprite whose images are
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
  CavalrySprite(const QString &svgid,
                 unsigned int width,
                 unsigned int height,
                 unsigned int nbFrames,
                 unsigned int nbDirs,
                 double zoom,
                 BackGnd* aBackGnd,
                 unsigned int visibility=200);

  /** The default destructor */
    ~CavalrySprite() override {}

/**
  * This function chooses the approach mode of a cavalry sprite towards its
  * destination:
  * if the distance between the origin and the destination is higher than half
  * the size of the map and if the origin and destination countries comunicate,
  * then the sprite should choose an approach by left or right, through the
  * edge of the map.
  */
  void setupTravel(GameLogic::Country* src, GameLogic::Country* dest, const QPointF* dpcav=0) override;

  /**
    * Gets the number of armies represented by a cavalry: 5
    * @return the number of armies represented by a cavalry: 5
    */
  inline unsigned int nbArmies() const override {return m_nbArmies;}

private:
    static const unsigned int m_nbArmies = 5;
};

}

#endif
