/* This file is part of KsirK.
   Copyright (C) 2001-2010 Gael de Chalendar <kleag@free.fr>
   Copyright (C) 2010 Cyril Tostivint <cyril@tostivint.com>
   
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

/*  begin                : mer jui 11 22:27:28 EDT 2001   */

// application specific includes
#include "InvasionSlider.h"
#include "kgamewin.h"
#include "mainMenu.h"
#include "ksirkConfigDialog.h"
#include "ksirksettings.h"
#include "Sprites/animspritesgroup.h"
#include "Sprites/arrowsprite.h"
#include "GameLogic/aiplayer.h"
#include "GameLogic/aiColsonPlayer.h"
#include "GameLogic/aiplayerio.h"
#include "GameLogic/country.h"
#include "GameLogic/onu.h"
#include "GameLogic/dice.h"
#include "GameLogic/KMessageParts.h"
#include "GameLogic/goal.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/KsirkChatItem.h"
#include "GameLogic/KsirkChatModel.h"
#include "GameLogic/KsirkChatDelegate.h"
#include "SaveLoad/ksirkgamexmlloader.h"
#include "Sprites/backgnd.h"
#include "Dialogs/kwaitedplayersetupdialog.h"
#include "Dialogs/restartOrExitDialogImpl.h"
#include "Dialogs/jabbergameui.h"
#include "im.h"
#include "xmpp_tasks.h"


//include files for QT
#include <QDockWidget>
#include <QTreeView>
#include <QPushButton>
#include <QGridLayout>
#include <QString>
#include <QVBoxLayout>
#include <QMovie>
#include <QUuid>
#include <QHostInfo>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardgameaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <phonon/mediaobject.h>
#include <KPushButton>
#include <kgamepopupitem.h>
#include <kglobal.h>
#include <KStatusBar>
#include <KToolBar>
#include <KAction>
#include <QSvgRenderer>
#include <KDialog>
#include <KAboutData>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kchatdialog.h>
#include <libkdegamesprivate/kgame/kgamechat.h>

#include <sys/utsname.h>

#include <QLabel>
#include <QSlider>


namespace Ksirk
{
using namespace GameLogic;

// port all occurrences of m_accels
// port all occurrences of setBarPos

InvasionSlider::InvasionSlider(KGameWindow* game, GameLogic::Country * attack, GameLogic::Country * defender, InvasionType invasionType):m_game(game)
{
  m_nbLArmy = attack->nbArmies();
  m_nbRArmy = defender->nbArmies();

  m_nbLArmies = new QLabel(QString::number(m_nbLArmy));
  m_nbRArmies = new QLabel(QString::number(m_nbRArmy));

  if (invasionType == Invasion)
  {
    setButtons( KDialog::Ok );
  }
  else if (invasionType == Moving)
  {
    setButtons( KDialog::Cancel | KDialog::Ok );
  }

  QWidget* widget = new QWidget(this);

  //Infantery picture
  InfantrySprite *sprite = new InfantrySprite(1.0, game->backGnd());
  sprite-> setDestination(0);             // Sprite immobile
  QPixmap soldat = sprite->image(0).scaled(24,24,Qt::KeepAspectRatioByExpanding);
  delete sprite;
  
  QLabel * nb = new QLabel();
  nb->setPixmap(soldat);
  nb->setFixedSize(35,35);

  m_invadeSlide = new QSlider(Qt::Horizontal,widget);

  m_invadeSlide->setTickInterval(1);
  m_invadeSlide->setMinimum(0);
  m_invadeSlide->setMaximum(attack->nbArmies()-1);
  m_invadeSlide->setTickPosition(QSlider::TicksBelow);
  m_currentSlideValue = m_invadeSlide->value();

  QGridLayout * wSlideLayout = new QGridLayout(widget);
  QHBoxLayout * center = new QHBoxLayout(); // remove parameter to avoid message "which already has a layout"
  QVBoxLayout * left = new QVBoxLayout();
  QVBoxLayout * right = new QVBoxLayout();
  
  //init. main layout
  if (invasionType == Invasion)
  {
    setWindowTitle(i18n("Invasion"));
    wSlideLayout->addWidget(new QLabel(i18n("You are invading <font color=\"blue\">%1</font> with <font color=\"red\">%2</font>!", defender->i18name(), attack->i18name())),0,0);
    wSlideLayout->addWidget(new QLabel(i18n("<br><i>Choose the number of invading armies.</i>")),1,0);
  }
  else if (invasionType == Moving)
  {
    setWindowTitle(i18n("Moving"));
    wSlideLayout->addWidget(new QLabel(i18n("You are moving armies from <font color=\"red\">%1</font> to <font color=\"blue\">%2</font>!", attack->i18name(), defender->i18name())),0,0);
    wSlideLayout->addWidget(new QLabel(i18n("<br><i>Choose the number of armies to move.</i>")),1,0);
  }

  wSlideLayout->addLayout(center,2,0);
  wSlideLayout->addWidget(m_invadeSlide,3,0);

  //init. center layout
  center->addLayout(left);
  center->addWidget(nb);
  center->addLayout(right);

  //init. left layout
  left->addWidget(new QLabel("<b>"+attack->i18name()+"</b>"),Qt::AlignCenter);
  left->addWidget(m_nbLArmies,Qt::AlignCenter);

  //init. right layout
  right->addWidget(new QLabel("<b>"+defender->i18name()+"</b>"),Qt::AlignCenter);
  right->addWidget(m_nbRArmies,Qt::AlignCenter);

  //val->setText(QString::number(invadeSlide->value()));
  connect(m_invadeSlide,SIGNAL(valueChanged(int)),this,SLOT(slideMove(int)));
  connect(m_invadeSlide,SIGNAL(sliderReleased()),this,SLOT(slideReleased()));
  connect(this,SIGNAL(okClicked()),this,SLOT(slideClose()));
  if (invasionType == Moving)
  {
    connect(this,SIGNAL(cancelClicked()),this,SLOT(slideCancel()));
  }

  setMainWidget(widget);
  
  widget->setLayout(wSlideLayout);
  setWindowModality(Qt::ApplicationModal);
}

void InvasionSlider::slideMove(int v)
{
  kDebug() << v;
  m_nbLArmy = m_nbLArmy-(v-m_currentSlideValue);
  m_nbRArmy = m_nbRArmy+(v-m_currentSlideValue);
  m_nbLArmies->setText(QString::number(m_nbLArmy));
  m_nbRArmies->setText(QString::number(m_nbRArmy));
  update();
  m_currentSlideValue = v;
  
  m_game->automaton()->currentPlayerPlayed(true);
}

void InvasionSlider::slideReleased()
{
  kDebug() << "do nothing";
}

void InvasionSlider::slideClose()
{
  kDebug() << m_currentSlideValue;
  
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << quint32(m_currentSlideValue);
  m_game->automaton()->sendMessage(buffer,Invade);
  
  QByteArray buffer2;
  m_game->automaton()->sendMessage(buffer2,InvasionFinished);
  m_game->automaton()->currentPlayerPlayed(true);
  QPointF point;
  m_game->automaton()->gameEvent("actionNextPlayer", point);
}

void InvasionSlider::slideCancel()
{
  kDebug() << "Move cancel";
}

} // closing namespace Ksirk

#include "InvasionSlider.moc"
