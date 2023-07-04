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

#include "mainMenu.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/onu.h"
#include "kgamewin.h"


#include <KLocalizedString>

mainMenu::mainMenu(Ksirk::KGameWindow* game, QWidget* parent) : QWidget(parent)
{
  qCDebug(KSIRK_LOG);
  setupUi(this);

  // Load image
  QString imageFileName;
  QPixmap imag1, imag2;
  
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, "skins/default/Images/logoRight.png");
  imag1.load(imageFileName);
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, "skins/default/Images/logoLeft.png");
  imag2.load(imageFileName);
  
  lImage1->setPixmap(imag1.scaled(100,100,Qt::KeepAspectRatioByExpanding));
  lImage2->setPixmap(imag2.scaled(100,100,Qt::KeepAspectRatioByExpanding));

  connect(pbNewGame, &QAbstractButton::clicked, game, &Ksirk::KGameWindow::slotNewGame);
  connect(pbJabberGame, &QAbstractButton::clicked, game, &Ksirk::KGameWindow::slotJabberGame);
  connect(pbNewSocketGame, &QAbstractButton::clicked, game, &Ksirk::KGameWindow::slotNewSocketGame);
  connect(pbJoin, &QAbstractButton::clicked, game, &Ksirk::KGameWindow::slotJoinNetworkGame);
  connect(pbLoad, &QAbstractButton::clicked, game, &Ksirk::KGameWindow::slotOpenGame);
  connect(pbQuit, &QAbstractButton::clicked, game, &QWidget::close);
}

void mainMenu::init(Ksirk::GameLogic::ONU* /*theWorld*/)
{
}

#include "moc_mainMenu.cpp"
