/* This file is part of KsirK.
   Copyright (C) 2004-2007 Gael de Chalendar <kleag@free.fr>

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

#include "gameautomaton.h"

#include "ksirksettings.h"
#include "kgamewin.h"
#include "aiplayer.h"
#include "aiColsonPlayer.h"
#include "aiplayerio.h"
#include "onu.h"
#include "dice.h"
#include "goal.h"
#include "country.h"
#include "KMessageParts.h"
#include "newgamesetup.h"
#include "krightdialog.h"
#include "Dialogs/joingame.h"
#include "newplayerdata.h"
#include "gamesequence.h"

#include <QLayout>
#include <QSpinBox>
#include <QPixmap>
#include <QMouseEvent>
#include <QFile>
#include <QUuid>
#include <QInputDialog>

#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KLineEdit>
#include <KMessageBox>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kmessageclient.h>
#include <libkdegamesprivate/kgame/kmessageserver.h>
#include <libkdegamesprivate/kgame/kgamechat.h>

#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <QCache>

namespace Ksirk{
namespace GameLogic {

const char* GameAutomaton::GameStateNames[] = {
    "INIT",
    "INTERLUDE",
    "NEWARMIES",
    "WAIT",
    "WAIT1",
    "WAIT_RECYCLING",
    "ATTACK",
    "ATTACK2",
    "INVADE",
    "SHIFT1",
    "SHIFT2",
    "FIGHT_BRING",
    "FIGHT_ANIMATE",
    "FIGHT_BRINGBACK",
    "WAITDEFENSE",
    "EXPLOSION_ANIMATE",
    "WAIT_PLAYERS",
    "GAME_OVER",
    "INVALID",
    "STARTING_GAME"
};

const char* GameAutomaton::KsirkMessagesIdsNames[] = {
"CountryOwner", // 257
"PlayerPutsArmy", // 258
"StateChange", // 259
"PlayerChange", // 260
"RegisterCountry", // 261
"PlayerAvailArmies", // 262
"ResetPlayersDistributionData", // 263
"ChangeItem", // 264
"DisplayRecyclingButtons", // 265
"ClearHighlighting", // 266
"ActionRecycling", // 267
"ClearGameActionsToolbar", // 268
"DisplayDefenseButtons", // 269
"ActionDefense", // 270
"FirstCountry", // 271
"SecondCountry", // 272
"InitCombatMovement", // 273
"AnimCombat", // 274
 // 275
"TerminateAttackSequence", // 276
"DecrNbArmies", // 277
"StartLocalCurrentAI", // 278
"Invade", // 279
"Retreat", // 280
"NextPlayerNormal", // 281
"NextPlayerRecycling", // 282
"ShowArmiesToPlace", // 283
"PlayerPutsInitialArmy", // 284
"PlayerRemovesArmy", //285
"VoteRecyclingFinished", // 286
"CancelShiftSource", // 287
"ChangePlayerNation", // 288
"ChangePlayerName", // 289
"StartGame", // 290
"SetNation", // 291
"SetBarFlagButton", // 292
"FinishMoves", // 293
"AnimExplosion", // 294
"SetupOnePlayer", // 295
"SetupWaitedPlayer", // 296
"ValidateWaitedPlayerPassword", // 297
"ValidPassword", // 298
"InvalidPassword", // 299
"SetupCountries", // 300
"AddMsgIdPair", // 301
"CheckGoal", // 302
"SetGoalFor", // 303
"GoalForIs", // 304
"Winner", // 305
"NbPlayers", // 306
"FinalizePlayers", // 307
"Acknowledge", // 308
"DisplayGoals", // 309
"DisplayFightResult", // 310
"MoveSlide", // 311
"InvasionFinished", // 312
"AttackAuto", // 313
"DisplayRecycleDetails", // 314
"CurrentPlayerPlayed", // 315
"NewGameSetupMsg", // 316
};

#define KSIRK_DEFAULT_PORT 20000

GameAutomaton::GameAutomaton() :
    KGame(),
    m_aicannotrunhack(true),
    m_state(INIT),
    m_game(nullptr),
    m_networkPlayersNumber(0),
    m_currentPlayer(""),
    m_currentPlayerPlayed(false),
    m_savedState(INVALID),
    m_goals(),
    m_useGoals(true),
    m_attackAuto(false),
    m_defenseAuto(false),
    m_port(KSIRK_DEFAULT_PORT),
    m_startingGame(false)
{
  m_skin = "skins/default";
  //   qCDebug(KSIRK_LOG);
//   m_stateId = m_state.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_state"));
  m_skinId = m_skin.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_skin"));
//   m_currentPlayerId = m_currentPlayer.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_currentPlayer"));
//   m_events.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_events"));

    setGameSequence(new GameSequence(this));

  // Connect the most important slot which tells us which properties are
  // changed
  connect(this,&KGame::signalPropertyChanged,
          this,&GameAutomaton::slotPropertyChanged);
  
  connect(this,&KGame::signalPlayerJoinedGame,
          this,&GameAutomaton::slotPlayerJoinedGame);
  
  connect(this,&KGame::signalNetworkData,
          this,&GameAutomaton::slotNetworkData);
  
  connect(this,&KGame::signalClientJoinedGame,
          this,&GameAutomaton::slotClientJoinedGame);
  
  connect(messageClient(),&KMessageClient::connectionBroken,
          this,&GameAutomaton::slotConnectionToServerBroken);
  
  connect(messageServer(),&KMessageServer::connectionLost,
          this,&GameAutomaton::slotConnectionToClientBroken);

  setPolicy(KGame::PolicyDirty,true);
  
//   qCDebug(KSIRK_LOG) << "finished";
}

GameAutomaton::~GameAutomaton()
{
  qCDebug(KSIRK_LOG);
  qDeleteAll(m_goals);
}

void GameAutomaton::init(KGameWindow* gw)
{
  m_game = gw;
}

GameAutomaton::GameState GameAutomaton::state() const
{
  return m_state;
}
    
void GameAutomaton::state(GameAutomaton::GameState state) 
{
  qCDebug(KSIRK_LOG) << "new state (id=" << state << ") is " << GameStateNames[state];
  m_state = state;
  m_game->setSaveGameActionEnabled(m_state == WAIT);
  m_game->setContextualHelpActionEnabled(m_state, currentPlayer() && currentPlayer()->isAI());
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << state;
  sendMessage(buffer,StateChange);
}

Player* GameAutomaton::getAnyLocalPlayer()
{
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    if ( !((Player*)(*it))->isVirtual() )
    {
      return (Player*)(*it);
    }
  }
  return nullptr;
}

GameAutomaton::GameState GameAutomaton::run()
{
//   qCDebug(KSIRK_LOG) << "(KGame running=" <<  (gameStatus()==KGame::Run) << ")";
  if (m_game == nullptr || gameStatus() == KGame::Pause)
  {
    QTimer::singleShot(200, this, &GameAutomaton::run);
    return m_state;
  }
      
  activateNeededAIPlayers();

  QString event = "";
  QPointF point;
  if (!m_events.empty())
  {
    QPair< QString, QPointF > pair = m_events.front();
    event = pair.first;
    point = pair.second;
    m_events.pop_front();
  }

//   qCDebug(KSIRK_LOG) << "Handling " << stateName() << " ; " << event << " ; " << point;
//   if (currentPlayer())
//   {
//     qCDebug(KSIRK_LOG) << "current player=" << currentPlayer()->name() << " is active=" << currentPlayer()->isActive();
//   }
  if (event == "requestForAck")
  {
    qCDebug(KSIRK_LOG) << "requestForAck";
  }
  if((event == "actionRButtonDown" || event == "actionLButtonDown") && (m_state != INIT && m_state != NEWARMIES && m_state != INTERLUDE && m_state != WAIT_RECYCLING))
  {
     if (m_game->getRightDialog()->isOpen())
     {
        m_game->getRightDialog()->close();
     }
  }

  if (event == "actionNewGame")
  {
    if (m_game->actionNewGame(GameAutomaton::None))
    {
      state(INIT);
      QTimer::singleShot(200, this, &GameAutomaton::run);
      return INIT;
    }
    else
    {
      QTimer::singleShot(200, this, &GameAutomaton::run);
      return m_state;
    }
  }
  if (event == "actionOpenGame")
  {
    if (m_game->actionOpenGame())
    {
      qCDebug(KSIRK_LOG) << "opened";
      bool ok;
      m_port = QInputDialog::getInt(m_game,
              i18nc("@title:window", "Network Configuration"),
              i18n("Please type in the port number on which to offer connections:"), 
              m_port, 0, 32000, 1, &ok);
      offerConnections(m_port);
      state(WAIT_PLAYERS);
      QTimer::singleShot(200, this, &GameAutomaton::run);
      return WAIT_PLAYERS;
    }
    else
    {
      qCDebug(KSIRK_LOG) << "opened";
      QTimer::singleShot(200, this, &GameAutomaton::run);
      return m_state;
    }
  }
  if (event == "actionJoinNetworkGame")
  {
    joinNetworkGame();
    QTimer::singleShot(200, this, &GameAutomaton::run);
    return m_state;
  }

  switch (m_state)
  {
  case INIT:
    if (currentPlayer() != nullptr && isAdmin())
    {
      if  ( (event == "actionLButtonDown") && (m_game->playerPutsInitialArmy(point)) )
      {
        m_choosedToRecycleNumber = 0;
        m_game->initRecycling();
        state(WAIT_RECYCLING);
      }
    }
    break;
  case ATTACK:
        state(ATTACK2);
    break;
  case ATTACK2:
    qCDebug(KSIRK_LOG) << "Handling ATTACK2";
    if (isAdmin())
    {
      QByteArray buffer;
      switch ( m_game->attacked(point) )
      {
        case 0:
          qCDebug(KSIRK_LOG) << "handling attacked value; 0";
          state(WAIT);
        break;
        case 1:
          qCDebug(KSIRK_LOG) << "handling attacked value; 1";
          sendMessage(buffer,CurrentPlayerPlayed);
          state(WAITDEFENSE);
        break;
        case 2:
          qCDebug(KSIRK_LOG) << "handling attacked value; 2";
          sendMessage(buffer,CurrentPlayerPlayed);
          qCDebug(KSIRK_LOG) << "calling defense(1)";
          m_game-> defense(1);
          qCDebug(KSIRK_LOG) << "setting state to FIGHT_BRING";
          state(FIGHT_BRING);
        break;
        case 3:
          qCDebug(KSIRK_LOG) << "handling attacked value; 3";
          // AI action: nothing to do.
        break;
        default:
          qCCritical(KSIRK_LOG) << "Unknown return value from attacked";
          exit(1);
      }
    }
    qCDebug(KSIRK_LOG) << "handling of ATTACK2 finished !";
  break;
  case EXPLOSION_ANIMATE:
  break;
  case FIGHT_ANIMATE:
  break;
  case FIGHT_BRING:
  break;
  case FIGHT_BRINGBACK:
    // no more moving fighter returning home

//     qCDebug(KSIRK_LOG) << "$$$$$$$STATE FIGHT_BRINGBACK $$$$$$$$$$$" << m_game->haveAnimFighters();

    if (!m_game->haveAnimFighters() && isAdmin())
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      sendMessage(buffer,TerminateAttackSequence);
    }
    break;
  case INTERLUDE:
    if  (event == "playersLooped")
    {
      m_choosedToRecycleNumber = 0;
      m_game->initRecycling();
      state(WAIT_RECYCLING);
    }
    else if  (event == "actionLButtonDown" ) 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << point;
//       qCDebug(KSIRK_LOG) << "Sending message PlayerPutsArmy " << point ;
      sendMessage(buffer,PlayerPutsInitialArmy);
    }
    else if (event == "actionRButtonDown") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << point;
      sendMessage(buffer,PlayerRemovesArmy);
    }
    else if (event == "actionNextPlayer") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << -1;
      sendMessage(buffer,NextPlayerRecycling);
    }
    else if (event == "actionRecycling") 
    {
      QByteArray buffer;
      sendMessage(buffer,ActionRecycling);
    }
    else if (event == "actionRecyclingFinished") 
    {
      qCDebug(KSIRK_LOG) << "actionRecyclingFinished";
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
//       stream << currentPlayer()->name();
      PlayersArray::iterator it = playerList()->begin();
      PlayersArray::iterator it_end = playerList()->end();
      quint32 nbLocal = 0;
      for (; it != it_end; it++)
      {
        if ( !((Player*)(*it))->isVirtual() )
        {
          qCDebug(KSIRK_LOG) << "Local:" << ((Player*)(*it))->name();
          nbLocal++;
        }
      }
      qCDebug(KSIRK_LOG) << "Nb Local:" << nbLocal;
      stream << nbLocal;

      for (it = playerList()->begin(); it != it_end; ++it)
      {
        if ( !((Player*)(*it))->isVirtual() )
        {
          stream << ((Player*)(*it))->id();
        }
      }
      sendMessage(buffer,VoteRecyclingFinished);
    }
    else
    {
      //        if (!event.isEmpty())
//          qCCritical(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
    }
  break;
  case INVADE:
//     qCDebug(KSIRK_LOG) << "$$$$$$$STATE INVADE$$$$$$$$$$$";
    if (event == "actionInvade1")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,Invade);
    }
    else if (event == "actionInvade5")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(5);
      sendMessage(buffer,Invade);
    }
    else if (event == "actionInvade10")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(10);
      sendMessage(buffer,Invade);
    }
    else if (event == "actionRetreat1")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,Retreat);
    }
    else if (event == "actionRetreat5")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(5);
      sendMessage(buffer,Retreat);
    }
    else if (event == "actionRetreat10")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(10);
      sendMessage(buffer,Retreat);
    }
    else if (event == "actionInvasionFinished")
    {
      state(WAIT);
      m_game-> invasionFinished();
    }
    else
    {
      //        if (!event.isEmpty())
//          qCCritical(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
    }
  break;
  case NEWARMIES:
    if  (event == "actionLButtonDown") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << point << quint32(true);
      sendMessage(buffer,PlayerPutsArmy);
    }
    else if  (event == "actionRButtonDown") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << point;
      sendMessage(buffer,PlayerRemovesArmy);
    }
    else if (event == "actionNextPlayer") 
    { 
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(WAIT);
      sendMessage(buffer,NextPlayerRecycling);
    }
//     else
//     {
//              if (!event.isEmpty())
//          qCCritical(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
//     }
  break;
  case SHIFT1:
    if (event == "actionCancel")
    {
      m_game-> cancelAction();
      state(WAIT);
    }
    else if (event == "actionLButtonDown")
    {
      m_game->firstCountryAt(point);
    }
    else if (event == "actionLButtonUp")
    {
      m_game->secondCountryAt(point);
      if (m_game->isMoveValid(point))
      {
        m_game->startLocalCurrentAI();
        QByteArray buffer;
        sendMessage(buffer,CurrentPlayerPlayed);
        state(SHIFT2);
      }
      else
      {
//         state(WAIT);
      }
    }
    else
    {
      //        if (!event.isEmpty())
//          qCCritical(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
    }
  break;
  case SHIFT2:
//     qCDebug(KSIRK_LOG) << "$$$$$$$STATE SHIFT2$$$$$$$$$$$";
    if (event == "actionInvade1")
    {
//       qCDebug(KSIRK_LOG) << "actionInvade1";
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,Invade);
    }
    else if (event == "actionInvade5")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(5);
      sendMessage(buffer,Invade);
    }
    else if (event == "actionInvade10")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(10);
      sendMessage(buffer,Invade);
    }
    else if (event == "actionRetreat1")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,Retreat);
    }
    else if (event == "actionRetreat5")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(5);
      sendMessage(buffer,Retreat);
    }
    else if (event == "actionRetreat10")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(10);
      sendMessage(buffer,Retreat);
    }
    else if (event == "actionInvasionFinished")
    {
      m_game-> shiftFinished();
      state(WAIT);
    }
    else if (event == "actionCancel")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      sendMessage(buffer,CancelShiftSource);
    }
    else
    {
      //qCCritical(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
    }
  break;
  case WAIT_RECYCLING:
    if (event == "actionRecycling")
    {
      QByteArray buffer;
      sendMessage(buffer,ActionRecycling);
    }
    else if (event == "actionRecyclingFinished") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
//       stream << currentPlayer()->name();
      PlayersArray::iterator it = playerList()->begin();
      PlayersArray::iterator it_end = playerList()->end();
      quint32 nbLocal = 0;
      for (; it != it_end; it++)
      {
        if ( !((Player*)(*it))->isVirtual() )
        {
          nbLocal++;
        }
      }
      stream << nbLocal;
      it = playerList()->begin();
      it_end = playerList()->end();
      for (; it != it_end; it++)
      {
        if ( !((Player*)(*it))->isVirtual() )
        {
          stream << (*it)->id();
        }
      }
      sendMessage(buffer,VoteRecyclingFinished);
    }
    else
    {
      if (!event.isEmpty())
        qCDebug(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
      else
      {
      if (allLocalPlayersComputer())
        {
          m_game->getRightDialog()->updateRecycleDetails(nullptr,true,0);
          m_game->displayRecyclingButtons();
        }
      }
    }
    break;
  case WAITDEFENSE:
    if (event == "actionDefense1")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,ActionDefense);
    }
    else if (event == "actionDefense2")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(2);
      sendMessage(buffer,ActionDefense);
    }
    else if ( m_game->secondCountry() != nullptr
          && !m_game->secondCountry()->owner()->isVirtual()
          && isDefenseAuto()
          && (m_game->secondCountry()->owner()->getNbDefense() == 0) )
    {
      quint32 nbDefense = 1;
      if (m_game->secondCountry()->nbArmies() > 1)
      {
        nbDefense = 2;
      }
      m_game->secondCountry()->owner()->setNbDefense(nbDefense);
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << nbDefense;
      sendMessage(buffer,ActionDefense);
    }
    else
    {
      //        if (!event.isEmpty())
         qCDebug(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
    }
  break;
  case WAIT:
    if (event == "actionNextPlayer") 
    {
      actionNextPlayer();
    }
    else if (event == "actionLButtonDown")
    {
      if (m_game->firstCountryAt(point))
        state(WAIT1);
    }
    else
    {
      //        if (!event.isEmpty())
//          qCCritical(KSIRK_LOG) << "Unhandled event " << event << " during handling of " << stateName();
    }
    // other case : state doesn't change
    break;
  case WAIT1:
    if (event == "actionNextPlayer")
    {
      actionNextPlayer();
    }
    else if (event == "actionAttack1")
    {
      m_game->attack(1);
      state(ATTACK);
    }
    else if (event == "actionAttack2")
    {
      m_game->attack(2);
      state(ATTACK);
    }
    else if (event == "actionAttack3")
    {
      m_game->attack(3);
      state(ATTACK);
    }
    else if (event == "actionInvade1")
    {
//       qCDebug(KSIRK_LOG) << "actionInvade1";
      QByteArray buffer2;
      sendMessage(buffer2,CurrentPlayerPlayed);
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,Invade);
//       state(WAIT);
    }
    else if (event == "actionInvade5")
    {
      QByteArray buffer2;
      sendMessage(buffer2,CurrentPlayerPlayed);
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(5);
      sendMessage(buffer,Invade);
//       state(WAIT);
    }
    else if (event == "actionInvade10")
    {
      QByteArray buffer2;
      sendMessage(buffer2,CurrentPlayerPlayed);
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(10);
      sendMessage(buffer,Invade);
//       state(WAIT);
    }
    else if (event == "actionInvasionFinished")
    {
      m_game-> shiftFinished();
      state(WAIT);
    }
    else if (event == "actionLButtonUp")
    {
      qCDebug(KSIRK_LOG) << "actionLButtonUp in WAIT1";
      
      if (!currentPlayer()-> isAI())
      {
        if (m_game->isMoveValid(point) && m_game->firstCountry()->nbArmies() !=1)
        {
          m_game->secondCountryAt(point);
          state(WAIT);
          qCDebug(KSIRK_LOG) << "Sending MoveSlide";
          QByteArray buffer;
          QDataStream stream(&buffer, QIODevice::WriteOnly);
          sendMessage(buffer,MoveSlide);
        }
        else if (m_game->isFightValid(point)
                && m_game->firstCountry()->nbArmies() != 1)
        {
          m_game->secondCountryAt(point);
          if (m_game->firstCountry()->nbArmies() > 3)
          {
            m_game->frame()->getAttack1Action()->setVisible(true);
            m_game->frame()->getAttack2Action()->setVisible(true);
            m_game->frame()->getAttack3Action()->setVisible(true);

          }
          else if (m_game->firstCountry()->nbArmies() > 2)
          {
            m_game->frame()->getAttack1Action()->setVisible(true);
            m_game->frame()->getAttack2Action()->setVisible(true);
            m_game->frame()->getAttack3Action()->setVisible(false);
          }
          else if (m_game->firstCountry()->nbArmies() > 1)
          {
            m_game->frame()->getAttack1Action()->setVisible(true);
            m_game->frame()->getAttack2Action()->setVisible(false);
            m_game->frame()->getAttack3Action()->setVisible(false);
          }
          m_game->frame()->setMenuPoint(QCursor::pos());
          m_game->frame()->getAttackContextMenu()->exec(QCursor::pos());
        }
        else
        {
          m_game-> cancelAction();
          state(WAIT);
        }
      }
      else
      {
        m_game->secondCountryAt(point);
      }
    }
    else if (event == "actionLButtonDown")
    {
      m_game-> cancelAction();
      if (m_game->firstCountryAt(point))
      {
        state(WAIT1);
      }
      state(WAIT);
    }
   break;
  /*case WAIT_INPUT:
    
    break;*/   
  case WAIT_PLAYERS:
    break;
  case GAME_OVER:
    break;
  case STARTING_GAME:
    break;
  default:
    qCCritical(KSIRK_LOG) << "Unhandled state: " << stateName() << ". Event was: " << event ;
    exit(1); // @todo handle this error
  }

  QTimer::singleShot(200, this, &GameAutomaton::run);

