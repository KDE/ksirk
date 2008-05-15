/***************************************************************************
                          cavalrysprite.cpp  -  description
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

#include "cavalrysprite.h"

#include "skinSpritesData.h"

#include "GameLogic/country.h"

namespace Ksirk {

CavalrySprite::CavalrySprite(double zoom,
                              BackGnd* aBackGnd,
                              unsigned int visibility) :
                              ArmySprite(
        "cavalry",
        Sprites::SkinSpritesData::single().intData("cavalry-width"),
        Sprites::SkinSpritesData::single().intData("cavalry-height"),
        Sprites::SkinSpritesData::single().intData("cavalry-frames"),
        Sprites::SkinSpritesData::single().intData("cavalry-versions"), zoom, aBackGnd, visibility)
{
}

CavalrySprite::CavalrySprite(const QString &svgid,
                              unsigned int width,
                              unsigned int height,
                              unsigned int nbFrames,
                              unsigned int nbDirs,
                              double zoom,
                              BackGnd* aBackGnd,                               unsigned int visibility) :
                              ArmySprite(svgid, width, height, nbFrames, nbDirs, zoom, aBackGnd, visibility)
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
    AnimSprite::setupTravel(src, dest, src->pointCavalry()*m_zoom, dest-> pointCavalry()*m_zoom);
  }
  else 
  {
    AnimSprite::setupTravel(src, dest, src->pointCavalry()*m_zoom, *dpcav);
  }
}


}
