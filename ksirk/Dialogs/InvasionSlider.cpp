/* This file is part of KsirK.
   Copyright (C) 2001-2010 Gael de Chalendar <kleag@free.fr>
   Copyright (C) 2010 Cyril Tostivint <cyril@tostivint.com>
   
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

/*  begin                : mer jui 11 22:27:28 EDT 2001   */

// application specific includes
#include "InvasionSlider.h"

#include "kgamewin.h"
#include "GameLogic/country.h"
#include "ksirk_debug.h"

//include files for QT
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QDialogButtonBox>

// include files for KDE
#include <KLocalizedString>


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

  QVBoxLayout* dialogLayout = new QVBoxLayout(this);
  QDialogButtonBox* buttonBox = nullptr;

  if (invasionType == Invasion)
  {
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  }
  else if (invasionType == Moving)
  {
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  }

  QWidget* widget = new QWidget(this);

  //Infantery picture
  InfantrySprite *sprite = new InfantrySprite(1.0, game->backGnd());
  sprite-> setDestination(nullptr);             // Sprite immobile
  QPixmap soldat = sprite->image(0);
  soldat = soldat.scaled(QSize(24, 24) * soldat.devicePixelRatio(), Qt::KeepAspectRatioByExpanding);
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

  auto * wSlideLayout = new QGridLayout();
  QHBoxLayout * center = new QHBoxLayout(); // remove parameter to avoid message "which already has a layout"
  QVBoxLayout * left = new QVBoxLayout();
  QVBoxLayout * right = new QVBoxLayout();
  
  //init. main layout
  if (invasionType == Invasion)
  {
    setWindowTitle(i18nc("@title:window", "Invasion"));
    wSlideLayout->addWidget(new QLabel(i18n("You are invading <font color=\"blue\">%1</font> with <font color=\"red\">%2</font>!", defender->i18name(), attack->i18name())),0,0);
    wSlideLayout->addWidget(new QLabel(i18n("<br><i>Choose the number of invading armies.</i>")),1,0);
  }
  else if (invasionType == Moving)
  {
    setWindowTitle(i18nc("@title:window", "Moving"));
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
  connect(m_invadeSlide,&QAbstractSlider::valueChanged,this,&InvasionSlider::slideMove);
  connect(m_invadeSlide,&QAbstractSlider::sliderReleased,this,&InvasionSlider::slideReleased);
  connect(buttonBox,&QDialogButtonBox::accepted,this,&InvasionSlider::slideClose);
  if (invasionType == Moving)
  {
    connect(buttonBox,&QDialogButtonBox::rejected,this,&InvasionSlider::slideCancel);
  }

  widget->setLayout(wSlideLayout);

  dialogLayout->addWidget(widget);
  if (buttonBox)
      dialogLayout->addWidget(buttonBox);
  setWindowModality(Qt::ApplicationModal);
}

void InvasionSlider::slideMove(int v)
{
  qCDebug(KSIRK_LOG) << v;
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
  qCDebug(KSIRK_LOG) << "do nothing";
}

void InvasionSlider::slideClose()
{
  qCDebug(KSIRK_LOG) << m_currentSlideValue;
  
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << quint32(m_currentSlideValue);
  m_game->automaton()->sendMessage(buffer,Invade);
  
  QByteArray buffer2;
  m_game->automaton()->sendMessage(buffer2,InvasionFinished);
  m_game->automaton()->currentPlayerPlayed(true);
  QPointF point;
  m_game->automaton()->gameEvent("actionNextPlayer", point);
  accept();
}

void InvasionSlider::slideCancel()
{
  qCDebug(KSIRK_LOG) << "Move cancel";
  reject();
}

} // closing namespace Ksirk

#include "moc_InvasionSlider.cpp"
