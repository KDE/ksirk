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
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

#include "cannonsprite.h"

#include "GameLogic/country.h"

namespace Ksirk {


CannonSprite::CannonSprite(const QString &svgid,
        BackGnd* aBackGnd, unsigned int nbFrames, unsigned int nbDirs,
        double zoom, unsigned int visibility) :
    ArmySprite(svgid, aBackGnd, nbFrames, nbDirs, zoom, visibility)
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
                             src->pointCannon()*m_zoom, dest-> pointCannon()*m_zoom);
  }
  else 
  {
    AnimSprite::setupTravel(src, dest, src->pointCannon()*m_zoom, *dpi);
  }
}

}
