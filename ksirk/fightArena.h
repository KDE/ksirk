/* This file is part of KsirK.
   Copyright (C) 2008 Guillaume Pelouas <pelouas@hotmail.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

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


#ifndef FIGHTARENA_H
#define FIGHTARENA_H

#include "KsirkGlobalDefinitions.h"
#include "GameLogic/onu.h"
#include "GameLogic/country.h"
#include "GameLogic/gameautomaton.h"
#include "Sprites/backgnd.h"

#include <stdlib.h>
#include <qtimer.h>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QPixmap>
#include <kconfig.h>
#include <kconfiggroup.h>

namespace Ksirk
{

namespace GameLogic
{
  class ONU;
  class Country;
  class GameAutomaton;
}

   /**
   * The FightArena class is the widget displayed while fights, and where
   * all the units sprites are displayed. It is linked to its parent 
   * widget (the main window)
   */
   class FightArena: public QGraphicsView
   {
      Q_OBJECT
      
      public:
      /**
      * Creates the frame, its timer and set some parameters
      * @param parent The parent widget, the main window
      * @param mapW The width of the arena. Will be the width given in size hint.
      * @param mapH The height of the arena. Will be the height given in size hint.
      * @param sceneArena where sprites will be added.
      * @param onuObject onuObject for getting map and country information
      * @param automaton automaton for the construction of countries
      */
      FightArena(QWidget* parent, unsigned int mapW, unsigned int mapH, QGraphicsScene* sceneArena, GameLogic::ONU* onuObject, GameLogic::GameAutomaton* automaton);

      /**
      * Destroy the frame : stops and deletes the timer
      */
      ~FightArena() override;
      
      /**
      * Returns the size given to the constructor.
      * @return The size given to the constructor.
      */
      QSize sizeHint() const override;

      /**
      * Initializes the fight arena with the two country which fight each other
      * @param countryA Country attacker
      * @param countryD Country defender
      * @param bg backgroud where to paint the elements
      */
      void initFightArena (GameLogic::Country* countryA, GameLogic::Country* countryD, BackGnd* bg);

      /**
        * Get the attacking arena country
        */
      inline GameLogic::Country* countryAttack () {return m_countryAttack;}

      /**
        * Get the defensing arena country
        */
      inline GameLogic::Country* countryDefense () {return m_countryDefense;}

      private:
      unsigned int m_mapW;
      unsigned int m_mapH;

      GameLogic::Country* m_countryAttack;
      GameLogic::Country* m_countryDefense;
      QGraphicsScene* m_scene;
      GameLogic::ONU* m_onu;
      GameLogic::GameAutomaton* m_automaton;
      QPixmap* m_bgImage;
   };
   
}

#endif // FIGHTARENA_H
