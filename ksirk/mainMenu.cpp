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

/* begin                : Fri  21 2007 */

#include "mainMenu.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/onu.h"
#include "kgamewin.h"


#include <KLocale>

mainMenu::mainMenu(Ksirk::GameLogic::GameAutomaton* automaton, QWidget* parent) : QWidget(parent)
{
  kDebug();
  
  setupUi(this);
  
  // Load image
  if (automaton->game()->theWorld() != 0)
  {
    KConfig config(automaton->game()->theWorld()->getConfigFileName());
    KConfigGroup onugroup = config.group("onu");
    QString skin = onugroup.readEntry("skinpath");
    QString imageFileName;
    QPixmap imag1, imag2;

    imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/logoRight.png");
    imag1.load(imageFileName);
    imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/logoLeft.png");
    imag2.load(imageFileName);

    lImage1->setPixmap(imag1.scaled(100,100,Qt::KeepAspectRatioByExpanding));
    lImage2->setPixmap(imag2.scaled(100,100,Qt::KeepAspectRatioByExpanding));
  }
  connect(pbNewGame, SIGNAL(clicked()), parent, SLOT(slotNewGame()));
  connect(pbJabberGame, SIGNAL(clicked()), parent, SLOT(slotJabberGame()));
  connect(pbNewSocketGame, SIGNAL(clicked()), parent, SLOT(slotNewSocketGame()));
  connect(pbJoin, SIGNAL(clicked()), parent, SLOT(slotJoinNetworkGame()));
  connect(pbLoad, SIGNAL(clicked()), parent, SLOT(slotOpenGame()));
  connect(pbQuit, SIGNAL(clicked()), parent, SLOT(close()));
}

#include "mainMenu.moc"
