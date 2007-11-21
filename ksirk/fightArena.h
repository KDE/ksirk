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


#ifndef FIGHTARENA_H
#define FIGHTARENA_H

#include "KsirkGlobalDefinitions.h"

#include <stdlib.h>
#include <QGraphicsView>

class KGameIO;

namespace Ksirk
{

   /**
   * The DecoratedGameFrame class is the widget displayed while fights, and where
   * all the units sprites are displayed. It is linked to its parent widget (the main 
   * window)
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
      */
      FightArena(QWidget* parent, unsigned int mapW, unsigned int mapH);
      
      /**
      * Destroy the frame : stops and deletes the timer
      */
      virtual ~FightArena();
      
      /**
      * Returns the size given to the constructor.
      * @return The size given to the constructor.
      */
      virtual QSize sizeHint() const;
      
      private:
      unsigned int m_mapW;
      unsigned int m_mapH;
   };
   
};

#endif // FIGHTARENA_H
