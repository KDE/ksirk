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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

// application specific includes
#include "kgamewin.h"
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
//   kDebug() << k_funcinfo << endl;
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

  if ( (!m_timer.isActive())
    && ( ((mousePos.x() < 10) && (mousePos.x() >= 0)
          && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
        || ((mousePos.x() > m_frame-> viewport()->width() -10) &&
          (mousePos.x() <= m_frame-> viewport()->width())
          && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
        || ((mousePos.y() < 10) && (mousePos.y() >= 0)
          && (mousePos.x() >= 0) && (mousePos.x() <= m_frame-> viewport()->width()))
        || ((mousePos.y() >  m_frame-> viewport()->height() -10) &&
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

//   kDebug() << "KGameWindow::evenementTimer" << endl;
  mousePos = QCursor::pos();
  mousePosition =  m_frame-> mapFromGlobal(mousePos);
//   kDebug() << "mousePosition = ( " << mousePosition.x() << " , " << mousePosition.y() << " )" << endl;
//   kDebug() << "m_frame = ( " << m_frame-> viewport()->width() << " , " << m_frame-> viewport()->height() << " )" << endl;

  bool restart = false;
  if ((mousePosition.x() < 10) && (mousePosition.x() >= 0)
      && (mousePosition.y() >= 0) && (mousePosition.y() <= m_frame-> viewport()->height()))
  {
    kDebug() << "scrollRight" << endl;
    m_frame-> horizontalScrollBar()->setValue ( m_frame-> horizontalScrollBar()->value() - 16);
    restart = true;
  }

  if ((mousePosition.x() > m_frame-> viewport()->width() -10) &&
      (mousePosition.x() <= m_frame-> viewport()->width())
      && (mousePosition.y() >= 0) && (mousePosition.y() <= m_frame-> viewport()->height()))
  {
    kDebug() << "scrollLeft" << endl;
    m_frame-> horizontalScrollBar()->setValue ( m_frame-> horizontalScrollBar()->value() + 16);
    restart = true;
  }
  if ((mousePosition.y() < 10) && (mousePosition.y() >= 0)
      && (mousePosition.x() >= 0) && (mousePosition.x() <= m_frame-> viewport()->width()))
  {
    kDebug() << "scrollDown" << endl;
    m_frame-> verticalScrollBar()->setValue ( m_frame-> verticalScrollBar()->value() - 16);
    restart = true;
  }
  if ((mousePosition.y() >  m_frame-> viewport()->height() -10) &&
      (mousePosition.y() <=  m_frame-> viewport()->height())
      && (mousePosition.x() >= 0) && (mousePosition.x() <= m_frame-> viewport()->width()))
  {
    kDebug() << "scrollUp " << m_frame-> verticalScrollBar()->value()
      << "("<< m_frame-> verticalScrollBar()->minimum()
      << ", " << m_frame-> verticalScrollBar()->maximum() << ")" << endl;
    m_frame-> verticalScrollBar()->setValue ( m_frame-> verticalScrollBar()->value() + 16);
    restart = true;
  }

  if ( m_animFighters->isEmpty() )
  {
    if ( m_secondCountry && !( m_secondCountry-> owner() ) )
      kError() << m_secondCountry-> name() << " does not belong to anybody !" << endl;
    // handling of the AI player actions
    if ( !( ( ( currentPlayer() && currentPlayer()-> isAI() )
            || ( ( isMyState(GameLogic::GameAutomaton::WAITDEFENSE)  ) && ( m_secondCountry ) && (m_secondCountry->owner())
                 && ( m_secondCountry->owner()->isAI() ) ) ) ) )
      toolBar("gameActionsToolBar")-> show();
  }


//    kDebug() << "OUT KGameWindow::evenementTimer" << endl;
  if (restart)
  {
//     kDebug() << "restarting timer" << endl;
    m_timer.start(200);
  }
}

void KGameWindow::slotLeftButtonDown(const QPointF& point)
{
//   kDebug() << "slotLeftButtonDown" << endl;
//   if (currentPlayer() && !(currentPlayer()-> isAI()) )
    m_automaton->event("actionLButtonDown", point);
}

void KGameWindow::slotLeftButtonUp(const QPointF& point)
{
//     kDebug() << "slotLeftButtonUp" << endl;
//     if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->event("actionLButtonUp", point);
}

void KGameWindow::slotRightButtonDown(const QPointF& point)
{
//   if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->event("actionRButtonDown", point);
  return;
}

/** @todo Clean exit with memory freeing */
bool KGameWindow::queryExit()
{
//   kDebug() << "Writing skin m_config: " << m_automaton->skin() << endl;
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

void KGameWindow::slotNewGame()
{
//   kDebug() << "Slot new game: posting event actionNewGame" << endl;
//   QPoint point;
  if (actionNewGame())
  {
    m_automaton->state(GameAutomaton::INIT);
  }
//   m_automaton->event("actionNewGame", point);
}

void KGameWindow::slotOpenGame()
{
  kDebug() << "Slot open game: posting event actionOpenGame" << endl;
  QPoint point;
  m_automaton->event("actionOpenGame", point);
//   actionOpenGame();
}

void KGameWindow::slotSaveGame()
{
  if (m_automaton->isAdmin())
  {
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
  m_automaton->event("actionRecycling", point);
}

void KGameWindow::slotRecyclingFinished()
{
  QPoint point;
  m_automaton->event("actionRecyclingFinished", point);
}

void KGameWindow::slotNextPlayer()
{
  QPoint point;
  m_automaton->event("actionNextPlayer", point);
}

void KGameWindow::slotAttack1()
{
  QPoint point;
  m_automaton->event("actionAttack1", point);
}

void KGameWindow::slotAttack2()
{
  QPoint point;
  m_automaton->event("actionAttack2", point);
}

void KGameWindow::slotAttack3()
{
  QPoint point;
  m_automaton->event("actionAttack3", point);
}

void KGameWindow::slotDefense1()
{
  QPoint point;
  m_automaton->event("actionDefense1", point);
}

void KGameWindow::slotDefense2()
{
  QPoint point;
  m_automaton->event("actionDefense2", point);
}

void KGameWindow::slotInvade1()
{
  QPoint point;
  m_automaton->event("actionInvade1", point);
}

void KGameWindow::slotInvade5()
{
  QPoint point;
  m_automaton->event("actionInvade5", point);
}

void KGameWindow::slotInvade10()
{
  QPoint point;
  m_automaton->event("actionInvade10", point);
}

void KGameWindow::slotInvasionFinished()
{
  QPoint point;
  m_automaton->event("actionInvasionFinished", point);
}

void KGameWindow::slotRetreat1()
{
  QPoint point;
  m_automaton->event("actionRetreat1", point);
}

void KGameWindow::slotRetreat5()
{
  QPoint point;
  m_automaton->event("actionRetreat5", point);
}

void KGameWindow::slotRetreat10()
{
  QPoint point;
  m_automaton->event("actionRetreat10", point);
}

void KGameWindow::slotMove()
{
  QPoint point;
  m_automaton->event("actionMove", point);
}

void KGameWindow::slotCancel()
{
  QPoint point;
  m_automaton->event("actionCancel", point);
}

void KGameWindow::slotDumpGameInformations()
{
//   kDebug() << "Game information : " << endl;
//    kDebug() << "  state : " << GameStateNames[getState()] << endl;
//   kDebug() << "  current player : " 
//       << ((currentPlayer()) ? currentPlayer()-> name() : "nobody") << endl;
}

void KGameWindow::slotFinishMoves()
{
//   kDebug() << "slotFinishMoves" << endl;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  m_automaton->sendMessage(buffer,FinishMoves);
}

void KGameWindow::slotShowAboutApplication()
{
    //kDebug() << "Dans mon About !" << endl;

/*  KAboutApplication kAA;
  kAA.exec();*/
  KAboutApplicationDialog dialog(KGlobal::mainComponent().aboutData(), this);
  dialog.exec();
}

void KGameWindow::slotJoinNetworkGame() 
{
  kDebug() << k_funcinfo << endl;
  QPoint point;
  m_automaton->event("actionJoinNetworkGame", point);
}

void KGameWindow::slotShowGoal()
{
//   kDebug() << "slotShowGoal" << endl;
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

void KGameWindow::slotMovingFightersArrived(AnimSpritesGroup* sprites)
{
Q_UNUSED(sprites)
  kDebug() << k_funcinfo << endl;
  m_automaton->movingFigthersArrived();
}

void KGameWindow::slotMovingArmiesArrived(AnimSpritesGroup* sprites)
{
  kDebug() << k_funcinfo << endl;
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

void KGameWindow::slotMovingArmyArrived(AnimSprite* sprite)
{
  kDebug() << k_funcinfo << endl;
  sprite->setStatic();
  sprite->hide();
  m_automaton->movingArmyArrived(sprite->getDestination(),
    ((ArmySprite*)sprite)->nbArmies());
}

void KGameWindow::slotFiringFinished(AnimSpritesGroup* sprites)
{
Q_UNUSED(sprites)
  kDebug() << k_funcinfo << endl;
  m_automaton->firingFinished();
}

void KGameWindow::slotExplosionFinished(AnimSpritesGroup* sprites)
{
  kDebug() << k_funcinfo << endl;
  while (!sprites->empty())
  {
    AnimSprite* sprite = sprites->front();
    sprites->pop_front();
    sprite->setStatic();
    delete sprite;
  }
  m_automaton->explosionFinished();
}

void KGameWindow::slotZoomIn()
{
  kDebug() << k_funcinfo << endl;
  m_theWorld->applyZoomFactor(1.1);
  m_frame->setMaximumSize(m_theWorld->width(),m_theWorld->height());
  m_scene->setSceneRect ( QRectF() );
  m_frame->updateGeometry();
  m_backGnd->setPixmap(m_theWorld->map());
  
}

void KGameWindow::slotZoomOut()
{
  kDebug() << k_funcinfo << endl;
  m_theWorld->applyZoomFactor(1.0/1.1);
  m_frame->setMaximumSize(m_theWorld->width(),m_theWorld->height());
  m_scene->setSceneRect ( QRectF() );
  m_frame->updateGeometry();
  
  m_backGnd->setPixmap(m_theWorld->map());
}

void KGameWindow::slotRemoveMessage()
{
  kDebug() << k_funcinfo << endl;
  if (m_message != 0)
  {
    kDebug() << "hiding and deleting" << endl;
    m_message->hide();
    delete m_message;
    m_message = 0;
  }
}

} // closing namespace Ksirk
