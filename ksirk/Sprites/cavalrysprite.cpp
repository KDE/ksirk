/***************************************************************************
                          cavalrysprite.cpp  -  description
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

#include "cavalrysprite.h"

#include "GameLogic/country.h"

namespace Ksirk {

CavalrySprite::CavalrySprite(const QString &imgPath,
        BackGnd* aBackGnd, unsigned int nbFrames, unsigned int nbDirs, unsigned int visibility) :
    ArmySprite(imgPath, aBackGnd, nbFrames, nbDirs, visibility)
{
}

/**
  * This function chooses the approach mode of a cavalry sprite towards its
  * destination:
  * if the distance between the origin and the destination is higher than half
  * the size of the map and if the origin and destination countries comunicate,
  * then the sprite should choose an approach by left or right, through the
  * edge of the map.
  */
void CavalrySprite::setupTravel(
    GameLogic::Country* src, 
    GameLogic::Country* dest, 
    const QPointF *dpcav)
{
  if (dpcav==0) 
  {
    AnimSprite::setupTravel(src, dest, src->pointCavalry(), dest-> pointCavalry());
  }
  else 
  {
    AnimSprite::setupTravel(src, dest, src->pointCavalry(), *dpcav);
  }
}


};
