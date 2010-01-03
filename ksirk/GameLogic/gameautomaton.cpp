/* This file is part of KsirK.
   Copyright (C) 2004-2007 Gael de Chalendar <kleag@free.fr>

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

#include "gameautomaton.h"

#include "ksirksettings.h"
#include "kgamewin.h"
#include "kdebug.h"
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
#include "Jabber/kmessagejabber.h"
#include "newplayerdata.h"

#include <QLayout>
#include <QSpinBox>
#include <QPixmap>
#include <QMouseEvent>
#include <QFile>
#include <QUuid>

#include <klocale.h>
#include <kdialog.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kvbox.h>
#include <kgame/kmessageclient.h>
#include <kgame/kmessageserver.h>
#include <kgame/kgamechat.h>

#include <sstream>
#include <iostream>

#include <errno.h>
#include <sys/types.h>
#include <signal.h>

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
    m_game(0),
    m_networkPlayersNumber(0),
    m_currentPlayer(""),
    m_currentPlayerPlayed(false),
    m_savedState(INVALID),
    m_goals(),
    m_useGoals(true),
    m_attackAuto(false),
    m_defenseAuto(false),
    m_port(KSIRK_DEFAULT_PORT),
    m_startingGame(false),
    m_pixmapCache("GameAutomaton")
{
  m_skin = "skins/default";
  //   kDebug() << endl;
//   m_stateId = m_state.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_state"));
  m_skinId = m_skin.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_skin"));
//   m_currentPlayerId = m_currentPlayer.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_currentPlayer"));
//   m_events.registerData(dataHandler(),KGamePropertyBase::PolicyDirty,QString("m_events"));
  
  // Connect the most important slot which tells us which properties are
  // changed
  connect(this,SIGNAL(signalPropertyChanged(KGamePropertyBase *,KGame *)),
          this,SLOT(slotPropertyChanged(KGamePropertyBase *,KGame *)));
  
  connect(this,SIGNAL(signalPlayerJoinedGame(KPlayer*)),
          this,SLOT(slotPlayerJoinedGame(KPlayer*)));
  
  connect(this,SIGNAL(signalNetworkData(int, const QByteArray&, quint32, quint32)),
          this,SLOT(slotNetworkData(int, const QByteArray&, quint32, quint32)));
  
  connect(this,SIGNAL(signalClientJoinedGame(quint32, KGame*)),
          this,SLOT(slotClientJoinedGame(quint32, KGame*)));
  
  connect(messageClient(),SIGNAL(connectionBroken()),
          this,SLOT(slotConnectionToServerBroken()));
  
  connect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
          this,SLOT(slotConnectionToClientBroken(KMessageIO *)));

  setPolicy(KGame::PolicyDirty,true);
  
//   kDebug() << "finished" << endl;
}

GameAutomaton::~GameAutomaton()
{
  kDebug();
  foreach (Goal* goal, m_goals)
  {
    delete goal;
  }
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
  kDebug() << "new state (id=" << state << ") is " << GameStateNames[state] << endl;
  m_state = state;
  m_game->setSaveGameActionEnabled(m_state == WAIT);
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
  return 0;
}

GameAutomaton::GameState GameAutomaton::run()
{
//   kDebug() << "(KGame running=" <<  (gameStatus()==KGame::Run) << ")" << endl;
  if (m_game == 0 || gameStatus() == KGame::Pause)
  {
    QTimer::singleShot(200, this, SLOT(run()));
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

  kDebug() << "Handling " << stateName() << " ; " << event << " ; " << point << endl;
  if (currentPlayer())
  {
    kDebug() << "current player=" << currentPlayer()->name() << " is active=" << currentPlayer()->isActive() << endl;
  }
  if (event == "requestForAck")
  {
    kDebug() << "requestForAck" << endl;
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
      QTimer::singleShot(200, this, SLOT(run()));
      return INIT;
    }
    else
    {
      QTimer::singleShot(200, this, SLOT(run()));
      return m_state;
    }
  }
  if (event == "actionOpenGame")
  {
    if (m_game->actionOpenGame())
    {
      kDebug() << "opened" << endl;
      bool ok;
      m_port = KInputDialog::getInteger(
              i18n("KsirK - Network configuration"), 
              i18n("Please type in the port number on which to offer connections:"), 
              m_port, 0, 32000, 1, 10, &ok, m_game);
      offerConnections(m_port);
      state(WAIT_PLAYERS);
      QTimer::singleShot(200, this, SLOT(run()));
      return WAIT_PLAYERS;
    }
    else
    {
      kDebug() << "opened" << endl;
      QTimer::singleShot(200, this, SLOT(run()));
      return m_state;
    }
  }
  if (event == "actionJoinNetworkGame")
  {
    joinNetworkGame();
    QTimer::singleShot(200, this, SLOT(run()));
    return m_state;
  }

  switch (m_state)
  {
  case INIT:
    if (currentPlayer() != 0 && isAdmin())
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
    /*if  (event == "actionLButtonDown") 
    {
      if  ( m_game->attacker(point) )*/ 
        state(ATTACK2);
      /*else
        state(WAIT);
    }
    else if (event == "actionCancel")
    {
      m_game-> cancelAction();
      state(WAIT);
    }
    else
    {
//        if (!event.isEmpty())
//          kDebug() << "Unhandled event " << event << " during handling of " << stateName() << endl;
    }*/
    break;
  case ATTACK2:
    kDebug() << "Handling ATTACK2";
    if (isAdmin())
    {
      QByteArray buffer;
      switch ( m_game->attacked(point) )
      {
        case 0:
          kDebug() << "handling attacked value; 0" << endl;
          state(WAIT);
        break;
        case 1:
          kDebug() << "handling attacked value; 1" << endl;
          sendMessage(buffer,CurrentPlayerPlayed);
          state(WAITDEFENSE);
        break;
        case 2:
          kDebug() << "handling attacked value; 2" << endl;
          sendMessage(buffer,CurrentPlayerPlayed);
          kDebug() << "calling defense(1)" << endl;
          m_game-> defense(1);
          kDebug() << "setting state to FIGHT_BRING" << endl;
          state(FIGHT_BRING);
        break;
        case 3:
          kDebug() << "handling attacked value; 3" << endl;
          // AI action: nothing to do.
        break;
        default:
          kError() << "Unknown return value from attacked" << endl;
          exit(1);
      }
    }
    kDebug() << "handling of ATTACK2 finished !" << endl;
  break;
  case EXPLOSION_ANIMATE:
  break;
  case FIGHT_ANIMATE:
  break;
  case FIGHT_BRING:
  break;
  case FIGHT_BRINGBACK:
    // no more moving fighter returning home

