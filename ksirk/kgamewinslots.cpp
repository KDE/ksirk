/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

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

// application specific includes
#include "kgamewin.h"
#include "ksirksettings.h"
#include "newgamesetup.h"
#include "Sprites/backgnd.h"
#include "Sprites/arrowsprite.h"
#include "Dialogs/newGameSummaryWidget.h"
#include "GameLogic/newplayerdata.h"

#include <KAboutApplicationDialog>
#include <KToolBar>

#include "GameLogic/KMessageParts.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/country.h"
#include "GameLogic/onu.h"
#include "GameLogic/goal.h"
#include "SaveLoad/ksirkgamexmlloader.h"
#include "Sprites/animspritesgroup.h"
#include "Dialogs/jabbergameui.h"
#include "Dialogs/kplayersetupwidget.h"
#include "Jabber/kmessagejabber.h"

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kmessageserver.h>

// STL include files
#include <fstream>

// include files for QT
#include <QCursor>
#include <QScrollBar>
#include <QUuid>
#include <QTextStream>
#include <QInputDialog>
#include <QFileDialog>
#include <QUrl>

// include files for KDE
#include <KLocalizedString>
#include <KConfig>
#include "ksirk_debug.h"
#include <KMessageBox>
#include <KGamePopupItem>
#include <KPasswordDialog>
#include <tcpconnectwidget.h>

namespace Ksirk
{
using namespace GameLogic;

// void KGameWindow::slotTimerEvent()
void KGameWindow::mouseMoveEvent ( QMouseEvent * event )
{
//   qCDebug(KSIRK_LOG);
  QString countryName;
  QPoint mousePos;
  QPoint mousePosGlobal;
  QPointF mousePosition;
  Country *mouseLocalisation;

  if (m_frame == nullptr || m_reinitializingGame)
  {
    return;
  }
  mousePosGlobal = event->globalPos();
  mousePos = m_frame->mapFromGlobal(mousePosGlobal);
  mousePosition = m_frame->mapToScene(mousePos);
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
    else if (m_mouseLocalisation == nullptr)
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
        status1Text = i18np("%2 belongs to %3. 1 army.","%2 belongs to %3. %1 armies.",mouseLocalisation->nbArmies(),i18n(countryName.toUtf8().data()),player->name());
      }