//   m_game->initTimer();
  return m_state;
}

void GameAutomaton::activateNeededAIPlayers()
{
//   qCDebug(KSIRK_LOG);
  if ( currentPlayer() 
       && (currentPlayer()-> isAI() ) 
       && (!currentPlayer()->isVirtual())
       && (!(dynamic_cast< AIPlayer* >(currentPlayer())-> isRunning()))
     )
  {
    dynamic_cast< AIPlayer* >(currentPlayer())-> start();
  }
  if (    ( m_state == WAITDEFENSE ) 
       && ( m_game->secondCountry()) 
       && ( m_game->secondCountry()->owner())
       && ( m_game->secondCountry()->owner()->isAI() ) 
       && ( !m_game->secondCountry()->owner()->isVirtual())
       && ( !(dynamic_cast< AIPlayer* >(m_game->secondCountry()->owner())-> isRunning()))
     )
  {
    dynamic_cast< AIPlayer* >(m_game->secondCountry()->owner())-> start();
  }
}

void GameAutomaton::gameEvent(const QString& event, const QPointF& point)
{
  m_events.push_back(qMakePair(event, point));
}


/** returns the name of the current state */
QString GameAutomaton::stateName() const
{
  if (m_state >= (int) sizeof(GameStateNames))
  {
    QString string;
    QTextStream oss(&string);
    oss << "Invalid stored state id: " << m_state;
    qCCritical(KSIRK_LOG) << string ;
    return string;
  }
  else
  {
    return QString::fromUtf8(GameStateNames[m_state]);
  }
}

