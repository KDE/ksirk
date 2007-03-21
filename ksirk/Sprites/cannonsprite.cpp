/***************************************************************************
                          cannonsprite.cpp  -  description
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

#include "cannonsprite.h"

#include "GameLogic/country.h"

namespace Ksirk {


CannonSprite::CannonSprite(const QString &imgPath,
        BackGnd* aBackGnd, unsigned int nbFrames, unsigned int nbDirs, 
        unsigned int visibility) :
    ArmySprite(imgPath, aBackGnd, nbFrames, nbDirs, visibility)
{
}

void CannonSprite::setupTravel(
  GameLogic::Country* src, 
  GameLogic::Country* dest, 
  const QPointF* dpi)
{
  if (dpi == 0) 
  {
    AnimSprite::setupTravel(src, dest, 
      src->pointCannon(), dest-> pointCannon());
  }
  else 
  {
    AnimSprite::setupTravel(src, dest, src->pointCannon(), *dpi);
  }
}

};
