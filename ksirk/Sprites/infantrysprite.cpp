/***************************************************************************
                          infantrysprite.cpp  -  description
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

#include "infantrysprite.h"
#include "skinSpritesData.h"

#include "GameLogic/country.h"

namespace Ksirk {

InfantrySprite::InfantrySprite(double zoom,
                              BackGnd* aBackGnd,
                              unsigned int visibility) :
                              ArmySprite(
        "infantry",
        Sprites::SkinSpritesData::single().intData("infantry-width"),
        Sprites::SkinSpritesData::single().intData("infantry-height"),
        Sprites::SkinSpritesData::single().intData("infantry-frames"),
        Sprites::SkinSpritesData::single().intData("infantry-versions"),
        zoom, aBackGnd, visibility)
{
}


  InfantrySprite::InfantrySprite(const QString &svgid,
                                  unsigned int width,
                                  unsigned int height,
                                  unsigned int nbFrames,
                                  unsigned int nbDirs,
                                  double zoom,
                                  BackGnd* aBackGnd,
                                  unsigned int visibility) :
                                  ArmySprite(svgid, width, height, nbFrames, nbDirs, zoom, aBackGnd, visibility)
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
  if (dpi ==nullptr) 
  {
    AnimSprite::setupTravel(src, dest, src->pointInfantry()*m_zoom, dest-> pointInfantry()*m_zoom);
  }
  else 
  {
    AnimSprite::setupTravel(src, dest, src->pointInfantry()*m_zoom, *dpi);
  }
}

}
