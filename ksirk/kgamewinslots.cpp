/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Ffile:///home/kde-devel/playground/games/ksirk/ksirk/GameLogic/onu.cpp
file:///home/kde-devel/playground/games/ksirk/ksirk/GameLogic/onu.h
ranklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

// application specific includes
#include "kgamewin.h"
#include "ksirksettings.h"
#include "Sprites/backgnd.h"

#include <kaboutapplicationdialog.h>
#include <ktoolbar.h>

#include "GameLogic/gameautomaton.h"
#include "GameLogic/country.h"
#include "GameLogic/onu.h"
#include "GameLogic/goal.h"
#include "SaveLoad/ksirkgamexmlloader.h"
#include "Sprites/animspritesgroup.h"

// STL include files
#include <fstream>

// include files for QT
#include <QCursor>
#include <QScrollBar>

// include files for KDE
#include <kfiledialog.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <KStatusBar>
#include <kcomponentdata.h>
#include <kgamepopupitem.h>

namespace Ksirk
{
using namespace GameLogic;

// void KGameWindow::slotTimerEvent()
void KGameWindow::mouseMoveEvent ( QMouseEvent * event )
{
//   kDebug();
  QString countryName;
  QPoint mousePos;
  QPoint mousePosGlobal;
  QPointF mousePosition;
  Country *mouseLocalisation;

  mousePosGlobal = event->globalPos();
  mousePos = m_frame->mapFromGlobal(mousePosGlobal);
  mousePosition = m_frame-> mapToScene(mousePos); 
  mouseLocalisation = clickIn(mousePosition);
  countryName = (mouseLocalisation) ? mouseLocalisation->name() : "";
  if (mouseLocalisation)
  {
    if (m_mouseLocalisation && m_mouseLocalisation!=mouseLocalisation)
    {
      m_mouseLocalisation->clearHighlighting();
      m_mouseLocalisation = mouseLocalisation;
      mouseLocalisation->highlight(Qt::white, 0.5);
    }
    else if (m_mouseLocalisation == 0)
    {
      m_mouseLocalisation = mouseLocalisation;
      mouseLocalisation->highlight(Qt::white, 0.5);
    }
    if (!countryName.isEmpty())
    {
      QString status1Text = "";
      const Player* player = mouseLocalisation-> owner();
      if (player)
      {
        status1Text = i18n("%1 belongs to %2",countryName,player->name());
      }

      statusBar()-> changeItem(status1Text, ID_STATUS_MSG);
    }
  }
  else
  {
    if (m_mouseLocalisation!=0)
    {
      m_mouseLocalisation->clearHighlighting();
      m_mouseLocalisation = 0;
    }
    statusBar()-> changeItem("", ID_STATUS_MSG); // Reset
  }
  int borderScrollSize = 50;
  if ( (!m_timer.isActive())
    && ( ((mousePos.x() < borderScrollSize) && (mousePos.x() >= 0)
          && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
        || ((mousePos.x() > m_frame-> viewport()->width() -borderScrollSize) &&
          (mousePos.x() <= m_frame-> viewport()->width())
          && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
        || ((mousePos.y() < borderScrollSize) && (mousePos.y() >= 0)
          && (mousePos.x() >= 0) && (mousePos.x() <= m_frame-> viewport()->width()))
        || ((mousePos.y() >  m_frame-> viewport()->height() -borderScrollSize) &&
          (mousePos.y() <=  m_frame-> viewport()->height())
          && (mousePos.x() >= 0) && (mousePos.x() <= m_frame-> viewport()->width()))
    )
  )
  {
    m_timer.start(200);
  }
}

void KGameWindow::evenementTimer()
{        //Appel du Timer toutes les 50 ms
  QPoint mousePos;
  QPointF mousePosition;

//   kDebug() << "KGameWindow::evenementTimer";
  mousePos = QCursor::pos();
  mousePosition =  m_frame-> mapFromGlobal(mousePos);
//   kDebug() << "mousePosition = ( " << mousePosition.x() << " , " << mousePosition.y() << " )";
//   kDebug() << "m_frame = ( " << m_frame-> viewport()->width() << " , " << m_frame-> viewport()->height() << " )";

  bool restart = false;
  int borderScrollSize = 50;
  int borderScrollSpeed = 40;
  if ((mousePosition.x() < borderScrollSize) && (mousePosition.x() >= 0)
      && (mousePosition.y() >= 0) && (mousePosition.y() <= m_frame-> viewport()->height()))
  {
//     kDebug() << "scrollRight";
    m_frame-> horizontalScrollBar()->setValue ( m_frame-> horizontalScrollBar()->value() - borderScrollSpeed);
    restart = true;
  }

  if ((mousePosition.x() > m_frame-> viewport()->width() -borderScrollSize) &&
      (mousePosition.x() <= m_frame-> viewport()->width())
      && (mousePosition.y() >= 0) && (mousePosition.y() <= m_frame-> viewport()->height()))
  {
//     kDebug() << "scrollLeft";
    m_frame-> horizontalScrollBar()->setValue ( m_frame-> horizontalScrollBar()->value() + borderScrollSpeed);
    restart = true;
  }
  if ((mousePosition.y() < borderScrollSize) && (mousePosition.y() >= 0)
      && (mousePosition.x() >= 0) && (mousePosition.x() <= m_frame-> viewport()->width()))
  {
//     kDebug() << "scrollDown";
    m_frame-> verticalScrollBar()->setValue ( m_frame-> verticalScrollBar()->value() - borderScrollSpeed);
    restart = true;
  }
  if ((mousePosition.y() >  m_frame-> viewport()->height() -borderScrollSize) &&
      (mousePosition.y() <=  m_frame-> viewport()->height())
      && (mousePosition.x() >= 0) && (mousePosition.x() <= m_frame-> viewport()->width()))
  {
//     kDebug() << "scrollUp " << m_frame-> verticalScrollBar()->value()
//       << "("<< m_frame-> verticalScrollBar()->minimum()
//       << ", " << m_frame-> verticalScrollBar()->maximum() << ")";
    m_frame-> verticalScrollBar()->setValue ( m_frame-> verticalScrollBar()->value() + borderScrollSpeed);
    restart = true;
  }

  if ( m_animFighters->isEmpty() )
  {
    if ( m_secondCountry && !( m_secondCountry-> owner() ) )
      kError() << m_secondCountry-> name() << " does not belong to anybody !";
    // handling of the AI player actions
    /*if ( !( ( ( currentPlayer() && currentPlayer()-> isAI() )
            || ( ( isMyState(GameLogic::GameAutomaton::WAITDEFENSE)  ) && ( m_secondCountry ) && (m_secondCountry->owner())
                 && ( m_secondCountry->owner()->isAI() ) ) ) ) ) {
      toolBar("gameActionsToolBar")-> show();
    }*/
  }


//    kDebug() << "OUT KGameWindow::evenementTimer";
  if (restart)
  {
//     kDebug() << "restarting timer";
    m_timer.start(200);
  }
}

void KGameWindow::slotLeftButtonDown(const QPointF& point)
{
//   kDebug() << "slotLeftButtonDown";
//   if (currentPlayer() && !(currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionLButtonDown", point);
}

void KGameWindow::slotLeftButtonUp(const QPointF& point)
{
//     kDebug() << "slotLeftButtonUp";
//     if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionLButtonUp", point);
}

void KGameWindow::slotRightButtonDown(const QPointF& point)
{
//   if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionRButtonDown", point);
  return;
}

void KGameWindow::slotRightButtonUp(const QPointF& point)
{
//   if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionRButtonUp", point);
  return;
}

/** @todo Clean exit with memory freeing */
bool KGameWindow::queryExit()
{
//   kDebug() << "Writing skin m_config: " << m_automaton->skin();
  KGlobal::config()->group("skin").writeEntry("skin", m_automaton->skin());
  KGlobal::config()->sync();
  return true;
}

void KGameWindow::slotKey1()
{
  if (    ( isMyState(GameLogic::GameAutomaton::WAITDEFENSE)  ) 
       && ( m_secondCountry ) 
       && ( m_secondCountry->owner())
       && ( !m_secondCountry->owner()->isAI() ) 
       && ( !m_secondCountry->owner()->isVirtual())
     ) 
  {
    slotDefense1();
  }
  else if (    ( isMyState(GameLogic::GameAutomaton::WAIT)  ) 
       && ( !currentPlayer()->isAI() ) 
       && ( !currentPlayer()->isVirtual())
       )
  {
    slotAttack1();
  } 
}

void KGameWindow::slotKey2()
{
  if (    ( isMyState(GameLogic::GameAutomaton::WAITDEFENSE)  ) 
       && ( m_secondCountry ) 
       && ( m_secondCountry->owner())
       && ( !m_secondCountry->owner()->isAI() ) 
       && ( !m_secondCountry->owner()->isVirtual())
     ) 
  {
    slotDefense2();
  }
  else if (    ( isMyState(GameLogic::GameAutomaton::WAIT)  ) 
       && ( !currentPlayer()->isAI() ) 
       && ( !currentPlayer()->isVirtual())
       )
  {
    slotAttack2();
  } 
}

void KGameWindow::slotKey3()
{
  if (    ( isMyState(GameLogic::GameAutomaton::WAIT)  ) 
       && ( !currentPlayer()->isAI() ) 
       && ( !currentPlayer()->isVirtual())
       )
  {
    slotAttack3();
  } 
}

void KGameWindow::slotArena(bool isCheck)
{
	if (isCheck)
	{
		ARENA = true;
		kDebug() << "*******Arena On******" << ARENA;
	}
	else
	{
		ARENA = false;
		kDebug() << "*******Arena Off******" << ARENA;
	}
}

void KGameWindow::slotNewGame()
{
//   kDebug() << "Slot new game: posting event actionNewGame";
//   QPoint point;
  if (actionNewGame())
  {
    m_automaton->state(GameAutomaton::INIT);
  }
//   m_automaton->gameEvent("actionNewGame", point);
}

void KGameWindow::slotOpenGame()
{
  kDebug() << "Slot open game: posting event actionOpenGame";
  QPoint point;
  m_automaton->gameEvent("actionOpenGame", point);
//   actionOpenGame();
}

void KGameWindow::slotSaveGame()
{
  if (m_automaton->isAdmin())
  {
    if (isMyState(GameLogic::GameAutomaton::WAITDEFENSE))
    {
      KMessageBox::sorry(this,
          i18n("Cannot save when waiting for a defense."),
          i18n("KsirK - Cannot save !"));
      return;
    }
    QString fileName = KFileDialog::getSaveFileName (KUrl(), "*.xml", this, i18n("KsirK - Save Game")); 
    if (!fileName.isEmpty())
    {
      std::ofstream ofs(fileName.toUtf8().data());
      saveXml(ofs);
    }
  }
  else
  {
    KMessageBox::sorry(this, i18n("Only game master can save the game in network playing."),i18n("KsirK - Cannot save !"));
  }
}

void KGameWindow::slotRecycling()
{
  QPoint point;
  m_automaton->gameEvent("actionRecycling", point);
}

void KGameWindow::slotRecyclingFinished()
{
  kDebug();
  QPoint point;
  m_rightDock->hide();
  m_automaton->gameEvent("actionRecyclingFinished", point);
}

void KGameWindow::slotNextPlayer()
{
  kDebug();
  QPoint point;
  m_automaton->gameEvent("actionNextPlayer", point);
}

void KGameWindow::slotAttack1()
{
  QPoint point;
  m_automaton->gameEvent("actionAttack1", point);
}

void KGameWindow::slotAttack2()
{
  QPoint point;
  m_automaton->gameEvent("actionAttack2", point);
}

void KGameWindow::slotAttack3()
{
  QPoint point;
  m_automaton->gameEvent("actionAttack3", point);
}

void KGameWindow::slotDefense1()
{
  QPoint point;
  m_automaton->gameEvent("actionDefense1", point);
}

void KGameWindow::slotDefense2()
{
  QPoint point;
  m_automaton->gameEvent("actionDefense2", point);
}

void KGameWindow::slotInvade1()
{
  QPoint point;
  m_automaton->gameEvent("actionInvade1", point);
}

void KGameWindow::slotSimultaneousAttack(int state)
{
  QPoint point;
  
  kDebug() << "slotSimultaneousAttack" << state;

  // Attack
  if (state==0)
  {
  	m_automaton->gameEvent("actionSimultaneousAttackA", point);
  }
  else
  {
	// Defense	
	m_automaton->gameEvent("actionSimultaneousAttackD", point);
  }
}

void KGameWindow::slotInvade5()
{
  QPoint point;
  m_automaton->gameEvent("actionInvade5", point);
}

void KGameWindow::slotInvade10()
{
  QPoint point;
  m_automaton->gameEvent("actionInvade10", point);
}

void KGameWindow::slotInvasionFinished()
{
  QPoint point;
  m_automaton->gameEvent("actionInvasionFinished", point);
}

void KGameWindow::slotRetreat1()
{
  QPoint point;
  m_automaton->gameEvent("actionRetreat1", point);
}

void KGameWindow::slotRetreat5()
{
  QPoint point;
  m_automaton->gameEvent("actionRetreat5", point);
}

void KGameWindow::slotRetreat10()
{
  QPoint point;
  m_automaton->gameEvent("actionRetreat10", point);
}

void KGameWindow::slotMove()
{
  QPoint point;
  m_automaton->gameEvent("actionMove", point);
}

void KGameWindow::slotCancel()
{
  QPoint point;
  m_automaton->gameEvent("actionCancel", point);
}

void KGameWindow::slotDumpGameInformations()
{
//   kDebug() << "Game information : ";
//    kDebug() << "  state : " << GameStateNames[getState()];
//   kDebug() << "  current player : " 
//       << ((currentPlayer()) ? currentPlayer()-> name() : "nobody");
}

void KGameWindow::slotFinishMoves()
{
  kDebug();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  m_automaton->sendMessage(buffer,FinishMoves);
}

void KGameWindow::slotShowAboutApplication()
{
    //kDebug() << "Dans mon About !";

/*  KAboutApplication kAA;
  kAA.exec();*/
  KAboutApplicationDialog dialog(KGlobal::mainComponent().aboutData(), this);
  dialog.exec();
}

void KGameWindow::slotJoinNetworkGame() 
{
  kDebug();
  QPoint point;
  m_automaton->gameEvent("actionJoinNetworkGame", point);
}

void KGameWindow::slotShowGoal()
{
//   kDebug() << "slotShowGoal";
if (currentPlayer() && !currentPlayer()->isVirtual()
    && !currentPlayer()->isAI())
  {
    currentPlayer()->goal().show(GameLogic::Goal::GoalDesc|GameLogic::Goal::GoalAdvance);
  }
}

void KGameWindow::slotChatMessage()
{
  setFocus();
}

void KGameWindow::slotChatFloatButtonPressed()
{
  // change the floating state
  m_bottomDock->setFloating(!m_bottomDock->isFloating());
}

void KGameWindow::slotChatFloatChanged(bool value)
{
  // change the float button image
  if (!m_bottomDock->isFloating()) {
    m_floatChatButton->setIcon(m_upChatFloatPix);
  } else {
    m_floatChatButton->setIcon(m_downChatFloatPix);
  }
}

void KGameWindow::slotChatReduceButton()
{
  kDebug();

  // change the reduce button image, hide or show the chat and the short last message
  if (!m_chatIsReduced)
  {
    reduceChat();
  }
  else
  {
    unreduceChat();
  }
}

void KGameWindow::slotMovingFightersArrived(AnimSpritesGroup* sprites)
{
Q_UNUSED(sprites)
  kDebug();
  m_automaton->movingFigthersArrived();
}

void KGameWindow::slotMovingArmiesArrived(AnimSpritesGroup* sprites)
{
  kDebug();
  AnimSpritesGroup::iterator it, it_end;
  it = sprites->begin(); it_end = sprites->end();
  for (; it != it_end; it++)
  {
    (*it)->hide();
    delete *it;
  }
  sprites->clear();
  int index = m_animSpritesGroups.indexOf(sprites);
  if (index != -1)
  {
    m_animSpritesGroups.removeAt(index);
  }
  delete sprites;
}

void KGameWindow::slotBring(AnimSprite* sprite)
{
	m_automaton->game()->stopExplosion();
}

void KGameWindow::slotMovingArmyArrived(AnimSprite* sprite)
{
  kDebug();
  sprite->setStatic();
  sprite->hide();
  m_automaton->movingArmyArrived(sprite->getDestination(),
    ((ArmySprite*)sprite)->nbArmies());
}

void KGameWindow::slotFiringFinished(AnimSpritesGroup* sprites)
{
Q_UNUSED(sprites)
  kDebug();
  m_automaton->firingFinished();
}

void KGameWindow::slotExplosionFinished(AnimSpritesGroup* sprites)
{
  kDebug();
  while (!sprites->empty())
  {
    AnimSprite* sprite = sprites->front();
    sprites->pop_front();
    sprite->setStatic();
    delete sprite;
  }
  
  if (backGnd()->bgIsArena())
  {
    m_automaton->game()->initCombatBringBackForArena(m_automaton->game()->firstCountry(), m_automaton->game()->secondCountry());
  }

  m_automaton->explosionFinished();
  kDebug()<<"DONE";
}

void KGameWindow::slotZoomIn()
{
  kDebug();
  m_theWorld->applyZoomFactorFast(1.1);					//Call to the fast method (benj)
  m_frame->setMaximumSize(m_theWorld->width(),m_theWorld->height());
  m_scene_world->setSceneRect ( QRectF() );
  m_frame->updateGeometry();
  m_backGnd_world->setPixmap(m_theWorld->map());
  
}

void KGameWindow::slotZoomOut()
{
  kDebug();
  m_theWorld->applyZoomFactorFast(1.0/1.1);				//call to the fast method
  m_frame->setMaximumSize(m_theWorld->width(),m_theWorld->height());
  m_scene_world->setSceneRect ( QRectF() );
  m_frame->updateGeometry();
  m_backGnd_world->setPixmap(m_theWorld->map());
}

void KGameWindow::slotRemoveMessage()
{
  kDebug();
  if (m_message != 0)
  {
    kDebug() << "hiding and deleting";
    m_message->hide();
    delete m_message;
    m_message = 0;
  }
}

void KGameWindow::slideMove(int v)
{
  kDebug();
  if(m_slideReleased) m_previousSlideValue = m_currentSlideValue;
  m_slideReleased = false;
  m_nbLArmy = m_nbLArmy-(v-m_currentSlideValue);
  m_nbRArmy = m_nbRArmy+(v-m_currentSlideValue);
  m_nbLArmies->setText(QString::number(m_nbLArmy));
  m_nbRArmies->setText(QString::number(m_nbRArmy));
  m_currentSlideValue = v;
  m_wSlide->update();

  slideReleased();
}

void KGameWindow::slideReleased()
{
  kDebug();
  m_slideReleased = true;
  m_currentSlideValue = m_previousSlideValue;
  QByteArray* buffer;

  int units=0; 
  if(m_invadeSlide->value()!=m_currentSlideValue)
  {
    units = m_invadeSlide->value() - m_currentSlideValue;
  }
  
  if (units<0)
  {
    units*=(-1);
  }
  int reste10 = units%10;
  int reste5;
  for(int i=0;i<(units-reste10)/10;i++)
  {
    
    buffer = new QByteArray();
    QDataStream stream(buffer, QIODevice::WriteOnly);
    stream << quint32(10);
    if (m_invadeSlide->value()>m_currentSlideValue)
    {
    	automaton()->sendMessage(*buffer,Invade);
    }
    else
    {
        automaton()->sendMessage(*buffer,Retreat);
    }
  }

  reste5 = reste10%5;
  for(int i=0;i<(reste10-reste5)/5;i++)
  {
    buffer = new QByteArray();
    QDataStream stream(buffer, QIODevice::WriteOnly);
    stream << quint32(5);
    if (m_invadeSlide->value()>m_currentSlideValue)
    {
    	automaton()->sendMessage(*buffer,Invade);
    }
    else
    {
        automaton()->sendMessage(*buffer,Retreat);
    }
  }
  for(int i=0;i<reste5;i++)
  {
    buffer = new QByteArray();
    QDataStream stream(buffer, QIODevice::WriteOnly);
    stream << quint32(1);
    if (m_invadeSlide->value()>m_currentSlideValue)
    {
    	automaton()->sendMessage(*buffer,Invade);
    }
    else
    {
        automaton()->sendMessage(*buffer,Retreat);
    }
  }
  m_currentSlideValue = m_invadeSlide->value();
}
void KGameWindow::slideClose()
{
  m_wSlide->close();
  QPoint point;
  m_automaton->gameEvent("actionInvasionFinished", point);
}

void KGameWindow::slotContextualHelp()
{
  kDebug();
  if (currentPlayer()->isAI())
  {
    return;
  }
  switch (m_automaton->state())
  {
    case GameLogic::GameAutomaton::WAIT:
      showMessage(i18n("Now, attack by dragging from the attacking country<br/>and droping to the attacked one<br/>or choose another action by using the right mouse click.<br/>Note that moving armies is the last action of a turn."), 5);
    break;
    default:;
  }
}

void KGameWindow::slotDisableHelp(const QString & link)
{
  kDebug() << link;
  KsirkSettings::setHelpEnabled(false);
}

} // closing namespace Ksirk