//     kDebug () << "$$$$$$$STATE FIGHT_BRINGBACK $$$$$$$$$$$" << m_game->haveAnimFighters() << endl;

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
//       kDebug() << "Sending message PlayerPutsArmy " << point << endl;
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
      kDebug() << "actionRecyclingFinished";
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
          kDebug() << "Local:" << ((Player*)(*it))->name();
          nbLocal++;
        }
      }
      kDebug() << "Nb Local:" << nbLocal;
      stream << nbLocal;

      for (it = playerList()->begin(); it != it_end; it++)
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
//          kError() << "Unhandled event " << event << " during handling of " << stateName() << endl;
    }
  break;
  case INVADE:
//     kDebug () << "$$$$$$$STATE INVADE$$$$$$$$$$$" << endl;
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
//          kError() << "Unhandled event " << event << " during handling of " << stateName() << endl;
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
//          kError() << "Unhandled event " << event << " during handling of " << stateName() << endl;
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
//          std::cerr << "Unhandled event " << event << " during handling of " << stateName() << std::endl;
    }
  break;
  case SHIFT2:
//     kDebug () << "$$$$$$$STATE SHIFT2$$$$$$$$$$$" << endl;
    if (event == "actionInvade1")
    {
//       kDebug() << "actionInvade1" << endl;
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
      //std::cerr << "Unhandled event " << event << " during handling of " << stateName() << std::endl;
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
        kDebug() << "Unhandled event " << event << " during handling of " << stateName() << endl;
      else
      {
      if (allLocalPlayersComputer())
        {
          m_game->getRightDialog()->updateRecycleDetails(NULL,true,0);
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
    else if ( m_game->secondCountry() != 0
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
         kDebug() << "Unhandled event " << event << " during handling of " << stateName();
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
//          std::cerr << "Unhandled event " << event << " during handling of " << stateName() << std::endl;
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
//       kDebug() << "actionInvade1" << endl;
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
      kDebug() << "actionLButtonUp in WAIT1";
      
      if (!currentPlayer()-> isAI())
      {
        if (m_game->isMoveValid(point) && m_game->firstCountry()->nbArmies() !=1)
        {
          m_game->secondCountryAt(point);
          state(WAIT);
          kDebug() << "Sending MoveSlide";
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
    kError() << "Unhandled state: " << stateName() << ". Event was: " << event << endl; 
    exit(1); // @todo handle this error
  }

  QTimer::singleShot(200, this, SLOT(run()));

//   m_game->initTimer();
  return m_state;
}

void GameAutomaton::activateNeededAIPlayers()
{
//   kDebug();
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
    ::std::ostringstream oss;
    oss << "Invalid stored state id: " << m_state;
    kError() << oss.str().c_str() << endl;
    return QString::fromUtf8(oss.str().c_str());
  }
  else
  {
    return QString::fromUtf8(GameStateNames[m_state]);
  }
}

void GameAutomaton::saveXml(QTextStream& xmlStream)
{
    xmlStream << "<gameautomaton state=\"" << GameStateNames[m_state] << "\" />" << endl;
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
  kDebug();
//   if (player->isVirtual())
  if (!isAdmin())
  {
//     kDebug() << "Network player: nothing to do" << endl;
    return false;
  }

  // Convert the player to the right class
  Player* p = dynamic_cast<Player*>(player);

  QString action;
  QPointF point;
  msg >> action >> point;

  kDebug() << " ======================================================="<<endl;
  kDebug()  << "Player " << p->name() << " id=" << player->id()
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
    kDebug() << "acknowledging " << ack;
    if (p->isVirtual())
    {
      kDebug() << p->name() << "is virtual; sending message";
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << p->id() << ack;
      sendMessage(buffer,Acknowledge);
    }
    else
    {
      kDebug() << p->name() << "is local; acknowledging";
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

  kDebug() << "createIO for " << player->name() << endl;

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
      SIGNAL(signalMouseEvent(KGameIO *,QDataStream &,QMouseEvent *,bool *)),
      m_game->frame(),
      SLOT(slotMouseInput(KGameIO *,QDataStream &,QMouseEvent *,bool *)));

    // Add the device to the player
    player->addGameIO(input);
  }
  else if (io&AIPLAYERIO)
  {
    if (dynamic_cast<AIPlayer*>(player) != 0)
    {
      /*AIPlayerIO* input =*/ new AIPlayerIO(dynamic_cast<AIPlayer*>(player));
    }
    else
    {
      kError() << "Can create an AIPlayerIO only for AI players: " << io << endl;
    }
  }
  else
  {
    kError() << "Cannot create the requested IO device " << io << endl;
  }
  kDebug() << "Done createIO for " << player->name() << endl;
}

// Find out who will be the next player
// Note: The default behaviour as we have it here is done automatically
//        by the lib, too. So if all players player one after the other
//        this functions is NOT needed at all.
KPlayer * GameAutomaton::nextPlayer(KPlayer */*last*/,bool /*exclusive*/)
{
//   kDebug() << last->name() << endl;
//   m_game->setCurrentPlayerToNext();
  // If a last player is given switch the player
  
  // Should be enough if the admin sets the turn (unclear)
  if (isAdmin())
  {
    currentPlayer()->setTurn(true,true);
//     last->setTurn(true,true);
//     kDebug() << "nextPlayer::Setting turn to " << last->name() << ", " << last->id() << "("<<last->userId()<<")"<<endl;
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
  kDebug() << netGameType <<  endl;
  m_netGameType = netGameType;
  m_game->newGameDialog(m_skin.value(), m_netGameType);

//   m_networkPlayersNumber = ???;
  return false;
}

bool GameAutomaton::finishSetupPlayersNumberAndSkin()
{
  kDebug();
  
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
  kDebug() << player->name() << endl;
  unsigned int max = m_goals.size();
  unsigned int goalId = Dice::roll(max);
  QList< Goal* >::iterator it = m_goals.begin();
  for (unsigned int i = 1 ; i < goalId; it++,i++) {}
  Goal* goal = (*it);
  kDebug() << "Goal for " << player->name() << " is of type " << goal->type() << endl;
  if (goal->type() == Goal::GoalPlayer)
  {
    Player* target = 0;
    while (target==0 || target->id() == player->id())
    {
      unsigned int max = playerList()->count();
      unsigned int playerNum = Dice::roll(max);
//       kDebug() << "Choice player num " << playerNum << " on " << max << endl;
      PlayersArray::iterator itp = playerList()->begin();
      PlayersArray::iterator itp_end = playerList()->end();
      unsigned int j = 1;
      for (; j < playerNum; j++,itp++) {}
      target = dynamic_cast< Player* >(*itp);
    }
    if (target != 0)
    {
//       kDebug() << "Target choice for " << player->name() << ": " << target->name() << endl;
      goal->players().push_back(target->name());
    }
    else
    {
//       kDebug() << "No target chosen for " << player->name() << endl;
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
  kDebug() << "Sending message GoalForIs ("<<GoalForIs<<") for " << player->name() << endl; 
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
  kDebug();
   if (stateName() == "INIT"
     || (KMessageBox::warningContinueCancel(m_game,i18n("Do you really want to end your current game and join another?"),i18n("New game confirmation"),KStandardGuiItem::yes()) == KMessageBox::Continue))
   {
      m_game->joinNetworkGame();
   }
   return false;
}

bool GameAutomaton::connectToServ()
{
  kDebug();
  if (messageServer() != 0)
  {
    QObject::disconnect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
                        this,SLOT(slotConnectionToClientBroken(KMessageIO *)));
  }
  kDebug() << "Before connectToServer" << endl;
  QString host = m_game->newGameSetup()->host();
  int port = m_game->newGameSetup()->tcpPort();
  bool status = connectToServer(host, port);
  kDebug() << "After connectToServer" << status;
  if (messageServer())
    connect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
          this,SLOT(slotConnectionToClientBroken(KMessageIO *)));
  return status;
}

bool GameAutomaton::joinJabberGame(const QString& nick)
{
  if (stateName() == "INIT" || (KMessageBox::warningContinueCancel(m_game,i18n("Do you really want to end your current game and join another?"),i18n("New game confirmation"),KStandardGuiItem::yes()) == KMessageBox::Continue))
  {
    // stop game
    setGameStatus(KGame::End);
    state(INIT);
    savedState(INVALID);
    
    if (messageServer() != 0)
    {
      QObject::disconnect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
                           this,SLOT(slotConnectionToClientBroken(KMessageIO *)));
    }
    
    kDebug() << "Before connectToServer" << endl;
    m_game->setServerJid(nick);
    KMessageJabber* messageIO = new KMessageJabber(m_game->serverJid().full(), m_game->jabberClient(), this);
    bool status = connectToServer(messageIO);
    //       bool status = connectToServer(host, port);
    kDebug() << "After connectToServer" << status;
    if (status)
    {
      QByteArray msg("connect");
      XMPP::Message message(m_game->serverJid().full());
      message.setType("ksirkgame");
      message.setId(QUuid::createUuid().toString().remove("{").remove("}").remove("-"));
      message.setBody(msg);
      m_game->jabberClient()->sendMessage(message);
    }
    //       connect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
                                                           //          this,SLOT(slotConnectionToClientBroken(KMessageIO *)));
                                                           return status;
  }
  return false;
}

KPlayer * GameAutomaton::createPlayer(int rtti, 
                                    int /*io*/, 
                                    bool isVirtual)    
{
  kDebug() << "(" << rtti << ", " << isVirtual << ")" << endl;
  if (rtti == 1)
  {
    Player* p = new Player(this, "", 0, 0);
    p->setVirtual(isVirtual);
    if (!isVirtual)
    {
      kDebug() << "Calling player createIO" << endl;
      createIO(p,KGameIO::IOMode(KGameIO::MouseIO));
    }
    return (KPlayer*) p;
  }
  else if (rtti == 2)
  {
    AIPlayer* aip = new AIColsonPlayer("", 0, 0,  *playerList(), m_game->theWorld(),
                                   this);
    aip->stop();
    aip->setVirtual(isVirtual);
    if (!isVirtual)
    {
      kDebug() << "Calling player createIO" << endl;
      createIO(aip, KGameIO::IOMode(AIPLAYERIO));
    }
    return (KPlayer*) aip;
  }
  else 
  {
    kError() << "No rtti given... creating a Player" << endl;
    Player* p = new Player(this, "", 0, 0);
    p->setVirtual(isVirtual);
    if (!isVirtual)
    {
      kDebug() << "Calling player createIO" << endl;
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
  kError() << "GameAutomaton::playerNamed: there is no player named " 
      << playerName << endl;
  return 0;
}

Player* GameAutomaton::currentPlayer() 
{
  if (m_game && !m_currentPlayer.isEmpty())
  {
    return playerNamed(m_currentPlayer);
  }
  else return 0;
}

void GameAutomaton::currentPlayer(Player* player) 
{
  kDebug() << endl;
  if (player)
  {
    kDebug() << " name = " << player->name() << endl;
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
  kDebug() << "currently " << playerList()->count() << " / " << maxPlayers() << endl;
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
      kDebug() << "Sending ChangePlayerName for player id " << player->id() << endl;
      sendMessage(buffer,ChangePlayerName);
      
      return;
    }
    else if (nbWithNation != 1)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << player->id();
      kDebug() << "Sending ChangePlayerNation for player id " << player->id() << endl;
      sendMessage(buffer,ChangePlayerNation);
      
      return;
    }
    KMessageParts messageParts;
    messageParts 
      << I18N_NOOP("%1 (%2) joined game ; waiting for %3 players to connect")
      << p-> name() << p->getNation()->name() 
      << QString::number(maxPlayers() - int(playerList()->count()));
    m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    kDebug() << "Sending StartGame" << endl;
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
  kDebug() << stateName() << "nb players = " << playerList()->count() << " / " << maxPlayers() << endl;
  m_aicannotrunhack = true;
  //   kDebug() << "  state is " << GameStateNames[m_state] << endl;
//   kDebug() << "  saved state is " << GameStateNames[m_savedState] << endl;
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
      kDebug() << "at " <<  __FILE__ << ", line " << __LINE__ << ", setting state to " << m_savedState << endl;
      state(m_savedState);
      currentPlayer(playerNamed(m_savedPlayer));
      m_game->displayButtonsForState(m_savedState);
      m_savedPlayer = "";
      m_savedState = INVALID;
    }

    kDebug() << "Sending message FinalizePlayers" << endl;
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    sendMessage(buffer,FinalizePlayers);

    kDebug() << "Setting game status to Run" << endl;
    setGameStatus(KGame::Run);
//     m_game->initTimer();
//     kDebug() << "    true" << endl;
    return true;
  }
  else
  {
//     kDebug() << "    false" << endl;
    return false;
  }
}

void GameAutomaton::changePlayerName(Player* player)
{
//   kDebug();
  
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
  KMessageBox::information(m_game, i18n("Please choose another name"), i18n("KsirK - Name already used!"));
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
        KMessageBox::sorry(m_game, mes, i18n("Error"));
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
//     kDebug() << "Creating player " << nomEntre << "(computer: " << computer << "): " << nationName << endl;
      player->setName(nomEntre);
    }
  }
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  sendMessage(buffer,StartGame);
}