void GameAutomaton::saveXml(QTextStream& xmlStream)
{
    xmlStream << "<gameautomaton state=\"" << GameStateNames[m_state] << "\" />";
}

const QString& GameAutomaton::skin() const
{
  return m_skin.value();
}

void GameAutomaton::skin(const QString& newSkin)
{
  m_skin = newSkin;
}

// Called when a player input (e.g. a mouse event) is received from the KGame 
// object
// This is obviously the central function in the game as all player moves, 
// whether network or local, end up here. So do something sensible here. 
bool GameAutomaton::playerInput(QDataStream &msg, KPlayer* player)
{
  qCDebug(KSIRK_LOG);
//   if (player->isVirtual())
  if (!isAdmin())
  {
//     qCDebug(KSIRK_LOG) << "Network player: nothing to do";
    return false;
  }

  // Convert the player to the right class
  Player* p = dynamic_cast<Player*>(player);

  QString action;
  QPointF point;
  msg >> action >> point;

  qCDebug(KSIRK_LOG) << " =======================================================";
  qCDebug(KSIRK_LOG)  << "Player " << p->name() << " id=" << player->id()
  << " uid=" << player->userId() << " : " << action << " at " << point
  << "current is" << currentPlayer()->name();
  
  if (p->name() == currentPlayer()->name()
    || (m_state == WAITDEFENSE) )
  {
    if (action == "actionLButtonDown")
      m_game->slotLeftButtonDown( point );
    else if (action == "actionLButtonUp")
      m_game->slotLeftButtonUp( point );
    else if (action == "actionRButtonDown")
      m_game->slotRightButtonDown( point );
    else if (action == "actionRButtonUp")
      m_game->slotRightButtonUp( point );
    else if (action == "zoomInAction")
      m_game->slotZoomIn();
    else if (action == "zoomOutAction")
      m_game->slotZoomOut();
    else if (action == "actionAttack1")
      m_game->slotAttack1();
    else if (action == "actionAttack2")
      m_game->slotAttack2();
    else if (action == "actionAttack3")
      m_game->slotAttack3();
    else if (action == "actionMove")
      m_game->slotMove();
    else if (action == "slotRecyclingFinished")
      m_game->slotRecyclingFinished();
    else if (action == "actionInvade10")
      m_game->slotInvade10();
    else if (action == "actionInvade5")
      m_game->slotInvade5();
    else if (action == "actionInvade1")
      m_game->slotInvade1();
    else if (action == "actionInvasionFinished")
      m_game->slotInvasionFinished();
    else if (action == "slotDefense1")
      m_game->slotDefense1();
    else if (action == "slotDefense2")
      m_game->slotDefense2();
    else if (action == "actionNextPlayer")
      m_game->slotNextPlayer();
  }
  if (action == "requestForAck")
  {
    QString ack;
    msg >> ack;
    qCDebug(KSIRK_LOG) << "acknowledging " << ack;
    if (p->isVirtual())
    {
      qCDebug(KSIRK_LOG) << p->name() << "is virtual; sending message";
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << p->id() << ack;
      sendMessage(buffer,Acknowledge);
    }
    else
    {
      qCDebug(KSIRK_LOG) << p->name() << "is local; acknowledging";
      p->acknowledge(ack);
    }
  }
  return false;
}

// Create an IO device for the player. We could create any
// device here, e.g. mouse, keyboard, computer player. In the
// demo game here we have only the mouse player available.
void GameAutomaton::createIO(KPlayer *player,KGameIO::IOMode io)
{
  // Error check
  if (!player) return;

  qCDebug(KSIRK_LOG) << "createIO for " << player->name();

  if (io&KGameIO::MouseIO)
  {
    // Create new game mouse input
    KGameMouseIO *input;
    // We want the player to work over mouse
    // in our canvas view
    input=new KGameMouseIO(m_game->frame()->scene());

    // Connect mouse input to a function to process the actual input
    connect(
      input,
      &KGameMouseIO::signalMouseEvent,
      m_game->frame(),
      &DecoratedGameFrame::slotMouseInput);

    // Add the device to the player
    player->addGameIO(input);
  }
  else if (io&AIPLAYERIO)
  {
    if (dynamic_cast<AIPlayer*>(player) != nullptr)
    {
      /*AIPlayerIO* input =*/ new AIPlayerIO(dynamic_cast<AIPlayer*>(player));
    }
    else
    {
      qCCritical(KSIRK_LOG) << "Can create an AIPlayerIO only for AI players: " << io ;
    }
  }
  else
  {
    qCCritical(KSIRK_LOG) << "Cannot create the requested IO device " << io ;
  }
  qCDebug(KSIRK_LOG) << "Done createIO for " << player->name();
}

// Find out who will be the next player
// Note: The default behaviour as we have it here is done automatically
//        by the lib, too. So if all players player one after the other
//        this functions is NOT needed at all.
KPlayer * GameAutomaton::doNextPlayer(KPlayer */*last*/,bool /*exclusive*/)
{
//   qCDebug(KSIRK_LOG) << last->name();
//   m_game->setCurrentPlayerToNext();
  // If a last player is given switch the player
  
  // Should be enough if the admin sets the turn (unclear)
  if (isAdmin())
  {
    currentPlayer()->setTurn(true,true);
//     last->setTurn(true,true);
//     qCDebug(KSIRK_LOG) << "nextPlayer::Setting turn to " << last->name() << ", " << last->id() << "("<<last->userId()<<")";
  }
  
  // Notify the world that whose turn it is. The main window uses
  // this to show a little message
//   emit signalNextPlayer(next->userId().value(),next,this);
  
  // Return the next player
  return dynamic_cast<KPlayer *>(currentPlayer());
}

QDataStream& operator>>(QDataStream& s, GameAutomaton::GameState& state)
{
  int istate;
  s >> istate;
  state = GameAutomaton::GameState(istate);
  return s;
}