      //QT5 statusBar()-> changeItem(status1Text, ID_STATUS_MSG);
    }
  }
  else
  {
    if (m_mouseLocalisation!=nullptr)
    {
      m_mouseLocalisation->clearHighlighting();
      m_mouseLocalisation = nullptr;
    }
    //QT5 statusBar()-> changeItem("", ID_STATUS_MSG); // Reset
  }
  int borderScrollSize = 50;
  if ( (!m_timer.isActive())
    && ( ((mousePos.x() < borderScrollSize) && (mousePos.x() >= 0)
          && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
        || ((mousePos.x() > m_frame->viewport()->width() -borderScrollSize) &&
          (mousePos.x() <= m_frame-> viewport()->width())
          && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
        || ((mousePos.y() < borderScrollSize) && (mousePos.y() >= 0)
          && (mousePos.x() >= 0) && (mousePos.x() <= m_frame-> viewport()->width()))
        || ((mousePos.y() >  m_frame-> viewport()->height() -borderScrollSize) &&
          (mousePos.y() <=  m_frame-> viewport()->height())
          && (mousePos.x() >= 0) && (mousePos.x() <= m_frame-> viewport()->width()))
    )
  )
  // safety check for NULL arrow pointers, can happen with Qt 4.5
  if (m_uparrow == nullptr || m_downarrow == nullptr || m_leftarrow == nullptr || m_rightarrow == nullptr)
  {
    return;
  }
  if (currentWidget() != nullptr)
  {
    m_timer.start(200);
    if ((mousePos.y() < borderScrollSize) && (mousePos.y() >= 0)
      && (mousePos.x() >= 0) && (mousePos.x() <= m_frame-> viewport()->width()))
    {
      QPointF pos = currentWidget()->mapToScene(QPoint(m_frame->viewport()->width()/2,0));
      pos = pos + QPointF(-(m_uparrow->boundingRect().width()/2),m_uparrow->boundingRect().height());
      m_uparrow->setPos(pos);
      m_uparrow->setActive(true);
    }
    if ((mousePos.y() >  m_frame->viewport()->height() -borderScrollSize) &&
      (mousePos.y() <=  m_frame->viewport()->height())
      && (mousePos.x() >= 0) && (mousePos.x() <= m_frame-> viewport()->width()))
    {
      QPointF pos = currentWidget()->mapToScene(QPoint(m_frame->viewport()->width()/2,m_frame->viewport()->height()));
      pos = pos - QPointF(m_downarrow->boundingRect().width()/2,m_downarrow->boundingRect().height());
      m_downarrow->setPos(pos);
      m_downarrow->setActive(true);
    }
    if ((mousePos.x() < borderScrollSize) && (mousePos.x() >= 0)
      && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
    {
      QPointF pos = currentWidget()->mapToScene(QPoint(0,m_frame->viewport()->height()/2));
      pos = pos + QPointF(m_leftarrow->boundingRect().width(),-(m_leftarrow->boundingRect().height()/2));
      m_leftarrow->setPos(pos);
      m_leftarrow->setActive(true);
    }
    if ((mousePos.x() > m_frame->viewport()->width() -borderScrollSize) &&
      (mousePos.x() <= m_frame-> viewport()->width())
      && (mousePos.y() >= 0) && (mousePos.y() <= m_frame-> viewport()->height()))
    {
      QPointF pos = currentWidget()->mapToScene(QPoint(m_frame->viewport()->width(),m_frame->viewport()->height()/2));
      pos = pos - QPointF(m_rightarrow->boundingRect().width(),m_rightarrow->boundingRect().height()/2);
      m_rightarrow->setPos(pos);
      m_rightarrow->setActive(true);
      m_rightarrow->update();
    }
  }
  if (m_frame && m_frame->horizontalScrollBar()->value() == m_frame->horizontalScrollBar()->maximum())
  {
    m_rightarrow->hide();
  }
  else
  {
    m_rightarrow->show();
  }
  if (m_frame && m_frame->verticalScrollBar()->value() == m_frame->verticalScrollBar()->maximum())
  {
    m_downarrow->hide();
  }
  else
  {
    m_downarrow->show();
  }
  if (m_frame && m_frame->verticalScrollBar()->value() == m_frame->verticalScrollBar()->minimum())
  {
    m_uparrow->hide();
  }
  else
  {
    m_uparrow->show();
  }
}

void KGameWindow::evenementTimer()
{        //Appel du Timer toutes les 50 ms
  QPoint mousePos;
  QPointF mousePosition;

//   qCDebug(KSIRK_LOG) << "KGameWindow::evenementTimer";
  mousePos = QCursor::pos();
  mousePosition =  m_frame-> mapFromGlobal(mousePos);
//   qCDebug(KSIRK_LOG) << "mousePosition = ( " << mousePosition.x() << " , " << mousePosition.y() << " )";
//   qCDebug(KSIRK_LOG) << "m_frame = ( " << m_frame-> viewport()->width() << " , " << m_frame-> viewport()->height() << " )";

  bool restart = false;
  int borderScrollSize = 50;
  int borderScrollSpeed = 40;
  if ((mousePosition.x() < borderScrollSize) && (mousePosition.x() >= 0)
      && (mousePosition.y() >= 0) && (mousePosition.y() <= m_frame-> viewport()->height()))
  {
//     qCDebug(KSIRK_LOG) << "scrollRight";
    m_frame->horizontalScrollBar()->setValue(m_frame->horizontalScrollBar()->value() - borderScrollSpeed);
    if (m_frame->horizontalScrollBar()->value() == m_frame->horizontalScrollBar()->minimum())
    {
      m_leftarrow->hide();
    }
    else
    {
      m_leftarrow->show();
    }
    m_leftarrow->setActive(true);
    restart = true;
  }
  else
  {
    m_leftarrow->setActive(false);
  }
  QPointF pos = currentWidget()->mapToScene(QPoint(0,m_frame->viewport()->height()/2));
  pos = pos + QPointF(m_leftarrow->boundingRect().width(),-(m_leftarrow->boundingRect().height()/2));
  m_leftarrow->setPos(pos);
  
  if ((mousePosition.x() > m_frame-> viewport()->width() -borderScrollSize) &&
      (mousePosition.x() <= m_frame-> viewport()->width())
      && (mousePosition.y() >= 0) && (mousePosition.y() <= m_frame-> viewport()->height()))
  {
//     qCDebug(KSIRK_LOG) << "scrollLeft";
    m_frame-> horizontalScrollBar()->setValue ( m_frame-> horizontalScrollBar()->value() + borderScrollSpeed);
    m_rightarrow->setActive(true);
    m_rightarrow->update();
   restart = true;
  }
  else
  {
    m_rightarrow->setActive(false);
  }
  pos = currentWidget()->mapToScene(QPoint(m_frame->viewport()->width(),m_frame->viewport()->height()/2));
  pos = pos - QPointF(m_rightarrow->boundingRect().width(),m_rightarrow->boundingRect().height()/2);
  m_rightarrow->setPos(pos);
  
  if ((mousePosition.y() < borderScrollSize) && (mousePosition.y() >= 0)
      && (mousePosition.x() >= 0) && (mousePosition.x() <= m_frame-> viewport()->width()))
  {
//     qCDebug(KSIRK_LOG) << "scrollDown";
    m_frame-> verticalScrollBar()->setValue ( m_frame-> verticalScrollBar()->value() - borderScrollSpeed);
    m_uparrow->setActive(true);
    restart = true;
  }
  else
  {
    m_uparrow->setActive(false);
  }
  pos = currentWidget()->mapToScene(QPoint(m_frame->viewport()->width()/2,0));
  pos = pos + QPointF(-(m_uparrow->boundingRect().width()/2),m_uparrow->boundingRect().height());
  m_uparrow->setPos(pos);
  
  if ((mousePosition.y() >  m_frame-> viewport()->height() -borderScrollSize) &&
      (mousePosition.y() <=  m_frame-> viewport()->height())
      && (mousePosition.x() >= 0) && (mousePosition.x() <= m_frame-> viewport()->width()))
  {
//     qCDebug(KSIRK_LOG) << "scrollUp " << m_frame-> verticalScrollBar()->value()
//       << "("<< m_frame-> verticalScrollBar()->minimum()
//       << ", " << m_frame-> verticalScrollBar()->maximum() << ")";
    m_frame-> verticalScrollBar()->setValue ( m_frame-> verticalScrollBar()->value() + borderScrollSpeed);
    m_downarrow->setActive(true);
    restart = true;
  }
  else
  {
    m_downarrow->setActive(false);
  }
  pos = currentWidget()->mapToScene(QPoint(m_frame->viewport()->width()/2,m_frame->viewport()->height()));
  pos = pos - QPointF(m_downarrow->boundingRect().width()/2,m_downarrow->boundingRect().height());
  m_downarrow->setPos(pos);
  
  if ( m_animFighters->isEmpty() )
  {
    if ( m_secondCountry && !( m_secondCountry-> owner() ) )
      qCCritical(KSIRK_LOG) << m_secondCountry-> name() << " does not belong to anybody !";
    // handling of the AI player actions
    /*if ( !( ( ( currentPlayer() && currentPlayer()-> isAI() )
            || ( ( isMyState(GameLogic::GameAutomaton::WAITDEFENSE)  ) && ( m_secondCountry ) && (m_secondCountry->owner())
                 && ( m_secondCountry->owner()->isAI() ) ) ) ) ) {
      toolBar("gameActionsToolBar")-> show();
    }*/
  }

  if (m_frame->horizontalScrollBar()->value() == m_frame->horizontalScrollBar()->minimum())
  {
    m_leftarrow->hide();
  }
  else
  {
    m_leftarrow->show();
  }
  if (m_frame->horizontalScrollBar()->value() == m_frame->horizontalScrollBar()->maximum())
  {
    m_rightarrow->hide();
  }
  else
  {
    m_rightarrow->show();
  }
  if (m_frame->verticalScrollBar()->value() == m_frame->verticalScrollBar()->maximum())
  {
    m_downarrow->hide();
  }
  else
  {
    m_downarrow->show();
  }
  if (m_frame->verticalScrollBar()->value() == m_frame->verticalScrollBar()->minimum())
  {
    m_uparrow->hide();
  }
  else
  {
    m_uparrow->show();
  }
  
  //    qCDebug(KSIRK_LOG) << "OUT KGameWindow::evenementTimer";
  if (restart)
  {
//     qCDebug(KSIRK_LOG) << "restarting timer";
    m_timer.start(200);
  }
}

void KGameWindow::slotLeftButtonDown(const QPointF& point)
{
//   qCDebug(KSIRK_LOG) << "slotLeftButtonDown";
//   if (currentPlayer() && !(currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionLButtonDown", point);
}

void KGameWindow::slotLeftButtonUp(const QPointF& point)
{
//     qCDebug(KSIRK_LOG) << "slotLeftButtonUp";
//     if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionLButtonUp", point);
}

void KGameWindow::slotRightButtonDown(const QPointF& point)
{
  qCDebug(KSIRK_LOG);
//   if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionRButtonDown", point);
  return;
}

void KGameWindow::slotRightButtonUp(const QPointF& point)
{
  qCDebug(KSIRK_LOG);
//   if (currentPlayer() && ! (currentPlayer()-> isAI()) )
    m_automaton->gameEvent("actionRButtonUp", point);
  return;
}

void KGameWindow::slotArena(bool isCheck)
{
  if (isCheck)
  {
    m_useArena = true;
    qCDebug(KSIRK_LOG) << "*******Arena On******" << m_useArena;
  }
  else
  {
    m_useArena = false;
    qCDebug(KSIRK_LOG) << "*******Arena Off******" << m_useArena;
  }
}

void KGameWindow::slotJabberGame()
{
  m_jabberGameWidget->init(m_automaton);
  m_jabberGameWidget->setPreviousGuiIndex(m_centralWidget->currentIndex());
  m_centralWidget->setCurrentIndex(JABBERGAME_INDEX);
}

void KGameWindow::slotNewGame()
{
  //   qCDebug(KSIRK_LOG) << "Slot new game: posting event actionNewGame";
  actionNewGame(GameAutomaton::None);
}

void KGameWindow::slotNewJabberGame()
{
  actionNewGame(GameAutomaton::Jabber);
}

void KGameWindow::slotNewSocketGame()
{
  //   qCDebug(KSIRK_LOG) << "Slot new game: posting event actionNewGame";
  //   QPoint point;
  actionNewGame(GameAutomaton::Socket);
  
  /// @TODO set the state to init when new game is started
  /*  if (actionNewGame())
  {
    m_automaton->state(GameAutomaton::INIT);
}*/
//   m_automaton->gameEvent("actionNewGame", point);
}

void KGameWindow::slotOpenGame()
{
  qCDebug(KSIRK_LOG) << "Slot open game: posting event actionOpenGame";
  QPoint point;
  m_automaton->gameEvent("actionOpenGame", point);
//   actionOpenGame();
}

void KGameWindow::slotSaveGame()
{
  if (m_message == nullptr)
  {
    setupPopupMessage();
  }
  if (m_automaton->isAdmin())
  {
    if (isMyState(GameLogic::GameAutomaton::WAITDEFENSE))
    {
      m_message->setMessageTimeout(3000);
      m_message->showMessage(i18n("Cannot save when waiting for a defense."),
          KGamePopupItem::TopLeft,
          KGamePopupItem::ReplacePrevious);
      return;
    }
    if (m_fileName.isEmpty())
    {
      QString fileName = QFileDialog::getSaveFileName (this, i18nc("@title:window", "KsirK - Save Game"), QString(), "*.xml");
      if ( QFile::exists(fileName)
          && (KMessageBox::questionYesNo (this,
                i18n("%1 exists.\nDo you really want to overwrite it?",fileName),
                i18n("Overwrite file?")) == KMessageBox::No) )
      {
        m_message->setMessageTimeout(3000);
        m_message->showMessage(i18n("Saving canceled"), KGamePopupItem::TopLeft,
            KGamePopupItem::ReplacePrevious);
        return;
      }
      m_fileName = fileName;
    }
    if (!m_fileName.isEmpty())
    {
      QFile file(m_fileName);
      file.open(QIODevice::WriteOnly);
      QTextStream ofs(&file);
      ofs.setCodec("UTF-8");
      saveXml(ofs);
      m_message->setMessageTimeout(3000);
      m_message->showMessage(i18n("Game saved to %1",m_fileName),
          KGamePopupItem::TopLeft, KGamePopupItem::ReplacePrevious);
    }
  }
  else
  {
    m_message->setMessageTimeout(3000);
    m_message->showMessage(
        i18n("Only game master can save the game in network playing."),
        KGamePopupItem::TopLeft, KGamePopupItem::ReplacePrevious);
  }
}

void KGameWindow::slotRecycling()
{
  qCDebug(KSIRK_LOG);
  QPoint point;
  m_automaton->gameEvent("actionRecycling", point);
}

void KGameWindow::slotRecyclingFinished()
{
  qCDebug(KSIRK_LOG);
  QPoint point;
  m_rightDock->hide();
  m_automaton->gameEvent("actionRecyclingFinished", point);
}

void KGameWindow::slotNextPlayer()
{
  qCDebug(KSIRK_LOG);
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
//   qCDebug(KSIRK_LOG) << "Game information : ";
//    qCDebug(KSIRK_LOG) << "  state : " << GameStateNames[getState()];
//   qCDebug(KSIRK_LOG) << "  current player : " 
//       << ((currentPlayer()) ? currentPlayer()-> name() : "nobody");
}

void KGameWindow::slotFinishMoves()
{
  qCDebug(KSIRK_LOG);
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  m_automaton->sendMessage(buffer,FinishMoves);
}

void KGameWindow::slotShowAboutApplication()
{
    //qCDebug(KSIRK_LOG) << "Dans mon About !";

/*  KAboutApplication kAA;
  kAA.exec();*/
#if 0 //QT5
  KAboutApplicationDialog dialog(KGlobal::mainComponent().aboutData(), this);
  dialog.exec();
#endif
}

void KGameWindow::slotJoinNetworkGame() 
{
  qCDebug(KSIRK_LOG);
  QPoint point;
  m_automaton->gameEvent("actionJoinNetworkGame", point);
}

void KGameWindow::slotConnectToServer()
{
  qCDebug(KSIRK_LOG);
  m_newGameSetup->setHost(m_tcpConnectWidget->hostEdit->text());
  m_newGameSetup->setTcpPort(m_tcpConnectWidget->portEdit->value());
  m_automaton->setGameStatus(KGame::End);
  m_reinitializingGame = true;
  
  if (!(m_automaton->playerList()->isEmpty()))
  {
    m_automaton->playerList()->clear();
    m_automaton->currentPlayer(nullptr);
    qCDebug(KSIRK_LOG) << "  playerList size = " << m_automaton->playerList()->count();
  }
  theWorld()->reset();
  
  m_automaton->connectToServ();
}

void KGameWindow::slotShowGoal()
{
  qCDebug(KSIRK_LOG);
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

void KGameWindow::slotChatFloatChanged(bool /*value*/)
{
  // change the float button image
  if (!m_bottomDock->isFloating())
  {
    m_floatChatButton->setIcon(m_upChatFloatPix);
  }
  else
  {
    m_floatChatButton->setIcon(m_downChatFloatPix);
  }
}

void KGameWindow::slotChatReduceButton()
{
  qCDebug(KSIRK_LOG);

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
  qCDebug(KSIRK_LOG);
  m_automaton->movingFigthersArrived();
}

void KGameWindow::slotMovingArmiesArrived(AnimSpritesGroup* sprites)
{
  qCDebug(KSIRK_LOG);
  AnimSpritesGroup::iterator it, it_end;
  it = sprites->begin(); it_end = sprites->end();
  for (; it != it_end; it++)
  {
    (*it)->hide();
    (*it)->deleteLater();
  }
  sprites->clear();
  int index = m_animSpritesGroups.indexOf(sprites);
  if (index != -1)
  {
    m_animSpritesGroups.removeAt(index);
  }
  sprites->deleteLater();
  KMessageParts messageParts;
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
}

void KGameWindow::slotBring(AnimSprite* /*sprite*/)
{
	m_automaton->game()->stopExplosion();
}

void KGameWindow::slotMovingArmyArrived(AnimSprite* sprite)
{
  qCDebug(KSIRK_LOG);
  sprite->setStatic();
  sprite->hide();
  m_automaton->movingArmyArrived(sprite->getDestination(),
    ((ArmySprite*)sprite)->nbArmies());
}

void KGameWindow::slotFiringFinished(AnimSpritesGroup* sprites)
{
Q_UNUSED(sprites)
  qCDebug(KSIRK_LOG);
  m_automaton->firingFinished();
}

void KGameWindow::slotExplosionFinished(AnimSpritesGroup* sprites)
{
  qCDebug(KSIRK_LOG);
  while (!sprites->empty())
  {
    AnimSprite* sprite = sprites->front();
    sprites->pop_front();
    sprite->setStatic();
    sprite->deleteLater();
  }
  
  if (backGnd()->bgIsArena())
  {
    m_automaton->game()->initCombatBringBackForArena(m_automaton->game()->firstCountry(), m_automaton->game()->secondCountry());
  }

  m_automaton->explosionFinished();
  qCDebug(KSIRK_LOG)<<"DONE";
}

void KGameWindow::slotZoomIn()
{
  qCDebug(KSIRK_LOG);
  m_theWorld->applyZoomFactorFast(1.1);					//Call to the fast method (benj)
  m_frame->setMaximumSize(m_theWorld->width(),m_theWorld->height());
  m_scene_world->setSceneRect ( QRectF() );
  m_frame->updateGeometry();
  m_backGnd_world->setPixmap(m_theWorld->map());
  
}

void KGameWindow::slotZoomOut()
{
  qCDebug(KSIRK_LOG);
  m_theWorld->applyZoomFactorFast(1.0/1.1);				//call to the fast method
  m_frame->setMaximumSize(m_theWorld->width(),m_theWorld->height());
  m_scene_world->setSceneRect ( QRectF() );
  m_frame->updateGeometry();
  m_backGnd_world->setPixmap(m_theWorld->map());
}

void KGameWindow::slotRemoveMessage()
{
  qCDebug(KSIRK_LOG);
  if (m_message != nullptr)
  {
    qCDebug(KSIRK_LOG) << "hiding and deleting";
    m_message->hide();
    delete m_message;
    m_message = nullptr;
  }
}

void KGameWindow::slotContextualHelp()
{
  qCDebug(KSIRK_LOG);
  if (currentPlayer() == nullptr || currentPlayer()->isAI())
  {
    return;
  }
  switch (m_automaton->state())
  {
    case GameLogic::GameAutomaton::WAIT:
      showMessage(i18n("Attack by drag & drop between countries<br>Move armies the same way (last action of a turn)."), 5);
      break;
    case GameLogic::GameAutomaton::NEWARMIES:
    case GameLogic::GameAutomaton::INTERLUDE:
      showMessage(i18n("Now, place your armies in your countries<br>by clicking in the target countries."));
      break;
    default:;
  }
}

void KGameWindow::slotDisableHelp(const QString & link)
{
  qCDebug(KSIRK_LOG) << link;
  KsirkSettings::setHelpEnabled(false);
}

void KGameWindow::slotArmiesNumberChanged(int checked)
{
  qCDebug(KSIRK_LOG) << checked;
  KsirkSettings::setShowArmiesNumbers(checked);

  foreach (Country* country, m_theWorld->getCountries())
  {
    country->createArmiesSprites();
  }
}

void KGameWindow::slotDefAuto()
{
  QPoint point;
  qCDebug(KSIRK_LOG)<<"Recept signal defense auto";
  m_automaton->setDefenseAuto(true);
  if (this->firstCountry()->owner()->getNbAttack() == 1)
    m_automaton->gameEvent("actionDefense1", point);
  else
    m_automaton->gameEvent("actionDefense2", point);
  m_defenseDialog->close();
}

void KGameWindow::slotWindowDef1()
{
  QPoint point;
  qCDebug(KSIRK_LOG)<<"Recept signal defense with one army";
  m_automaton->gameEvent("actionDefense1", point);
  m_defenseDialog->close();
}

void KGameWindow::slotWindowDef2()
{
  QPoint point;
  qCDebug(KSIRK_LOG)<<"Recept signal defense with two army";
  m_automaton->gameEvent("actionDefense2", point);
  m_defenseDialog->close();
}

void KGameWindow::slotNewGameNext()
{
  qCDebug(KSIRK_LOG);
  m_automaton->newGameNext();
  m_reinitializingGame = false;
  
  if (m_automaton->networkGameType()==GameAutomaton::Jabber && m_jabberClient && m_jabberClient->isConnected())
  {
    sendGameInfoToJabber();
  }
}

// void KGameWindow::slotNewGameOK(unsigned int nbPlayers, const QString& skin, unsigned int nbNetworkPlayers, bool useGoals)
// {
//   qCDebug(KSIRK_LOG) << nbPlayers << skin << nbNetworkPlayers << useGoals;
//   m_automaton->setGameStatus(KGame::End);
//   m_reinitializingGame = true;
//   m_automaton->removeAllPlayers();
// 
//   showMap();
//   m_newPlayersNumber = nbPlayers;
//   m_automaton->setUseGoals(useGoals);
//   m_automaton->state(GameLogic::GameAutomaton::INIT);
//   m_automaton->savedState(GameLogic::GameAutomaton::INVALID);
//   m_automaton->setNetworkPlayersNumber(m_automaton->networkGameType()==GameAutomaton::None?0:nbNetworkPlayers);
//   m_automaton->finishSetupPlayersNumberAndSkin(skin, nbPlayers);
//   m_reinitializingGame = false;
// 
//   if (m_automaton->networkGameType()==GameAutomaton::Jabber && m_jabberClient && m_jabberClient->isConnected())
//   {
//     sendGameInfoToJabber();
//   }
// }

void KGameWindow::slotNewGameKO()
{
  qCDebug(KSIRK_LOG) << m_stateBeforeNewGame << m_stackWidgetBeforeNewGame;
  m_automaton->state(m_stateBeforeNewGame);
  m_centralWidget->setCurrentIndex(m_stackWidgetBeforeNewGame);
  m_automaton->setGameStatus(KGame::Run);
}

void KGameWindow::slotConnected()
{
  qCDebug(KSIRK_LOG) << "Connected to Jabber server.";
  
  qCDebug(KSIRK_LOG) << "Requesting roster...";
  m_jabberClient->requestRoster ();
}

void KGameWindow::slotRosterRequestFinished ( bool success )
{
  qCDebug(KSIRK_LOG) << success;
  
  /* Since we are online now, set initial presence. Don't do this
  * before the roster request or we will receive presence
  * information before we have updated our roster with actual
  * contacts from the server! (Iris won't forward presence
  * information in that case either). */
  qCDebug(KSIRK_LOG) << "Setting initial presence...";
  setPresence ( m_initialPresence );
}

void KGameWindow::slotCSDisconnected ()
{
  qCDebug(KSIRK_LOG) << "Disconnected from Jabber server.";
  
  /*
  * We should delete the JabberClient instance here,
  * but timers etc prevent us from doing so. Iris does
  * not like to be deleted from a slot.
  */

  /* It seems that we don't get offline notifications when going offline
  * with the protocol, so clear all resources manually. */
//   resourcePool()->clear();
  
}

void KGameWindow::slotReceivedMessage (const XMPP::Message & message)
{
  qCDebug(KSIRK_LOG) << "New " << message.type() << " message from " << message.from().full () << ": " << message.body();

  QString body = message.body();
  QString nick = message.from().full();

  if ( message.type() == "groupchat" )
  {
    qCDebug(KSIRK_LOG) << "my jid:" << m_groupchatRoom+'@'+m_groupchatHost+'/'+m_groupchatNick;
    XMPP::Jid jid ( message.from().domain() );
    if (body.startsWith(QLatin1String("I'm starting a game with skin"))
      && m_presents.contains(message.from().full ())
      && message.from().full() != m_groupchatRoom+'@'+m_groupchatHost+'/'+m_groupchatNick)
    {
      qCDebug(KSIRK_LOG) << "start game message";
      QRegExp rxlen("I'm starting a game with skin '([^']*)' and '(\\d+)' players");
//       int pos = rxlen.indexIn(body);
      QString skin = rxlen.cap(1); // "189"
      int nbPlayers = rxlen.cap(2).toInt();  // "cm"
      qCDebug(KSIRK_LOG) << "emiting newJabberGame" << nick << nbPlayers << skin;
      emit newJabberGame(nick, nbPlayers, skin);
    }
    else if (body.startsWith(QLatin1String("Who propose online KsirK games here?")))
    {
      qCDebug(KSIRK_LOG) << "online games question" << m_automaton->stateName();
      if (m_automaton->startingGame())
      {
        sendGameInfoToJabber();
      }
    }
  }
  else if ( message.type() == "ksirkgame" )
  {
    XMPP::Jid jid ( message.from() );
    if (body == "connect")
    {
      qCDebug(KSIRK_LOG) << "received connect from" << jid.full();
      KMessageJabber* messageIO = new KMessageJabber(jid.full(), m_jabberClient, this);
      if (m_automaton->messageServer())
      {
        m_automaton->messageServer()->addClient(messageIO);
      }
    }
    else
    {
      qCDebug(KSIRK_LOG) << "non connect ksirkgame message";
    }
  }
}
void KGameWindow::slotHandleTLSWarning (QCA::TLS::IdentityResult identityResult, QCA::Validity validityResult )
{
  qCDebug(KSIRK_LOG) << "Handling TLS warning..." << identityResult << validityResult;
}

void KGameWindow::slotClientError ( JabberClient::ErrorCode errorCode )
{
  qCDebug(KSIRK_LOG) << "Handling client error...";

  switch ( errorCode )
  {
    case JabberClient::NoTLS:
    default:
#if 0 //QT5
      KMessageBox::queuedMessageBox ( 0, KMessageBox::Error,
                                                                                i18n ("An encrypted connection with the Jabber server could not be established."),
                                                                                i18n ("Jabber Connection Error"));
//                                                                                 disconnect ( 0/*Kopete::Account::Manual*/ );
#endif
                                                                                break;
                                            }
}

void KGameWindow::slotCSError ( int error )
{
  qCDebug(KSIRK_LOG) << "Error in stream signalled.";
  
  if ( ( error == XMPP::ClientStream::ErrAuth )
    && ( m_jabberClient->clientStream()->errorCondition () == XMPP::ClientStream::NotAuthorized ) )
  {
    qCDebug(KSIRK_LOG) << "Incorrect password, retrying.";
//     disconnect(/*Kopete::Account::BadPassword*/0);
  }
  else
  {
//     int errorClass =  0;
    
    qCDebug(KSIRK_LOG) << "Disconnecting.";
    
    // display message to user
//     if(!m_removing) //when removing the account, connection errors are normal.
//       handleStreamError (error, m_jabberClient->clientStream()->errorCondition (), m_jabberClient->clientConnector()->errorCode (), server (), errorClass, m_jabberClient->clientStream()->errorText());
//     
//     disconnect ( errorClass );
    
    /*  slotCSDisconnected  will not be called*/
//     resourcePool()->clear();
  }
  
}

void KGameWindow::slotClientDebugMessage ( const QString &msg )
{
  qCDebug(KSIRK_LOG) << msg;
}

void KGameWindow::slotGroupChatJoined (const XMPP::Jid & jid)
{
  qCDebug(KSIRK_LOG) << "Joined groupchat " << jid.full ();
  
  /*    <message type="chat" to="kleag@localhost" id="aabca" >
  <body>hello</body>
  </message>*/
  XMPP::Message message(QString(m_groupchatRoom+'@'+m_groupchatHost));
  message.setType("groupchat");
  message.setId(QUuid::createUuid().toString().remove('{').remove('}').remove('-'));
  message.setBody("Hello, I'm a KsirK Game");
  m_jabberClient->sendMessage(message);

  askForJabberGames();
}

void KGameWindow::slotGroupChatLeft (const XMPP::Jid & jid)
{
  qCDebug(KSIRK_LOG) << "Left groupchat " << jid.full ();
}

void KGameWindow::slotGroupChatPresence (const XMPP::Jid & jid, const XMPP::Status & status)
{
  qCDebug(KSIRK_LOG) << "Received groupchat presence for room " << jid.full() << status.isAvailable();
  
  if (status.isAvailable())
  {
    XMPP::Message message(jid);
    message.setType("ksirkgame");
    message.setId(QUuid::createUuid().toString().remove('{').remove('}').remove('-'));
    message.setBody(QString("Hello, ")+jid.full());
    m_jabberClient->sendMessage(message);
    m_presents.insert(jid.full());
  }
  else
  {
    m_presents.remove(jid.full());
  }
}

void KGameWindow::slotGroupChatError (const XMPP::Jid &jid, int error, const QString &reason)
{
  qCDebug(KSIRK_LOG) << "Group chat error - room " << jid.full () << " had error " << error << " (" << reason << ")";
  
  switch (error)
  {
    case JabberClient::InvalidPasswordForMUC:
    {
      KPasswordDialog dlg(nullptr);
      dlg.setPrompt(i18n("A password is required to join the room %1.", jid.node()));
      if (dlg.exec() == KPasswordDialog::Accepted)
        m_jabberClient->joinGroupChat(jid.domain(), jid.node(), jid.resource(), dlg.password());
    }
    break;
    
    case JabberClient::NicknameConflict:
    {
      bool ok;
      QString nickname = QInputDialog::getText(this, i18n("Error trying to join %1: nickname %2 is already in use", jid.node(), jid.resource()), i18n("Provide your nickname"), QLineEdit::Normal, QString(), &ok);
      if (ok)
      {
        m_jabberClient->joinGroupChat(jid.domain(), jid.node(), nickname);
      }
    }
    break;
    
    case JabberClient::BannedFromThisMUC:
      //QT5 KMessageBox::queuedMessageBox ( 0, KMessageBox::Error, i18n ("You cannot join the room %1 because you have been banned", jid.node()), i18n ("Jabber Group Chat") );
      break;
                                      
    case JabberClient::MaxUsersReachedForThisMuc:
      //QT5 KMessageBox::queuedMessageBox ( 0, KMessageBox::Error, i18n ("You cannot join the room %1 because the maximum number of users has been reached", jid.node()), i18n ("Jabber Group Chat") );
      break;
                                      
    default:
    {
      QString detailedReason = reason.isEmpty () ? i18n ( "No reason given by the server" ) : reason;
      
      //QT5 KMessageBox::queuedMessageBox ( 0, KMessageBox::Error, i18n ("There was an error processing your request for groupchat %1. (Reason: %2, Code %3)", jid.full (), detailedReason, error ), i18n ("Jabber Group Chat") );
    }
  }
}

void KGameWindow::slotJabberGameCanceled(int previousIndex)
{
  qCDebug(KSIRK_LOG) << previousIndex;
  m_centralWidget->setCurrentIndex(previousIndex);
}

void KGameWindow::slotExit()
{
//   hide();
//   delete m_automaton;
  close();
}

void KGameWindow::slotNewPlayerNext()
{
  qCDebug(KSIRK_LOG) << m_newGameSetup->nbLocalPlayers() << m_newGameSetup->nbPlayers() << m_newGameSetup->nbNetworkPlayers();
  if (m_automaton->isAdmin() && m_newGameSetup->nbLocalPlayers() >= m_newGameSetup->nbPlayers()-m_newGameSetup->nbNetworkPlayers())
  {
    m_newGameSummaryWidget->show(this);
    m_centralWidget->setCurrentIndex(NEWGAMESUMMARY_INDEX);
  }
  else if (!m_automaton->isAdmin())
  {
    NewPlayerData* pd = m_newGameSetup->players().back();
    addPlayer(pd->name(), 0, 0, pd->nation(), pd->computer());
  }
}

void KGameWindow::slotNewPlayerPrevious()
{
  qCDebug(KSIRK_LOG);
  if (m_newGameSetup->players().empty())
  {
    m_centralWidget->setCurrentIndex(NEWGAME_INDEX);
  }
  else
  {
    NewPlayerData* player = m_newGameSetup->players().back();
    m_newGameSetup->players().pop_back();
    m_newPlayerWidget->init(player);
    delete player;
    m_centralWidget->setCurrentIndex(NEWPLAYER_INDEX);
  }
}

void KGameWindow::slotNewPlayerCancel()
{
  qCDebug(KSIRK_LOG);
  /// @TODO other uninits to do
  qDeleteAll(m_newGameSetup->players());
  m_newGameSetup->players().clear();
  m_centralWidget->setCurrentIndex(m_stackWidgetBeforeNewGame);
}

void KGameWindow::slotStartNewGame()
{
  qCDebug(KSIRK_LOG) << m_newGameSetup->nbPlayers() << m_newGameSetup->nbPlayers() << m_newGameSetup->players().size();
  m_automaton->setGameStatus(KGame::End);
  m_reinitializingGame = true;

  // delete remainings animations from previous game
  m_animFighters->clear();
  foreach(AnimSpritesGroup* sprites, m_animSpritesGroups)
  {
    sprites->clear();
    delete sprites;
  }
  m_animSpritesGroups.clear();
  
  // for network games, remote players are already created, should not remove them
  // TODO the players will not be in the orderd showed in the interface.
  if (!(m_automaton->playerList()->isEmpty()) && m_automaton->networkGameType() == GameAutomaton::None)
  {
    qCDebug(KSIRK_LOG) << "There was " << m_automaton->playerList()->count() << "players";
    m_automaton->playerList()->clear();
    m_automaton->currentPlayer(nullptr);
    qCDebug(KSIRK_LOG) << "  playerList size = " << m_automaton->playerList()->count();
  }
  theWorld()->reset();
  
  m_automaton->finishSetupPlayersNumberAndSkin();
  m_reinitializingGame = false;

  unsigned int nbAvailArmies = (unsigned int)(m_theWorld->getNbCountries() * 2.5 / m_newGameSetup->players().size());
  foreach (const NewPlayerData* player, m_newGameSetup->players())
  {
    if (!player->network())
    {
      // this has to be done locally
      addPlayer(player->name(), nbAvailArmies, 0, player->nation(), player->computer());
    }
  }

  if (m_newGameSetup->players().size() == m_newGameSetup->nbPlayers())
  {
    showMap();
    m_frame->setFocus();
    m_newGameSummaryWidget->finishButton->setEnabled(true);
  }
  else
  {
    m_newGameSummaryWidget->finishButton->setDisabled(true);
    m_newGameSummaryWidget->previousButton->setDisabled(true);
  }
}

void KGameWindow::slotTcpConnectCancel()
{
  showMainMenu();
}

void KGameWindow::slotTcpConnectPrevious()
{
  showMainMenu();
}


} // closing namespace Ksirk
