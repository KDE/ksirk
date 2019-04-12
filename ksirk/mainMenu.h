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

/* begin                : Fri  21 2007 */

#ifndef MAINMENU_H
#define MAINMENU_H

#include "ui_mainMenu.h"

#include "KsirkGlobalDefinitions.h"

#include <QWidget>


namespace Ksirk
{
  class KGameWindow;
  namespace GameLogic
  {
    class ONU;
  }
}

/**
  * The mainMenu class is the widget displayed in the main window
  */
class mainMenu : public QWidget, public Ui::MainMenu
{
  Q_OBJECT

public:
  mainMenu(Ksirk::KGameWindow* game, QWidget* parent = 0);

  ~mainMenu() override {}

  void init(Ksirk::GameLogic::ONU* theWorld);
};


#endif