bool GameAutomaton::setupPlayersNumberAndSkin(NetworkGameType netGameType)
{
  qCDebug(KSIRK_LOG) << netGameType;
  m_netGameType = netGameType;
  m_game->newGameDialog(m_skin.value(), m_netGameType);

//   m_networkPlayersNumber = ???;
  return false;
}

bool GameAutomaton::finishSetupPlayersNumberAndSkin()
{
  qCDebug(KSIRK_LOG);
  
  setUseGoals(m_game->newGameSetup()->useGoals());
  state(GameLogic::GameAutomaton::INIT);
  savedState(GameLogic::GameAutomaton::INVALID);
  setNetworkPlayersNumber(m_game->newGameSetup()->nbNetworkPlayers());
  m_startingGame = true;
  state(INIT);
  setMinPlayers(m_game->newGameSetup()->nbPlayers());
  setMaxPlayers(m_game->newGameSetup()->nbPlayers());
  m_nbPlayers = m_game->newGameSetup()->nbPlayers();
  
  return true;
}

void GameAutomaton::setGoalFor(Player* player)
{
  qCDebug(KSIRK_LOG) << player->name();
  unsigned int max = m_goals.size();
  unsigned int goalId = Dice::roll(max);
  QList< Goal* >::iterator it = m_goals.begin();
  for (unsigned int i = 1 ; i < goalId; it++,i++) {}
  Goal* goal = (*it);
  qCDebug(KSIRK_LOG) << "Goal for " << player->name() << " is of type " << goal->type();
  if (goal->type() == Goal::GoalPlayer)
  {
    Player* target = nullptr;
    while (target==nullptr || target->id() == player->id())
    {
      unsigned int max = playerList()->count();
      unsigned int playerNum = Dice::roll(max);
//       qCDebug(KSIRK_LOG) << "Choice player num " << playerNum << " on " << max ;
      PlayersArray::iterator itp = playerList()->begin();
      unsigned int j = 1;
      for (; j < playerNum; j++,itp++) {}
      target = dynamic_cast< Player* >(*itp);
    }
    if (target != nullptr)
    {
//       qCDebug(KSIRK_LOG) << "Target choice for " << player->name() << ": " << target->name();
      goal->players().push_back(target->name());
    }
    else
    {
//       qCDebug(KSIRK_LOG) << "No target chosen for " << player->name();
    }
  }
  /// @note hack to avoid too easy countries goal when there is only two players
  else if (goal->type() == Goal::Countries 
    && playerList()->count() == 2)
  {
    goal->nbCountries(int(goal->nbCountries()*1.5));
  }
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << player->id();
  stream << (*goal);
  qCDebug(KSIRK_LOG) << "Sending message GoalForIs ("<<GoalForIs<<") for " << player->name(); 
  sendMessage(buffer,GoalForIs);
//   delete goal;
  m_goals.erase(it);
}

Country * GameAutomaton::getDefCountry ()
{
  return this->defCountry;
}

bool GameAutomaton::joinNetworkGame()
{
  qCDebug(KSIRK_LOG);
   if (stateName() == "INIT"
     || (KMessageBox::warningContinueCancel(m_game,i18n("Do you really want to end your current game and join another?"), i18nc("@title:window", "New Game Confirmation"),KGuiItem(i18nc("@action:button", "Join New Game"))) == KMessageBox::Continue))
   {
      m_game->joinNetworkGame();
   }
   return false;
}

bool GameAutomaton::connectToServ()
{
  qCDebug(KSIRK_LOG);
  if (messageServer() != nullptr)
  {
    QObject::disconnect(messageServer(),&KMessageServer::connectionLost,
                        this,&GameAutomaton::slotConnectionToClientBroken);
  }
  qCDebug(KSIRK_LOG) << "Before connectToServer";
  QString host = m_game->newGameSetup()->host();
  int port = m_game->newGameSetup()->tcpPort();
  bool status = connectToServer(host, port);
  qCDebug(KSIRK_LOG) << "After connectToServer" << status;
  if (messageServer())
    connect(messageServer(),&KMessageServer::connectionLost,
          this,&GameAutomaton::slotConnectionToClientBroken);
  return status;
}

KPlayer * GameAutomaton::createPlayer(int rtti, 
                                    int /*io*/, 
                                    bool isVirtual)    
{
  qCDebug(KSIRK_LOG) << "(" << rtti << ", " << isVirtual << ")";
  if (rtti == 1)
  {
    Player* p = new Player(this, "", 0, nullptr);
    p->setVirtual(isVirtual);
    if (!isVirtual)
    {
      qCDebug(KSIRK_LOG) << "Calling player createIO";
      createIO(p,KGameIO::IOMode(KGameIO::MouseIO));
    }
    return (KPlayer*) p;
  }
  else if (rtti == 2)
  {
    AIPlayer* aip = new AIColsonPlayer("", 0, nullptr,  *playerList(), m_game->theWorld(),
                                   this);
    aip->stop();
    aip->setVirtual(isVirtual);
    if (!isVirtual)
    {
      qCDebug(KSIRK_LOG) << "Calling player createIO";
      createIO(aip, KGameIO::IOMode(AIPLAYERIO));
    }
    return (KPlayer*) aip;
  }
  else 
  {
    qCCritical(KSIRK_LOG) << "No rtti given... creating a Player";
    Player* p = new Player(this, "", 0, nullptr);
    p->setVirtual(isVirtual);
    if (!isVirtual)
    {
      qCDebug(KSIRK_LOG) << "Calling player createIO";
      createIO(p,KGameIO::IOMode(KGameIO::MouseIO));
    }
    return (KPlayer*) p;
  }
}

/**
  * @return A pointer to the given's named player ; 0 if there is no such player
  */
Player* GameAutomaton::playerNamed(const QString& playerName)
{
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    if ( (*it)-> name() ==  playerName)
    {
      return dynamic_cast<Player*>(*it);
    }
  }
  qCCritical(KSIRK_LOG) << "GameAutomaton::playerNamed: there is no player named " 
      << playerName ;
  return nullptr;
}

Player* GameAutomaton::currentPlayer() 
{
  if (m_game && !m_currentPlayer.isEmpty())
  {
    return playerNamed(m_currentPlayer);
  }
  else return nullptr;
}

void GameAutomaton::currentPlayer(Player* player) 
{
  qCDebug(KSIRK_LOG);
  if (player)
  {
    qCDebug(KSIRK_LOG) << " name = " << player->name();
    m_currentPlayer = player->name();
    m_currentPlayerPlayed = false;
    if (isAdmin())
    {   
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << m_currentPlayer;
      sendMessage(buffer,PlayerChange);
      player->setTurn(true,true);
    }
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << player->name();
    sendMessage(buffer,SetBarFlagButton);
    if (!player->isVirtual())
    {
      m_game->chatWidget()->setFromPlayer(player);
    }
  }
  else 
    m_currentPlayer = "";
}

void GameAutomaton::slotPlayerJoinedGame(KPlayer* player)
{
  qCDebug(KSIRK_LOG) << "currently " << playerList()->count() << " / " << maxPlayers();
  Player* p = dynamic_cast<Player*>(player);
  Q_ASSERT(p);
  
  if (isAdmin())
  {
    unsigned int nbWithNation = 0;
    unsigned int nbWithName = 0;
    PlayersArray::iterator it = playerList()->begin();
    PlayersArray::iterator it_end = playerList()->end();
    for (; it != it_end; it++)
    {
      if (p->getNation()->name() == ((Player*)(*it))->getNation()->name())
      {
        nbWithNation++;
      }
      if (p->name() == ((Player*)(*it))->name())
      {
        nbWithName++;
      }
    }
    if (nbWithName != 1)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(player->id());
      qCDebug(KSIRK_LOG) << "Sending ChangePlayerName for player id " << player->id();
      sendMessage(buffer,ChangePlayerName);
      
      return;
    }
    else if (nbWithNation != 1)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << player->id();
      qCDebug(KSIRK_LOG) << "Sending ChangePlayerNation for player id " << player->id();
      sendMessage(buffer,ChangePlayerNation);
      
      return;
    }
    KMessageParts messageParts;
    messageParts 
      << kli18n("%1 (%2) joined game ; waiting for %3 players to connect").untranslatedText()
      << p-> name() << p->getNation()->name() 
      << QString::number(maxPlayers() - int(playerList()->count()));
    m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    qCDebug(KSIRK_LOG) << "Sending StartGame";
    sendMessage(buffer,StartGame);
  }
  else
  {
    if (!player->isVirtual())
    {
      m_game->showNewGameSummary();
    }
    m_game->newGameSetup()->setNbPlayers(maxPlayers());
  }
  NewPlayerData* pd = new NewPlayerData(p->name(),p->getNation()->name(),"",p->isAI(),true);
  if (!m_game->newGameSetup()->addPlayer(pd))
  {
    delete pd;
  }
  m_game->updateNewGameSummary();
}