void GameAutomaton::changePlayerNation(Player* player)
{
//   kDebug() << endl;
  
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
  KMessageBox::information(m_game, i18n("Please choose another nation"), i18n("KsirK - Nation already used!"));
  QString password = false;
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
  kDebug() << prop->id() << " (skin is " << m_skinId << ")" << endl;
  if (prop->id() == m_skinId)
  {
    kDebug() << "skin changed to: " << m_skin << endl;
    m_game->newSkin();
    if (m_game->theWorld()!=0)
    {
      m_game->theWorld()->reset();
    }
  }
  kDebug() << "END GameAutomaton::slotPropertyChanged " << prop->id() << " (skin is " << m_skinId << ")" << endl;
}

void GameAutomaton::slotClientJoinedGame(quint32 clientid, KGame* /*me*/)
{
  kDebug() << clientid;
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
  kDebug();

//   m_game->haltTimer();
  if (m_state != GAME_OVER)
  {
    int answer = KMessageBox::questionYesNoCancel(m_game,
                                                  i18n("KsirK - Lost connection to server!\nWhat do you want to do?"),
                                                  i18n("Starting a new game or exit."),
                                                  KGuiItem(i18n("New Game")),
                                                  KGuiItem(i18n("Exit")),
                                                  KGuiItem(i18n("Do nothing")));
    if (answer == KMessageBox::Yes)
    {
      m_game->showMainMenu();
    }
    else if (answer == KMessageBox::No)
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
  kDebug() << endl;
//   m_game->haltTimer();
  if (m_state != GAME_OVER)
  {
    KMessageBox::information(m_game, 
                            i18n("Lost connection to a client.\nFor the moment, you can only save the game and start a new one or quit.\nThis will be improved in a future version."), 
                            i18n("KsirK - Lost connection to client!"));
    switch ( KMessageBox::warningYesNo( m_game, i18n("Do want to save your game?")) ) 
    {
    case KMessageBox::Yes :
      m_game->slotSaveGame();
      break;
    case KMessageBox::No :;
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
  kDebug();
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
  kDebug() << endl;

  if (isAdmin())
  {
    PlayersArray::iterator it = playerList()->begin();
    PlayersArray::iterator it_end = playerList()->end();
    for (; it != it_end; it++)
    {
      ((Player*)(*it))->setNbAvailArmies((unsigned int)(m_game->theWorld()->getNbCountries() * 2.5 / nbPlayers() ), true);
    }
    m_game->setCurrentPlayerToFirst();
    kDebug() << "Setup players: distributing countries" << endl;
    countriesDistribution();


  //    kDebug() << " KGameWindow::setupPlayers: before initTimer" << endl;
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
    kDebug() << "sending DisplayRecycleDetails " << currentPlayer()->name() << (quint32)currentPlayer()->getNbAvailArmies()
      << " at " << __FILE__ << ", line " << __LINE__;
//       kDebug() << "sending DisplayRecycleDetails " << currentPlayer()->name() << m_game->availArmies()
//       << " at " << __FILE__ << ", line " << __LINE__;
      sendMessage(buffer,DisplayRecycleDetails);


  //    kDebug() << "OUT  KGameWindow::setupPlayers" << endl;

  }
}

void GameAutomaton::countriesDistribution()
{
  kDebug();
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
    kDebug() << (m_game->theWorld()->getCountries().at(*it)->name()) << currentPlayer()-> name();
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
//   kDebug() << "All countries are now distributed." << endl;
  QString nextPlayerName = (*playerList()->begin())-> name();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << (quint32)((GameLogic::Player*)(*playerList()->begin()))->getNbAvailArmies();
//   kDebug() << "  Setting status " << nextPlayerName << " / " << m_game->availArmies() << endl;
  QPixmap pm = playerNamed(nextPlayerName)->getFlag()->image(0);
  KMessageParts messageParts;
  messageParts
    << pm
    << I18N_NOOP("%1: %2 armies to place")
    << nextPlayerName
    <<  QString::number( initialNbArmies - distributedCountriesNumberMap[nextPlayerName]);
  kDebug() << "Message parts size= " << messageParts.size() << endl;
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
  kDebug() << endl;
  state(FIGHT_ANIMATE);
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  sendMessage(buffer,AnimCombat);
}

void GameAutomaton::movingArmiesArrived()
{
  kDebug() << endl;
//   m_game->terminateAttackSequence();
}

void GameAutomaton::movingArmyArrived(Country* country, unsigned int number)
{
  kDebug() << number << endl;
  country->incrNbArmies(number);
  country->createArmiesSprites();
}

void GameAutomaton::firingFinished()
{
  kDebug() << endl;
  if (isAdmin())
  {
    m_game->resolveAttack();
    state(EXPLOSION_ANIMATE);
  }
}

void GameAutomaton::explosionFinished()
{
  kDebug() << endl;
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
  kDebug() << endl;
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    if ( (dynamic_cast<Player*>(*it) != 0)
        && ( ! dynamic_cast<Player*>(*it)-> isVirtual() )
        && ( ! dynamic_cast<Player*>(*it)-> isAI() ) )
    {
      KMessageBox::information(
          game(),
          i18n("%1, your goal will be displayed. Please<br>"
               "make sure that no other player can see it!",(*it)->name()),
          i18n("KsirK - Displaying Goal"));
      dynamic_cast<Player*>(*it)->goal().show();
    }
  }
  m_aicannotrunhack = false;
}

void GameAutomaton::moveSlide()
{
  kDebug();
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
  kDebug() << "msg " << msgid << " ; rec="<<receiver << " snd=" << sender << endl;
  if (m_game == 0)
  {
    exit(0);
  }

  if (msgid < CountryOwner || msgid>= UnusedLastMessageId)
  {
    return;
  }
  kDebug() << "("<<KsirkMessagesIdsNames[msgid-(KGameMessage::IdUser+1)]<<", " << receiver << ", " << sender << ")" << endl;
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
  
  if (currentPlayer() != 0 && currentPlayer()->getFlag() != 0)
  {
    pm = currentPlayer()->getFlag()->image(0);
  }

  QByteArray sendBuffer;
  QDataStream sendStream(&sendBuffer, QIODevice::WriteOnly);
  switch (msgid)
  {
  case CountryOwner:
    stream >> countryName >> playerName;
    kDebug() << "CountryOwner for " << (void*)m_game->theWorld()->countryNamed(countryName) << " " << countryName << " and " << (void*)playerNamed(playerName) << playerName << endl;
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
    kDebug() << "Got new state id: " << theState <<
        " ("<<GameStateNames[theState]<<")" << endl;
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
    if (playerNamed(playerName) != 0)
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
    kDebug() << "Got ChangeItem on " << statusBarId << " ; nb= " << messagePartsNb << endl;
    for (messagePartsCounter = 0; messagePartsCounter < messagePartsNb; messagePartsCounter++)
    {
      stream >> elemType;
      if (elemType == KMessageParts::Text)
      {
        stream >> messagePart;
        messageParts << messagePart;
        kDebug() << " message part: " << messagePart << endl;
      }
      else if (elemType == KMessageParts::StringId)
      {
        stream >> msgId;
        messageParts << msgForId(msgId);
        kDebug() << " message part for id "<<msgId<<": " << msgForId(msgId) << endl;
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
    m_game->getRightDialog()->updateRecycleDetails(0,true,0);
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
    messageParts << I18N_NOOP("%1 chooses its defense") << playerName;
    m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    break;
  case ActionDefense:
    stream >> nbArmies;
    if (isAdmin())
      m_game->defense((unsigned int)nbArmies);
    break;
  case FirstCountry:
    stream >> countryName;
    kDebug() << "FirstCountry " << countryName << endl;
    m_game->firstCountry(m_game->theWorld()->countryNamed(countryName));
    break;
  case SecondCountry:
    stream >> countryName;
    kDebug() << "SecondCountry " << countryName << endl;
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
          kDebug() << "at " <<  __FILE__ << ", line " << __LINE__ << ", setting state to " << newState << endl;
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
          kDebug() << "at " <<  __FILE__ << ", line " << __LINE__ << ", setting state to " << newState << endl;
          state(GameState(newState));
        }
      }
    }
    break;
  case ShowArmiesToPlace:
    messageParts << pm << I18N_NOOP("%1: %2 armies to place") << (currentPlayer()-> name()) 
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
      messageParts << I18N_NOOP("%1 choose to end recycling; there are now %2 players who want so");
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
      kDebug() << "VoteRecyclingFinished nb before: " << m_choosedToRecycleNumber << endl;
      m_choosedToRecycleNumber+=nbVotes;
      kDebug() << "VoteRecyclingFinished nb after : " << m_choosedToRecycleNumber << endl;
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
//     kDebug() << "Got ChangePlayerNation for player id " << playerId << endl;
    player = (Player*)(findPlayer(playerId));
//     kDebug() << "Found player id " << player;
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
    kDebug() << "Calling setBarFlagButton for player " << playerNamed(playerName) << " named " << playerName << endl;
    m_game->setBarFlagButton(playerNamed(playerName));
    break;
  case FinishMoves:
    m_game->finishMoves();
    break;
  case AnimExplosion:
    stream >> explosing;
    kDebug() << "AnimExplosion" << explosing;
    if (m_game->backGnd()->bgIsArena())
    {
      m_game->animExplosionForArena();
    }
    else
    {
      if (explosing != 0 && explosing != 1 && explosing != 2)
      {
        KMessageBox::information(m_game, i18n("Problem : no one destroyed"), i18n("Ksirk - Error!"));
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
//       kDebug() << "Validating password for waited player " << waitedPlayerId << " : " << m_game->waitedPlayers()[waitedPlayerId].password << " / " << waitedPlayerPassword << endl;
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
      KMessageBox::information(m_game, i18n("You entered an invalid password.\nPlease try again."), i18n("KsirK - Invalid password!"));
      m_game->setupOneWaitedPlayer();
    }
    break;
  case SetupCountries:
    stream >> nbCountries;
    for (unsigned int i = 0; i < nbCountries; i++)
    {
      stream >> countryName;
      country = m_game->theWorld()->countryNamed(countryName);
      kDebug() << "Setting up country n°" << i << " on " << nbCountries << ", " << (void*)country << " named " << countryName << endl;
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
      kDebug() << "CheckGoal: " << endl;
      stream >> playerId;
      player = dynamic_cast< Player* >(findPlayer(playerId));
      if (player)
      {
        if (player->checkGoal())
        {
          kDebug() << "    goal reached for " << player->name() << endl;
          kDebug() << "    setting state to " << GameStateNames[GAME_OVER] << endl;
//           m_game->haltTimer();
          setGameStatus(KGame::End);
          state(GAME_OVER);
          sendStream << player->id();
          kDebug() << "    sending Winner" << endl;
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
    kDebug() << "GoalForIs: " << endl;
    stream >> playerId;
    kDebug() << "  player id: " << playerId << endl;
    stream >> goal;
    player = dynamic_cast<Player*>(findPlayer(playerId));
    if (player != 0)
    {
      player->goal(goal);
    }
    break;
  case FinalizePlayers:
      kDebug() << "Got message FinalizePlayers" << endl;
      finalizePlayers();
    break;
  case Winner:
    QObject::disconnect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
                        this,SLOT(slotConnectionToClientBroken(KMessageIO *)));
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
        kDebug() << "Got message Acknowledge" << playerId << ack;
        dynamic_cast<Player*>(findPlayer(playerId))->acknowledge(ack);
      }
    break;
  case DisplayGoals:
      kDebug() << "Got message DisplayGoals" << endl;
      QTimer::singleShot(0,this,SLOT(displayGoals()));
    break;
  case DisplayFightResult:
      kDebug() << "Got message DisplayFightResult" << endl;
      stream >> A1 >> A2 >> A3 >> D1 >> D2 >> NKA >> NKD >> win;
      m_game->getRightDialog()->displayFightResult(A1,A2,A3,D1,D2,NKA,NKD,win);
    break;
  case MoveSlide:
      kDebug() << "Got message MoveSlide";
      moveSlide();
    break;
  case InvasionFinished:
      kDebug() << "Got message InvasionFinished";
      if (isAdmin())
      {
        gameEvent("actionInvasionFinished", point);
      }
    break;
  case AttackAuto:
      kDebug() << "Got message AttackAuto";
      stream >> attackAutoValue;
      m_attackAuto = attackAutoValue;
    break;
  case DisplayRecycleDetails:
      stream >> playerName;
      stream >> availArmies;
      kDebug() << "Got message DisplayRecycleDetails "
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
    kDebug() << "Got message NewGameSetupMsg";
    stream >> *m_game->newGameSetup();
    break;
  default: ;
  }
}

void GameAutomaton::askForJabberGames()
{
  m_game->askForJabberGames();
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
  kDebug() << m_startingGame;
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
    m_defenseAuto.firstCountry = 0;
    m_defenseAuto.secondCountry= 0;
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
    || (KMessageBox::questionYesNo (m_game,
                                     i18n("%1, you have not played anything this turn.\nDo you really want to lose your turn?",m_currentPlayer),
                                     i18n("Really Next Player?")) == KMessageBox::Yes) )
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << currentPlayer()->name();
    stream << (quint32)NEWARMIES;
    kDebug() << "sending NextPlayerNormal" << currentPlayer()->name() << NEWARMIES;
    sendMessage(buffer,NextPlayerNormal);
    m_game-> cancelAction();
  }
}

void GameAutomaton::removeAllPlayers()
{
  kDebug();
  m_currentPlayer = "";
  foreach (KPlayer*p, *playerList())
  {
    delete p;
  }
  playerList()->clear();
}

void GameAutomaton::newGameNext()
{
  kDebug();
  m_startingGame = true;
  state(INIT);
  
  kDebug() << "Changing skin" << endl;
  m_skin = m_game->newGameSetup()->skin();
  if (m_game->newGameSetup()->networkGameType() == Socket)
  {
    offerConnections(m_game->newGameSetup()->tcpPort());
  }
  m_game->finishSetupPlayers();
}

} // closing namespace GameLogic
} // closing namespace Ksirk

#include "gameautomaton.moc"
