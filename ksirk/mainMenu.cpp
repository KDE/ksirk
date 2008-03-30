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

#include "mainMenu.h"

namespace Ksirk
{
  using namespace GameLogic;

  
  mainMenu::mainMenu(QWidget* parent, unsigned int mapW, unsigned int mapH, GameAutomaton* automaton):
  w_parent(parent),
  m_automaton(automaton)
  {
    //setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    //setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setMinimumSize(200,100);
    setMaximumSize(mapW,mapH);
    updateGeometry();

    // Create main Layout
    mainLayout = new QGridLayout(this);

    // Create the differents layout for buttons and label
    QGridLayout * bottomLayout = new QGridLayout();
    QGridLayout * topLayout = new QGridLayout();


    // Create and add the main Layout
    this->setLayout(mainLayout);
    mainLayout->addLayout(bottomLayout, 1, 0, Qt::AlignCenter);
    mainLayout->addLayout(topLayout, 0, 0, Qt::AlignCenter);

    // Creates the buttons and label
    pbNewGame = new QPushButton("New Game");
    pbJoin = new QPushButton("Join");
    pbLoad = new QPushButton("Load");
    pbQuit = new QPushButton("Quit");
    QLabel * lTitle = new QLabel ();
    QLabel * lImage1 = new QLabel();
    QLabel * lImage2 = new QLabel();
    lTitle->setText("<h1>KsirK</h1>");

    // Fixe size of the buttons
    pbNewGame->setFixedSize(400, 100);
    pbJoin->setFixedSize(400, 100);
    pbLoad->setFixedSize(400, 100);
    pbQuit->setFixedSize(400, 100);

    // Load image
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

    // Add buttons and layout
    bottomLayout->addWidget(pbNewGame,0,0);
    bottomLayout->addWidget(pbJoin,1,0);
    bottomLayout->addWidget(pbLoad,2,0);
    bottomLayout->addWidget(pbQuit, 3, 0);
    topLayout->addWidget(lImage2, 0, 0);
    topLayout->addWidget(lTitle, 0, 1);
    topLayout->addWidget(lImage1, 0, 2);

    connect(pbNewGame, SIGNAL(clicked()), this->w_parent, SLOT(slotNewGame()));
    connect(pbJoin, SIGNAL(clicked()), this->w_parent, SLOT(slotJoinNetworkGame()));
    connect(pbLoad, SIGNAL(clicked()), this->w_parent, SLOT(slotOpenGame()));
    connect(pbQuit, SIGNAL(clicked()), this->w_parent, SLOT(close()));
  }

  mainMenu::~mainMenu()
  {
  }

  void mainMenu::slotMainNewGame()
  {
    kDebug()<<"************ New Game ************";
  }
}