bool GameAutomaton::startGame()
{
  qCDebug(KSIRK_LOG) << stateName() << "nb players = " << playerList()->count() << " / " << maxPlayers();
  m_aicannotrunhack = true;
  //   qCDebug(KSIRK_LOG) << "  state is " << GameStateNames[m_state];
//   qCDebug(KSIRK_LOG) << "  saved state is " << GameStateNames[m_savedState];
  if (isAdmin() && int(playerList()->count()) == maxPlayers()
      && gameStatus()!=KGame::Run)
  {
//     m_game->haltTimer();

    if (m_state == INIT && m_savedState == INVALID)
    {
      firstCountriesDistribution();

      if (useGoals())
      {
        PlayersArray::iterator it = playerList()->begin();
        PlayersArray::iterator it_end = playerList()->end();
        for (; it != it_end; it++)
        {
          QByteArray buffer;
          QDataStream stream(&buffer, QIODevice::WriteOnly);
          stream << (*it)->id();
          sendMessage(buffer,SetGoalFor);
        }
      }
    }
    else if (m_state == WAIT_PLAYERS)
    {
      sendCountries();
      qCDebug(KSIRK_LOG) << "at " <<  __FILE__ << ", line " << __LINE__ << ", setting state to " << m_savedState ;
      state(m_savedState);
      currentPlayer(playerNamed(m_savedPlayer));
      m_game->displayButtonsForState(m_savedState);
      m_savedPlayer = "";
      m_savedState = INVALID;
    }

    qCDebug(KSIRK_LOG) << "Sending message FinalizePlayers";
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    sendMessage(buffer,FinalizePlayers);

    qCDebug(KSIRK_LOG) << "Setting game status to Run";
    setGameStatus(KGame::Run);
//     m_game->initTimer();
//     qCDebug(KSIRK_LOG) << "    true";
    return true;
  }
  else
  {
//     qCDebug(KSIRK_LOG) << "    false";
    return false;
  }
}

void GameAutomaton::changePlayerName(Player* player)
{
//   qCDebug(KSIRK_LOG);
  
  QMap< QString, QString > nations = m_game->nationsList();
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    QMap<QString,QString>::iterator nationsIt;
    nationsIt = nations.find(((Player*)(*it))-> getNation()->name());
    if (nationsIt !=  nations.end())
    {
      nations.erase(nationsIt);
    }
  }
  
// Players names
  QString mes = "";
  QString nationName;
  
  QString nomEntre = player->name();
  
  bool found = true;
  KMessageBox::information(m_game, i18n("Please choose another name."), i18nc("@title:window", "Name Already Used"));
  while(found)
  {
    bool emptyName = true;
    while (emptyName)
    {
      mes = i18n("Player number %1, what's your name?", 1);
      QString password;
      if (nomEntre.isEmpty())
      {
        mes = i18n("Error - Player %1, you have to choose a name.", 1);
        KMessageBox::error(m_game, mes);
        nomEntre = i18nc("@info Forged player name", "Player%1", 1);
      }
      else 
      {
        emptyName = false;
      }
    }
    found = false;
    PlayersArray::iterator it = playerList()->begin();
    PlayersArray::iterator it_end = playerList()->end();
    for (; it != it_end; it++)
    {
      if ( (*it)-> name() ==  nomEntre)
      {
        found = true;
        it = it_end;
      }
    }
    if (!found)
    {
//     qCDebug(KSIRK_LOG) << "Creating player " << nomEntre << "(computer: " << computer << "): " << nationName ;
      player->setName(nomEntre);
    }
  }
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  sendMessage(buffer,StartGame);
}

void GameAutomaton::changePlayerNation(Player* player)
{
//   qCDebug(KSIRK_LOG);
  
  QMap< QString, QString > nations = m_game->nationsList();
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    QMap<QString,QString>::iterator nationsIt;
    nationsIt = nations.find(((Player*)(*it))-> getNation()->name());
    if (nationsIt !=  nations.end())
    {
      nations.erase(nationsIt);
    }
  }
  
// Players names
  QString mes = "";
  QString nationName;
  
  QString nomEntre = player->name();
  KMessageBox::information(m_game, i18n("Please choose another nation."), i18nc("@title:window", "Nation Already Used"));
  QString password;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << player->name() << nationName;
  sendMessage(buffer,SetNation);
  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  sendMessage(buffer2,StartGame);
}

quint32 GameAutomaton::idForMsg(const QString& msg)
{
  QMap<QString,quint32>::iterator it = m_msgs2ids.find(msg);
  if (it != m_msgs2ids.end())
  {
    return (*it);
  }
  else
  {
    quint32 id = m_msgs2ids.size();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << msg << id;
    sendMessage(buffer,AddMsgIdPair);
    return id;
  }
}

QString& GameAutomaton::msgForId(quint32 id)
{
  return m_ids2msgs[id];
}

void GameAutomaton::slotPropertyChanged(KGamePropertyBase *prop,KGame *)
{
  qCDebug(KSIRK_LOG) << prop->id() << " (skin is " << m_skinId << ")";
  if (prop->id() == m_skinId)
  {
    qCDebug(KSIRK_LOG) << "skin changed to: " << m_skin ;
    m_game->newSkin();
    if (m_game->theWorld()!=nullptr)
    {
      m_game->theWorld()->reset();
    }
  }
  qCDebug(KSIRK_LOG) << "END GameAutomaton::slotPropertyChanged " << prop->id() << " (skin is " << m_skinId << ")";
}

void GameAutomaton::slotClientJoinedGame(quint32 clientid, KGame* /*me*/)
{
  qCDebug(KSIRK_LOG) << clientid;
  if (isAdmin() && clientid!=gameId())
  {
    QByteArray buffernbp;
    QDataStream streamnbp(&buffernbp, QIODevice::WriteOnly);
    streamnbp << m_nbPlayers;
    sendMessage(buffernbp,NbPlayers,clientid);
    
    QByteArray bufferngs;
    QDataStream streamngs(&bufferngs, QIODevice::WriteOnly);
    streamngs << *m_game->newGameSetup();
    sendMessage(bufferngs,NewGameSetupMsg,clientid);
    
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (m_game->waitedPlayers().empty())
    {
      sendMessage(buffer,SetupOnePlayer,clientid);
    }
    else
    {
      stream << quint32(m_game->waitedPlayers().size());
      QList<PlayerMatrix>::iterator it, it_end;
      it = m_game->waitedPlayers().begin(); it_end = m_game->waitedPlayers().end();
      for (; it != it_end; it++)
      {
        stream << (*it);
      }
      sendMessage(buffer,SetupWaitedPlayer,clientid);
    }
  }
}

void GameAutomaton::slotConnectionToServerBroken()
{
  qCDebug(KSIRK_LOG);

//   m_game->haltTimer();
  if (m_state != GAME_OVER)
  {
    int answer = KMessageBox::questionTwoActionsCancel(m_game,
                                                  i18n("KsirK - Lost connection to server!\nWhat do you want to do?"),
                                                  i18nc("@title:window", "Starting a New Game or Exit."),
                                                  KGuiItem(i18nc("@action:button", "New Game")),
                                                  KGuiItem(i18nc("@action:button", "Exit")),
                                                  KGuiItem(i18nc("@action:button", "Do Nothing")));
    if (answer == KMessageBox::PrimaryAction)
    {
      m_game->showMainMenu();
    }
    else if (answer == KMessageBox::SecondaryAction)
    {
      exit(0);
    }
    else
    {
    }
  }
}
  
void GameAutomaton::slotConnectionToClientBroken(KMessageIO *)
{
  qCDebug(KSIRK_LOG);
//   m_game->haltTimer();
  if (m_state != GAME_OVER)
  {
    KMessageBox::information(m_game, 
                            i18n("Lost connection to a client.\nFor the moment, you can only save the game and start a new one or quit.\nThis will be improved in a future version."), 
                            i18nc("@title:window", "Lost Connection to Client"));
    switch ( KMessageBox::warningTwoActions( m_game,
                                        i18n("Do want to save your game?"),
                                        QString(),
                                        KStandardGuiItem::save(), KStandardGuiItem::discard()) )
    {
    case KMessageBox::PrimaryAction :
      m_game->slotSaveGame();
      break;
    case KMessageBox::SecondaryAction :;
    default: ;
    }
    if (!m_game->actionNewGame(GameAutomaton::None))
      exit(1);
  }
//   else
//   {
//     m_game->haltTimer();
//   }
}

void GameAutomaton::finalizePlayers()
{
  qCDebug(KSIRK_LOG);
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    dynamic_cast<Player*>(*it)-> finalize();
  }
  QByteArray buffer;
  if (isAdmin())
  {
    sendMessage(buffer,DisplayGoals);
  }
  m_game->showMap();
  m_startingGame = false;
}

/** @return true if all players are played by computer ; false otherwise */
bool GameAutomaton::allComputerPlayers()
{
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    if ( ! dynamic_cast<Player*>(*it)-> isAI() )
    {
      return false;
    }
  }
  return true;
}

bool GameAutomaton::allLocalPlayersComputer()
{
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    if ( ( ! dynamic_cast<Player*>(*it)-> isVirtual() ) &&  ( ! dynamic_cast<Player*>(*it)-> isAI() ) )
    {
      return false;
    }
  }
  return true;
}

void GameAutomaton::firstCountriesDistribution()
{
  qCDebug(KSIRK_LOG);

  if (isAdmin())
  {
    PlayersArray::iterator it = playerList()->begin();
    PlayersArray::iterator it_end = playerList()->end();
    for (; it != it_end; it++)
    {
      ((Player*)(*it))->setNbAvailArmies((unsigned int)(m_game->theWorld()->getNbCountries() * 2.5 / nbPlayers() ), true);
    }
    m_game->setCurrentPlayerToFirst();
    qCDebug(KSIRK_LOG) << "Setup players: distributing countries";
    countriesDistribution();


  //    qCDebug(KSIRK_LOG) << " KGameWindow::setupPlayers: before initTimer";
//     m_game->initTimer();
    m_game->setCurrentPlayerToFirst();
    if ( currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()) )
      if ( ! ( dynamic_cast<AIPlayer *>(currentPlayer())-> isRunning()) )
        dynamic_cast<AIPlayer *>(currentPlayer())-> start();

    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << currentPlayer()->name();
//     stream << (quint32)m_game->availArmies();
    stream << (quint32)currentPlayer()->getNbAvailArmies();
    qCDebug(KSIRK_LOG) << "sending DisplayRecycleDetails " << currentPlayer()->name() << (quint32)currentPlayer()->getNbAvailArmies()
      << " at " << __FILE__ << ", line " << __LINE__;
//       qCDebug(KSIRK_LOG) << "sending DisplayRecycleDetails " << currentPlayer()->name() << m_game->availArmies()
//       << " at " << __FILE__ << ", line " << __LINE__;
      sendMessage(buffer,DisplayRecycleDetails);


  //    qCDebug(KSIRK_LOG) << "OUT  KGameWindow::setupPlayers";

  }
}

