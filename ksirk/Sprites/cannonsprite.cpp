/***************************************************************************
                          cannonsprite.cpp  -  description
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

#include "cannonsprite.h"
#include "skinSpritesData.h"

#include "GameLogic/country.h"

namespace Ksirk {

CannonSprite::CannonSprite(double zoom,
                              BackGnd* aBackGnd,
                              unsigned int visibility) :
                              ArmySprite(
        "cannon",
        Sprites::SkinSpritesData::single().intData("cannon-width"),
        Sprites::SkinSpritesData::single().intData("cannon-height"),
        Sprites::SkinSpritesData::single().intData("cannon-frames"),
        Sprites::SkinSpritesData::single().intData("cannon-versions"), zoom, aBackGnd, visibility)
{
}



CannonSprite::CannonSprite(const QString &svgid,
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

void CannonSprite::setupTravel(
  GameLogic::Country* src, 
  GameLogic::Country* dest, 
  const QPointF* dpi)
{
  qCDebug(KSIRK_LOG) << src->name() << dest->name() << (void*)dpi << (dpi==0?QPointF():*dpi);
  if (dpi == 0) 
  {
    AnimSprite::setupTravel(src, dest, 
                             src->pointCannon()*m_zoom, dest-> pointCannon()*m_zoom);
  }
  else 
  {
    AnimSprite::setupTravel(src, dest, src->pointCannon()*m_zoom, *dpi);
  }
}

}
