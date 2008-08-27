/* This file is part of KsirK.
   Copyright (C) 2008 Guillaume Pelouas <pelouas@hotmail.fr>

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

   FightArena::FightArena(QWidget* parent, unsigned int mapW, unsigned int mapH,
      QGraphicsScene* sceneArena,ONU* onuObject, GameAutomaton* automaton):
   QGraphicsView(parent),
   m_scene(sceneArena),
   m_onu(onuObject),
   m_automaton(automaton)
   {
      kDebug();
      
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
                                      QPointF(0,0));
      // create the second country of the arena
      m_countryDefense = new Country(automaton,
                                      "",
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0),
                                      QPointF(0,0));
      // make the two arena countrys neighbours
      QList<Country*> arenaAttackNeighbours;
      arenaAttackNeighbours.insert(arenaAttackNeighbours.begin(), m_countryDefense);
      m_countryAttack->neighbours(arenaAttackNeighbours);
      QList<Country*> arenaDefenseNeighbours;
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
    kDebug();
     delete m_countryAttack; m_countryAttack = 0;
     delete m_countryDefense; m_countryDefense= 0;
     delete m_bgImage; m_bgImage = 0;
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
     kDebug();
     // new size
     int width = m_automaton->game()->centralWidget()->width();
     int height = m_automaton->game()->centralWidget()->height();

     // resize the background image
     QPixmap pix;
     pix = m_bgImage->scaled(width,height);
     // and put the image
     bg->setPixmap(pix);

     // resize the widget
     m_scene->setSceneRect ( 0, 0, width, height);
     setMaximumSize(width, height);

     // new zoom
     double newZ = height/280;
     m_onu->setZoomArena(newZ);

     kDebug() << "Hi";
     // re-place the anchor point of the two countries
     m_countryAttack->anchorPoint(QPointF((width/4)/newZ,(height/2)/newZ));
     m_countryAttack->centralPoint(QPointF((width/4)/newZ,(height/2)/newZ));
     m_countryAttack->pointFlag(QPointF((width/7)/newZ,(height/7)/newZ));
     m_countryAttack->pointCannon(QPointF((2*width/18)/newZ,(height/2)/newZ));
     m_countryAttack->pointCavalry(QPointF((4*width/18)/newZ,(2*height/5)/newZ));
     m_countryAttack->pointInfantry(QPointF((6*width/18)/newZ,(3*height/5)/newZ));

     m_countryDefense->anchorPoint(QPointF((3*width/4)/newZ,(height/2)/newZ));
     m_countryDefense->centralPoint(QPointF((3*width/4)/newZ,(height/2)/newZ));
     m_countryDefense->pointFlag(QPointF((6*width/7)/newZ,(height/7)/newZ));
     m_countryDefense->pointCannon(QPointF((16*width/18)/newZ,(height/2)/newZ));
     m_countryDefense->pointCavalry(QPointF((14*width/18)/newZ,(3*height/5)/newZ));
     m_countryDefense->pointInfantry(QPointF((12*width/18)/newZ,(2*height/5)/newZ));

     kDebug() << "Ho";
     // create the arena countries with the originals
     m_countryAttack->copyForArena(countryA);
     m_countryDefense->copyForArena(countryD);
     
     kDebug() << "Done";
   }

}

#include "fightArena.moc"