void GameAutomaton::countriesDistribution()
{
  qCDebug(KSIRK_LOG);
  unsigned int initialNbArmies;
  QList< int > vect;
  QMap<QString,unsigned int> distributedCountriesNumberMap;
  PlayersArray::iterator it = playerList()->begin();
  initialNbArmies = ((Player*)(*playerList()->begin()))->getNbAvailArmies();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    distributedCountriesNumberMap[(*it)-> name()] = 0;
  }
  // creates a vector containing the numbers of the countries
  for (unsigned int i = 0; i < m_game->theWorld()->getNbCountries()  ; i++) vect.push_back(i);

  // do while the vector not empty (will distribute one country each turn and
  // remove its number from the vector)
  while (vect.size())
  {
    // chooses randomly a position in the remaining countries vector
    int h = Dice::roll(vect.size()) - 1;

    // moves an iterator up to the position chosen
    QList< int >::iterator it = vect.begin();
    for (int itPos = 0; itPos < h-1; itPos++) it++;

    // affect the country that have the number at this position
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << (m_game->theWorld()->getCountries().at(*it)->name()) << currentPlayer()-> name();
    qCDebug(KSIRK_LOG) << (m_game->theWorld()->getCountries().at(*it)->name()) << currentPlayer()-> name();
    distributedCountriesNumberMap[currentPlayer()-> name()] = distributedCountriesNumberMap[currentPlayer()-> name()]+1;
    sendMessage(buffer,CountryOwner);
    //m_game->theWorld()->getCountries().at(*it)-> owner(currentPlayer());
    m_game->setCurrentPlayerToNext(false);

    // removes the chosen country number from the vector, thus reducing its size
    vect.erase(it);
  }
  it = playerList()->begin();
  it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    ((GameLogic::Player*)(*it))->setNbAvailArmies(((GameLogic::Player*)(*it))->getNbAvailArmies() - distributedCountriesNumberMap[(*it)-> name()], true);
    
    ((GameLogic::Player*)(*it))->incrNbCountries(distributedCountriesNumberMap[(*it)-> name()]);
  }
//   qCDebug(KSIRK_LOG) << "All countries are now distributed.";
  m_game->setCurrentPlayerToFirst();
  QString nextPlayerName = currentPlayer()->name();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << (quint32)((GameLogic::Player*)(currentPlayer()))->getNbAvailArmies();
//   qCDebug(KSIRK_LOG) << "  Setting status " << nextPlayerName << " / " << m_game->availArmies();
  QPixmap pm = playerNamed(nextPlayerName)->getFlag()->image(0);
  KMessageParts messageParts;
  messageParts
    << pm
    << kli18n("%1: %2 armies to place").untranslatedText()
    << nextPlayerName
    <<  QString::number( initialNbArmies - distributedCountriesNumberMap[nextPlayerName]);
  qCDebug(KSIRK_LOG) << "Message parts size= " << messageParts.size();
  m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
  m_game->showMessage(i18n("Now, place your armies in your countries<br>by clicking in the target countries."));
  state(INTERLUDE);
  m_game->setNextPlayerActionEnabled(false);
}

void GameAutomaton::sendCountries()
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);

  m_game->theWorld()->sendCountries(stream);
  sendMessage(buffer,SetupCountries);
}

void GameAutomaton::movingFigthersArrived()
{
  qCDebug(KSIRK_LOG);
  state(FIGHT_ANIMATE);
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  sendMessage(buffer,AnimCombat);
}

void GameAutomaton::movingArmiesArrived()
{
  qCDebug(KSIRK_LOG);
//   m_game->terminateAttackSequence();
}

void GameAutomaton::movingArmyArrived(Country* country, unsigned int number)
{
  qCDebug(KSIRK_LOG) << number ;
  country->incrNbArmies(number);
  country->createArmiesSprites();
  checkGoal(country->owner());
}

void GameAutomaton::firingFinished()
{
  qCDebug(KSIRK_LOG);
  if (isAdmin())
  {
    m_game->resolveAttack();
    state(EXPLOSION_ANIMATE);
  }
}

void GameAutomaton::explosionFinished()
{
  qCDebug(KSIRK_LOG);
  if (isAdmin())
  {
    switch (gameStatus())
    {
      case KGame::Pause:
        m_game->setStateBeforeNewGame(FIGHT_BRINGBACK);
        break;
      case KGame::Run:;
      default:
        state(FIGHT_BRINGBACK);
    }
  }
}

void GameAutomaton::displayGoals()
{
  qCDebug(KSIRK_LOG);
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    if ( (dynamic_cast<Player*>(*it) != nullptr)
        && ( ! dynamic_cast<Player*>(*it)-> isVirtual() )
        && ( ! dynamic_cast<Player*>(*it)-> isAI() ) )
    {
      KMessageBox::information(
          game(),
          i18n("%1, your goal will be displayed. Please<br>"
               "make sure that no other player can see it!",(*it)->name()),
          i18nc("@title:window", "Displaying Goal"));
      dynamic_cast<Player*>(*it)->goal().show();
    }
  }
  m_aicannotrunhack = false;
}

void GameAutomaton::moveSlide()
{
  qCDebug(KSIRK_LOG);
  if (!currentPlayer()->isVirtual())
    m_game->slideInvade(m_game->firstCountry(), m_game->secondCountry(),InvasionSlider::Moving);
}

/**
  * Change the automatic attack state.
  * @param activated new state
  */
void GameAutomaton::setAttackAuto(bool activated)
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << (quint32)activated;

  sendMessage(buffer,AttackAuto);
}

