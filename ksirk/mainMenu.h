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

/* begin                : Fri  21 2007 */

#ifndef MAINMENU_H
#define MAINMENU_H

#include "KsirkGlobalDefinitions.h"
#include "GameLogic/onu.h"
#include "GameLogic/country.h"
#include "GameLogic/gameautomaton.h"
#include "Sprites/backgnd.h"

#include <stdlib.h>
#include <QGraphicsView>
#include <QPixmap>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <kstandardgameaction.h>
#include <KAction>

class KGameIO;

namespace Ksirk
{

  namespace GameLogic
  {
    class ONU;
    class Country;
    class GameAutomaton;
  }

  /**
     * The mainMenu class is the widget displayed in the main window
     */

  class mainMenu : public QWidget
  {
    Q_OBJECT

  public:
    mainMenu(QWidget* parent, unsigned int mapW, unsigned int mapH, GameLogic::GameAutomaton* automaton);

    ~mainMenu();

  public slots:
  /**
    * The slots associated to the buttons
    */
  void slotMainNewGame();

  private:
    unsigned int m_mapW;
    unsigned int m_mapH;

    QWidget * w_parent;

    GameLogic::GameAutomaton* m_automaton;
    QPixmap* m_bgImage;

    QGridLayout * mainLayout;

    QPushButton * pbNewGame;
    QPushButton * pbJoin;
    QPushButton * pbLoad;
    QPushButton * pbQuit;
  };

}

#endif