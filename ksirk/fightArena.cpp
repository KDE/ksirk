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

   FightArena::FightArena(QWidget* parent, unsigned int mapW, unsigned int mapH, QGraphicsScene* sceneArena,ONU* onuObject, GameAutomaton* automaton):
   m_scene(sceneArena),
   m_onu(onuObject)
   {
      setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
      setCacheMode(QGraphicsView::CacheBackground);
      setMinimumSize(200,100);
      setMaximumSize(mapW,mapH);
      updateGeometry();

      // create the first country of the arena
      m_countryAttack = new Country(automaton,
                                      "",
                                      QPointF(m_onu->width()/4,m_onu->height()/2),
                                      QPointF(m_onu->width()/4,m_onu->height()/2),
                                      QPointF(m_onu->width()/7,m_onu->height()/7),
                                      QPointF(4*m_onu->width()/16,m_onu->height()/2),
                                      QPointF(5*m_onu->width()/16,3*m_onu->height()/5),
                                      QPointF(5*m_onu->width()/16,4*m_onu->height()/5),
                                      0);
      // create the second country of the arena
      m_countryDefense = new Country(automaton,
                                      "",
                                      QPointF(3*m_onu->width()/4,m_onu->height()/2),
                                      QPointF(3*m_onu->width()/4,m_onu->height()/2),
                                      QPointF(6*m_onu->width()/7,m_onu->height()/7),
                                      QPointF(12*m_onu->width()/16,m_onu->height()/2),
                                      QPointF(11*m_onu->width()/16,3*m_onu->height()/5),
                                      QPointF(11*m_onu->width()/16,4*m_onu->height()/5),
                                      0);
      // make the two arena countrys neighbours
      vector<Country*> arenaAttackNeighbours;
      arenaAttackNeighbours.insert(arenaAttackNeighbours.begin(), m_countryDefense);
      m_countryAttack->neighbours(arenaAttackNeighbours);
      vector<Country*> arenaDefenseNeighbours;
      arenaDefenseNeighbours.insert(arenaDefenseNeighbours.begin(), m_countryAttack);
      m_countryDefense->neighbours(arenaDefenseNeighbours);
   }
   
   FightArena::~FightArena()
   {
   }
   
   QSize FightArena::sizeHint() const
   {
      return QSize(m_mapW, m_mapH);
   }

   /**
     * Init the arena with the two countries engaged
     * @param countryA attacker country
     * @param countryD defender country
     */
   void FightArena::initFightArena (Country* countryA, Country* countryD)
   {
     m_countryAttack->copyForArena(countryA);
     m_countryDefense->copyForArena(countryD);
   }

}