void GameAutomaton::slotNetworkData(int msgid, const QByteArray &buffer, quint32 receiver, quint32 sender)
{
  qCDebug(KSIRK_LOG) << "msg " << msgid << " ; rec="<<receiver << " snd=" << sender ;
  if (m_game == nullptr)
  {
    exit(0);
  }

  if (msgid < CountryOwner || msgid>= UnusedLastMessageId)
  {
    return;
  }
  qCDebug(KSIRK_LOG) << "("<<KsirkMessagesIdsNames[msgid-(KGameMessage::IdUser+1)]<<", " << receiver << ", " << sender << ")";
  QDataStream stream(const_cast<QByteArray*>(&buffer), QIODevice::ReadOnly);
  QString countryName, playerName, nationName;
  QPointF point;
  quint32 removable;
  quint32 theState;
  quint32 nbArmies;
  quint32 newState;
  quint32 statusBarId, messagePartsNb, messagePartsCounter, logStatus;
  quint32 propId, prop2Id;
  quint32 playerId;
  quint32 nbVotes = 0;
  quint32 explosing;
  KMessageParts messageParts;
  QString messagePart;
  Player* player;
  PlayerMatrix waitedPlayerDef(this);
  quint32 nbWaitedPlayers;
  quint32 waitedPlayerId;
  QString waitedPlayerPassword;
  quint32 nbCountries;
  Country* country;
  quint32 msgId;
  QString ack;
  QString msg;
  Goal goal(this);
  QString playersNames;
  QPixmap pm;
  quint32 A1, A2, A3, D1, D2, NKA, NKD;
  quint32 win;
  quint32 attackAutoValue;
  quint32 availArmies;
  quint32 elemType;
  quint32 nb;
  
  if (currentPlayer() != nullptr && currentPlayer()->getFlag() != nullptr)
  {
    pm = currentPlayer()->getFlag()->image(0);
  }

  QByteArray sendBuffer;
  QDataStream sendStream(&sendBuffer, QIODevice::WriteOnly);
  switch (msgid)
  {
  case CountryOwner:
    stream >> countryName >> playerName;
    qCDebug(KSIRK_LOG) << "CountryOwner for " << (void*)m_game->theWorld()->countryNamed(countryName) << " " << countryName << " and " << (void*)playerNamed(playerName) << playerName ;
    m_game->theWorld()->countryNamed(countryName)-> owner(playerNamed(playerName));
    break;
  case PlayerPutsInitialArmy:
    stream >> point;
    if (m_game->playerPutsInitialArmy(point))
    {
      if (isAdmin())
        gameEvent("playersLooped", point);
    }
    break;
  case PlayerPutsArmy:
    stream >> point >> removable;
    if (m_game->playerPutsArmy(point, removable) && isAdmin())
    {
      gameEvent("playersLooped", point);
    }
    break;
  case StateChange:
    stream >> theState;
    qCDebug(KSIRK_LOG) << "Got new state id: " << theState <<
        " ("<<GameStateNames[theState]<<")";
    m_state = GameState(theState);
    break;
  case PlayerChange:
    stream >> playerName;
    if (!isAdmin())
    {
      m_currentPlayer = playerName;
      m_currentPlayerPlayed = false;
    }
    break;
  case RegisterCountry:
    stream >> countryName >> propId >> prop2Id;
    m_nbArmiesIdsNamesCountriesMap.insert(int(propId),countryName);
    m_namesNbArmiesIdsCountriesMap.insert(countryName,int(propId));;
//     m_nbAddedArmiesIdsNamesCountriesMap.insert(int(prop2Id),countryName);
//     m_namesNbAddedArmiesIdsCountriesMap.insert(countryName,int(prop2Id));
    break;
  case PlayerAvailArmies:
    if (sender == gameId()) break;
    stream >> playerName >> nbArmies;
    if (playerNamed(playerName) != nullptr)
      playerNamed(playerName)->setNbAvailArmies((unsigned int)nbArmies, false);
    break;
/*  case KGameWinAvailArmies:
    if (sender == gameId()) break;
    stream >> nbArmies;
    m_game->availArmies(nbArmies);*/
    break;
  case ChangeItem:
    if (sender == gameId()) break;
    stream >> statusBarId >> logStatus >> messagePartsNb;
    qCDebug(KSIRK_LOG) << "Got ChangeItem on " << statusBarId << " ; nb= " << messagePartsNb;
    for (messagePartsCounter = 0; messagePartsCounter < messagePartsNb; ++messagePartsCounter)
    {
      stream >> elemType;
      if (elemType == KMessageParts::Text)
      {
        stream >> messagePart;
        messageParts << messagePart;
        qCDebug(KSIRK_LOG) << " message part: " << messagePart;
      }
      else if (elemType == KMessageParts::StringId)
      {
        stream >> msgId;
        messageParts << msgForId(msgId);
        qCDebug(KSIRK_LOG) << " message part for id "<<msgId<<": " << msgForId(msgId);
      }
      else if (elemType == KMessageParts::Pixmap)
      {
        stream >> pm;
        messageParts << pm;
      }
    }
    m_game->changeItem(messageParts,statusBarId, logStatus);
    break;
  case DisplayRecyclingButtons:
    m_game->getRightDialog()->updateRecycleDetails(nullptr,true,0);
    m_game->displayRecyclingButtons();
    break;
  case ClearHighlighting:
    m_game->clearHighlighting();
    break;
  case ActionRecycling:
    if (isAdmin())
    {
      m_game->actionRecycling();
      state(NEWARMIES);
    }
    break;
  case ClearGameActionsToolbar:
    break;
  case DisplayDefenseButtons:
    stream >> playerName;
    if ( (!playerNamed(playerName)->isVirtual())
      && (!playerNamed(playerName)->isAI()) && (!isDefenseAuto()))
    {
      defCountry = this->game()->secondCountry();
      m_game->displayDefenseWindow();
    }
    messageParts << kli18n("%1 chooses its defense").untranslatedText() << playerName;
    m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    break;
  case ActionDefense:
    stream >> nbArmies;
    if (isAdmin())
      m_game->defense((unsigned int)nbArmies);
    break;
  case FirstCountry:
    stream >> countryName;
    qCDebug(KSIRK_LOG) << "FirstCountry " << countryName ;
    m_game->firstCountry(m_game->theWorld()->countryNamed(countryName));
    break;
  case SecondCountry:
    stream >> countryName;
    qCDebug(KSIRK_LOG) << "SecondCountry " << countryName ;
    m_game->secondCountry(m_game->theWorld()->countryNamed(countryName));
    break;
  case InitCombatMovement:
    m_game->initCombatMovement();
    break;
  case AnimCombat:
    m_game->animCombat();
    break;
  case TerminateAttackSequence:
    {
      // update country display
      if (m_game->terminateAttackSequence())
      {
        // Re-display the world view
        m_game->showMap();

        setAttackAuto(false);
//         setDefenseAuto(false);
        if(!currentPlayer()->isAI() && !currentPlayer()->isVirtual())
        {
          m_game->slideInvade(m_game->firstCountry(), m_game->secondCountry());
        }
        if (isAdmin())
        {
          state(INVADE);
        }
      }
      else if (isAdmin())
      {
        // if there is more than 1 army on my country and automatic
        // attack is activated
        if (m_game->firstCountry()->nbArmies() > 1 && m_attackAuto)
        {
          // continue automatically attacking by making the same attack
          state(WAIT1);
          if (m_game->firstCountry()->nbArmies() > 3)
          {
            gameEvent("actionAttack3", m_game->secondCountry()->centralPoint());
          }
          else if (m_game->firstCountry()->nbArmies() > 2)
          {
            gameEvent("actionAttack2", m_game->secondCountry()->centralPoint());
          }
          else
          {
            gameEvent("actionAttack1", m_game->secondCountry()->centralPoint());
          }

        // else wait user choice
        }
        else
        {
          // Re-display the world view
          m_game->showMap();

          setAttackAuto(false);
//           setDefenseAuto(false);
          state(WAIT);
        }
      }
      m_game->firstCountry()-> createArmiesSprites();
      m_game->secondCountry()-> createArmiesSprites();
    }
    break;
  case DecrNbArmies:
    stream >> countryName >> nbArmies;
    m_game->theWorld()->countryNamed(countryName)->decrNbArmies(nbArmies);
    [[fallthrough]];
  case StartLocalCurrentAI:
    m_game->startLocalCurrentAI();
    break;
  case Invade:
    stream >> nbArmies;
    nb = nbArmies;
    while (nb >= 10)
    {
      if (m_game-> invade(10))
        m_game-> incrNbMovedArmies(10);
      nb -= 10;
    }
    while (nb >= 5)
    {
      if (m_game-> invade(5))
        m_game-> incrNbMovedArmies(5);
      nb -= 5;
    }
    while (nb > 0)
    {
      if (m_game-> invade(1))
        m_game-> incrNbMovedArmies(1);
      nb--;
    }
    
    break;
  case Retreat:
    stream >> nbArmies;
    if (m_game-> retreat(nbArmies))
      m_game-> decrNbMovedArmies(nbArmies);
    break;
  case  NextPlayerNormal:
    if (isAdmin())
    {
      stream >> playerName;
      stream >> newState;
      if (playerName == currentPlayer()->name() &&  m_game->nextPlayerNormal())
      {
        if (newState > 0)
        {
          qCDebug(KSIRK_LOG) << "at " <<  __FILE__ << ", line " << __LINE__ << ", setting state to " << newState ;
          state(GameState(newState));
        }
      }
    }
    break;
  case  NextPlayerRecycling:
    if (isAdmin())
    {
      stream >> newState;
      if (m_game->nextPlayerRecycling())
      {
        m_choosedToRecycleNumber = 0;
        if (newState > 0)
        {
          qCDebug(KSIRK_LOG) << "at " <<  __FILE__ << ", line " << __LINE__ << ", setting state to " << newState ;
          state(GameState(newState));
          m_game->slotContextualHelp();
        }
      }
    }
    break;
  case ShowArmiesToPlace:
    messageParts << pm << kli18n("%1: %2 armies to place").untranslatedText() << (currentPlayer()-> name())
      << QString::number(currentPlayer()-> getNbAvailArmies());
    m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    m_game->showMessage(i18n("Now, place your armies in your countries<br>by clicking in the target countries."));
    break;
  case PlayerRemovesArmy:
    stream >> point;
    m_game->playerRemovesArmy(point);
    break;
  case VoteRecyclingFinished:
    if (isAdmin())
    {
//       stream >> playerName;
      messageParts << kli18n("%1 choose to end recycling; there are now %2 players who want so").untranslatedText();
      stream >> nbVotes;
      stream >> playerId;
      playersNames = ((Player*)(findPlayer(playerId)))->name();
      if (!m_choosedToRecycle.contains(playerId))
      {
        m_choosedToRecycle.push_back(playerId);
      }
      else
      {
        nbVotes--;
      }

      for (quint32 i = 1 ; i < nbVotes; i++)
      {
        stream >> playerId;
        playersNames += QString(", ") + ((Player*)(findPlayer(playerId)))->name();
        if (!m_choosedToRecycle.contains(playerId))
        {
          m_choosedToRecycle.push_back(playerId);
        }
        else
        {
          nbVotes--;
        }
      }
      qCDebug(KSIRK_LOG) << "VoteRecyclingFinished nb before: " << m_choosedToRecycleNumber ;
      m_choosedToRecycleNumber+=nbVotes;
      qCDebug(KSIRK_LOG) << "VoteRecyclingFinished nb after : " << m_choosedToRecycleNumber ;
      messageParts << playersNames << QString::number(m_choosedToRecycleNumber);
      m_game->broadcastChangeItem(messageParts, ID_NO_STATUS_MSG);
      if (m_choosedToRecycleNumber == (unsigned int)(playerList()->count()))
      {
        m_game->actionRecyclingFinished();
        m_choosedToRecycle.clear();
      }
    }
    break;
  case CancelShiftSource:
    m_game-> cancelShiftSource();
    if (isAdmin())
    {
      state(SHIFT1);
    }
    break;
  case ChangePlayerNation:
    stream >> playerId;
//     qCDebug(KSIRK_LOG) << "Got ChangePlayerNation for player id " << playerId ;
    player = (Player*)(findPlayer(playerId));
//     qCDebug(KSIRK_LOG) << "Found player id " << player;
    if (player && !player->isVirtual())
    {
      changePlayerNation(player);
    }
    break;
  case ChangePlayerName:
    stream >> playerId;
    player = (Player*)(findPlayer(playerId));
    if (player && !player->isVirtual())
    {
      changePlayerName(player);
    }
    break;
  case StartGame:
    if (isAdmin())
      startGame();
    break;
  case SetNation:
    stream >> playerName >> nationName;
    playerNamed(playerName)->setNation(nationName);
    break;
  case SetBarFlagButton:
    stream >> playerName;
    qCDebug(KSIRK_LOG) << "Calling setBarFlagButton for player " << playerNamed(playerName) << " named " << playerName ;
    m_game->setBarFlagButton(playerNamed(playerName));
    break;
  case FinishMoves:
    m_game->finishMoves();
    break;
  case AnimExplosion:
    stream >> explosing;
    qCDebug(KSIRK_LOG) << "AnimExplosion" << explosing;
    if (m_game->backGnd()->bgIsArena())
    {
      m_game->animExplosionForArena();
    }
    else
    {
      if (explosing != 0 && explosing != 1 && explosing != 2)
      {
        KMessageBox::error(m_game, i18n("Problem : no one destroyed"));
      }
      else
      {
        m_game->animExplosion(explosing);
      }
    }
    break;
  case SetupOnePlayer:
    if (receiver == gameId())
    {
      m_game->setupOnePlayer();
    }
    break;
  case SetupWaitedPlayer:
    stream >> nbWaitedPlayers;
    for (quint32 i = 0; i < nbWaitedPlayers; i++)
    {
      stream >> waitedPlayerDef;
      m_game->waitedPlayers().push_back(waitedPlayerDef);
    }
    m_game->setupOneWaitedPlayer();
    break;
  case ValidateWaitedPlayerPassword:
    if (isAdmin())
    {
      stream >> waitedPlayerId >> waitedPlayerPassword;
//       qCDebug(KSIRK_LOG) << "Validating password for waited player " << waitedPlayerId << " : " << m_game->waitedPlayers()[waitedPlayerId].password << " / " << waitedPlayerPassword ;
      if (m_game->waitedPlayers()[waitedPlayerId].password == waitedPlayerPassword)
      {
        sendStream << waitedPlayerId;
        sendMessage(sendBuffer,ValidPassword, sender);
      }
      else
      {
        sendMessage(sendBuffer,InvalidPassword, sender);
      }
    }
    break;
  case ValidPassword:
    if (receiver == gameId())
    {
      stream >> waitedPlayerId;
      m_game->createWaitedPlayer(waitedPlayerId);
    }
    break;
  case InvalidPassword:
    if (receiver == gameId())
    {
      KMessageBox::information(m_game, i18n("You entered an invalid password.\nPlease try again."), i18nc("@title:window", "Invalid Password"));
      m_game->setupOneWaitedPlayer();
    }
    break;
  case SetupCountries:
    stream >> nbCountries;
    for (unsigned int i = 0; i < nbCountries; i++)
    {
      stream >> countryName;
      country = m_game->theWorld()->countryNamed(countryName);
      qCDebug(KSIRK_LOG) << "Setting up country n°" << i << " on " << nbCountries << ", " << (void*)country << " named " << countryName ;
      if (country)
      {
        stream >> country;
      }
    }
    break;
  case AddMsgIdPair:
    stream >> msg >> msgId;
    m_msgs2ids.insert(msg,msgId);
    m_ids2msgs.insert(msgId,msg);
    break;
  case CheckGoal:
    if (isAdmin())
    {
      qCDebug(KSIRK_LOG) << "CheckGoal: ";
      stream >> playerId;
      player = dynamic_cast< Player* >(findPlayer(playerId));
      if (player)
      {
        if (player->checkGoal())
        {
          qCDebug(KSIRK_LOG) << "    goal reached for " << player->name();
          qCDebug(KSIRK_LOG) << "    setting state to " << GameStateNames[GAME_OVER];
//           m_game->haltTimer();
          setGameStatus(KGame::End);
          state(GAME_OVER);
          sendStream << player->id();
          qCDebug(KSIRK_LOG) << "    sending Winner";
          sendMessage(sendBuffer,Winner);
        }
      }
    }
    break;
  case SetGoalFor:
    if (isAdmin())
    {
      stream >> playerId;
      setGoalFor(dynamic_cast<Player*>(findPlayer(playerId)));
    }
    break;
  case GoalForIs:
    qCDebug(KSIRK_LOG) << "GoalForIs: ";
    stream >> playerId;
    qCDebug(KSIRK_LOG) << "  player id: " << playerId ;
    stream >> goal;
    player = dynamic_cast<Player*>(findPlayer(playerId));
    if (player != nullptr)
    {
      player->goal(goal);
    }
    break;
  case FinalizePlayers:
      qCDebug(KSIRK_LOG) << "Got message FinalizePlayers";
      finalizePlayers();
    break;
  case Winner:
    QObject::disconnect(messageServer(),&KMessageServer::connectionLost,
                        this,&GameAutomaton::slotConnectionToClientBroken);
    stream >> playerId;
    m_game->winner(dynamic_cast<Player*>(findPlayer(playerId)));
    break;
  case NbPlayers:
    stream >> m_nbPlayers;
    break;
  case Acknowledge:
      stream >> playerId;
      stream >> ack;
      if (!isAdmin() && !dynamic_cast<Player*>(findPlayer(playerId))->isVirtual())
      {
        qCDebug(KSIRK_LOG) << "Got message Acknowledge" << playerId << ack;
        dynamic_cast<Player*>(findPlayer(playerId))->acknowledge(ack);
      }
    break;
  case DisplayGoals:
      qCDebug(KSIRK_LOG) << "Got message DisplayGoals";
      QTimer::singleShot(0,this,&GameAutomaton::displayGoals);
    break;
  case DisplayFightResult:
      qCDebug(KSIRK_LOG) << "Got message DisplayFightResult";
      stream >> A1 >> A2 >> A3 >> D1 >> D2 >> NKA >> NKD >> win;
      m_game->getRightDialog()->displayFightResult(A1,A2,A3,D1,D2,NKA,NKD,win);
    break;
  case MoveSlide:
      qCDebug(KSIRK_LOG) << "Got message MoveSlide";
      moveSlide();
    break;
  case InvasionFinished:
      qCDebug(KSIRK_LOG) << "Got message InvasionFinished";
      if (isAdmin())
      {
        gameEvent("actionInvasionFinished", point);
      }
    break;
  case AttackAuto:
      qCDebug(KSIRK_LOG) << "Got message AttackAuto";
      stream >> attackAutoValue;
      m_attackAuto = attackAutoValue;
    break;
  case DisplayRecycleDetails:
      stream >> playerName;
      stream >> availArmies;
      qCDebug(KSIRK_LOG) << "Got message DisplayRecycleDetails "
          << playerName << availArmies;
      m_game->getRightDialog()->displayRecycleDetails(playerNamed(playerName),availArmies);
//       bool value;
//       if (playerNamed(playerName)->isAI() || playerNamed(playerName)->isVirtual())
//       {
//         value = false;
//       }
//       else
//       {
//         value = true;
//       }
      m_game->setNextPlayerActionEnabled(false);
    break;
  case CurrentPlayerPlayed:
    m_currentPlayerPlayed = true;
    break;
  case ResetPlayersDistributionData:
    resetPlayersDistributionData();
    break;
  case NewGameSetupMsg:
    qCDebug(KSIRK_LOG) << "Got message NewGameSetupMsg";
    stream >> *m_game->newGameSetup();
    break;
  default: ;
  }
}

