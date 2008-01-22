/* This file is part of KsirK.

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/* begin                : Thu Nov 21 2007 */


#include "fightArena.h"
#include "GameLogic/player.h"

namespace Ksirk
{
   using namespace GameLogic;

   FightArena::FightArena(QWidget* parent, unsigned int mapW, unsigned int mapH, QGraphicsScene* sceneArena,ONU* onuObject):
   m_scene(sceneArena),
   m_onu(onuObject)
   {
      setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      setCacheMode(QGraphicsView::CacheBackground);
      setMinimumSize(200,100);
      setMaximumSize(mapW,mapH);
      updateGeometry();
   }
   
   FightArena::~FightArena()
   {
   }
   
   QSize FightArena::sizeHint() const
   {
      return QSize(m_mapW, m_mapH);
   }

   void FightArena::initFightArena (Country* countryA, Country* countryD)
   {
      this->m_countryAttack = countryA;
      this->m_countryDefense = countryD;
   }

}
