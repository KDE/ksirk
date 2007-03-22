/***************************************************************************
                          infantrysprite.cpp  -  description
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

#include "infantrysprite.h"

#include "GameLogic/country.h"

namespace Ksirk {

InfantrySprite::InfantrySprite(const QString &imgPath,
        BackGnd* aBackGnd, unsigned int nbFrames, unsigned int nbDirs, unsigned int visibility) :
    ArmySprite(imgPath, aBackGnd, nbFrames, nbDirs, visibility)
{
}

/**
  * This function chooses the approach mode of an infantry sprite towards its
  * destination:
  * if the distance between the origin and the destination is higher than half
  * the size of the map and if the origin and destination countries comunicate,
  * then the sprite should choose an approach by left or right, through the
  * edge of the map.
  */
void InfantrySprite::setupTravel(
    GameLogic::Country* src, 
    GameLogic::Country* dest, 
    const QPointF* dpi)
{
  if (dpi ==0) 
  {
    AnimSprite::setupTravel(src, dest, src->pointInfantry(), dest-> pointInfantry());
  }
  else 
  {
    AnimSprite::setupTravel(src, dest, src->pointInfantry(), *dpi);
  }
}

}
