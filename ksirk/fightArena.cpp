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
   m_onu(onuObject),
   m_automaton(automaton)
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
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      0);
      // create the second country of the arena
      m_countryDefense = new Country(automaton,
                                      "",
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      0);
      // make the two arena countrys neighbours
      vector<Country*> arenaAttackNeighbours;
      arenaAttackNeighbours.insert(arenaAttackNeighbours.begin(), m_countryDefense);
      m_countryAttack->neighbours(arenaAttackNeighbours);
      vector<Country*> arenaDefenseNeighbours;
      arenaDefenseNeighbours.insert(arenaDefenseNeighbours.begin(), m_countryAttack);
      m_countryDefense->neighbours(arenaDefenseNeighbours);


      // search the background image for the arena
      KConfig config(onuObject->getConfigFileName());
      KConfigGroup onugroup = config.group("onu");
      QString skin = onugroup.readEntry("skinpath");
      QString imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/arena.svg");
      // create the background image
      m_bgImage = new QPixmap(imageFileName);
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
   void FightArena::initFightArena (Country* countryA, Country* countryD, BackGnd* bg)
   {
     // new size
     int width = m_automaton->game()->centralWidget()->width();
     int height = m_automaton->game()->centralWidget()->height();
     if (m_automaton->game()->getRightDialog()->isVisible()) {
       int width = width - m_automaton->game()->getRightDialog()->width();
     }

     // resize the background image
     QPixmap pix;
     pix = m_bgImage->scaled(width,height);
     // and put the image
     bg->setPixmap(pix);

     m_scene->setSceneRect ( 0, 0, width, height);
     setMaximumSize(width, height);

     // re-place the anchor point of the two countries
     m_countryAttack->anchorPoint(QPointF(width/4,height/2));
     m_countryAttack->centralPoint(QPointF(width/4,height/2));
     m_countryAttack->pointFlag(QPointF(width/7,height/7));
     m_countryAttack->pointCannon(QPointF(4*width/16,height/2));
     m_countryAttack->pointCavalry(QPointF(5*width/16,3*height/5));
     m_countryAttack->pointInfantry(QPointF(5*width/16,4*height/5));

     m_countryDefense->anchorPoint(QPointF(3*width/4,height/2));
     m_countryDefense->centralPoint(QPointF(3*width/4,height/2));
     m_countryDefense->pointFlag(QPointF(6*width/7,height/7));
     m_countryDefense->pointCannon(QPointF(12*width/16,height/2));
     m_countryDefense->pointCavalry(QPointF(11*width/16,4*height/5));
     m_countryDefense->pointInfantry(QPointF(11*width/16,3*height/5));

     // create the arena countries with the originals
     m_countryAttack->copyForArena(countryA);
     m_countryDefense->copyForArena(countryD);
   }

}