QSvgRenderer& GameAutomaton::rendererFor(const QString& skinName)
{
  if (!m_renderers.contains(skinName))
  {
    m_renderers.insert(skinName,new QSvgRenderer(this));
  }
  return *m_renderers[skinName];
}

KGameSvgDocument& GameAutomaton::svgDomFor(const QString& skinName)
{
  if (!m_svgDoms.contains(skinName))
  {
    m_svgDoms.insert(skinName,KGameSvgDocument());
  }
  return m_svgDoms[skinName];
}

bool GameAutomaton::startingGame() const
{
  qCDebug(KSIRK_LOG) << m_startingGame;
  return m_startingGame;
}

bool GameAutomaton::isDefenseAuto()
{
  return m_defenseAuto.isDefenseAuto(m_game->firstCountry(),m_game->secondCountry());
}

void GameAutomaton::setDefenseAuto(bool activated)
{
  m_defenseAuto.value = activated;
  if (activated)
  {
    m_defenseAuto.firstCountry = m_game->firstCountry();
    m_defenseAuto.secondCountry= m_game->secondCountry();
  }
  else
  {
    m_defenseAuto.firstCountry = nullptr;
    m_defenseAuto.secondCountry= nullptr;
  }
}

void GameAutomaton::resetPlayersDistributionData()
{
  foreach (KPlayer* p, *playerList())
  {
    if ( !p->isVirtual() )
    {
      ((GameLogic::Player*)p)->reset();
    }
  }
}

void GameAutomaton::actionNextPlayer()
{
  if ( currentPlayer()->isVirtual()
    || currentPlayer()->isAI()
    || m_currentPlayerPlayed
    || (KMessageBox::questionTwoActions (m_game,
                                    i18n("%1, you have not played anything this turn.\nDo you really want to lose your turn?",m_currentPlayer),
                                    i18nc("@title:window", "Next Player Confirmation"),
                                    KGuiItem(i18nc("@action:button", "Next Player")),
                                    KStandardGuiItem::cancel())
       == KMessageBox::PrimaryAction) )
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << currentPlayer()->name();
    stream << (quint32)NEWARMIES;
    qCDebug(KSIRK_LOG) << "(state " << stateName() << ") sending NextPlayerNormal" << currentPlayer()->name() << NEWARMIES;
    sendMessage(buffer,NextPlayerNormal);
    m_game-> cancelAction();
  }
}

void GameAutomaton::removeAllPlayers()
{
  qCDebug(KSIRK_LOG);
  m_currentPlayer = "";
  
  /* Bug 304362. KPlayer destructor removes
   * player from the list and makes iterators invalid.
   * qDeleteAll crashes in that case. */
  while (!playerList()->isEmpty()) 
  {
    delete playerList()->takeFirst();
  }
  // qDeleteAll(*playerList());
  // playerList()->clear();
}

// Bug 308527.
void GameAutomaton::removeAllGoals()
{
    qCDebug(KSIRK_LOG);
    while (!goals().isEmpty())
        delete goals().takeFirst();
}

void GameAutomaton::newGameNext()
{
  qCDebug(KSIRK_LOG);
  m_startingGame = true;
  state(INIT);
  
  qCDebug(KSIRK_LOG) << "Changing skin";
  m_skin = m_game->newGameSetup()->skin();
  if (m_game->newGameSetup()->networkGameType() == Socket)
  {
    offerConnections(m_game->newGameSetup()->tcpPort());
  }
  m_game->finishSetupPlayers();
}

void GameAutomaton::checkGoal(Player* player) 
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  if (player == nullptr)
    stream << currentPlayer()->id();
  else
    stream << player->id();
  qCDebug(KSIRK_LOG) << "sending CheckGoal";
  sendMessage(buffer,CheckGoal);
}


} // closing namespace GameLogic
} // closing namespace Ksirk

#include "moc_gameautomaton.cpp"
