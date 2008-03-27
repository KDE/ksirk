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
#include "Dialogs/newGameDialogImpl.h"
#include "Dialogs/kplayersetupdialog.h"
#include "krightdialog.h"

#include <qlayout.h>
#include <qspinbox.h>
#include <QPixmap>
#include <QMouseEvent>
#include <QFile>

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
    "INVALID"
};

const char* GameAutomaton::KsirkMessagesIdsNames[] = {
"CountryOwner", // 257
"PlayerPutsArmy", // 258
"StateChange", // 259
"PlayerChange", // 260
"RegisterCountry", // 261
"PlayerAvailArmies", // 262
"KGameWinAvailArmies", // 263
"ChangeItem", // 264
"DisplayRecyclingButtons", // 265
"DisplayNormalGameButtons", // 266
"ActionRecycling", // 267
"ClearGameActionsToolbar", // 268
"DisplayDefenseButtons", // 269
"ActionDefense", // 270
"FirstCountry", // 271
"SecondCountry", // 272
"InitCombatMovement", // 273
"AnimCombat", // 274
"DisplayInvasionButtons", // 275
"TerminateAttackSequence", // 276
"DecrNbArmies", // 277
"DisplayNextPlayerButton", // 278
"Invade", // 279
"SimultaneousAttackA",
"SimultaneousAttackD",
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
};

// GameAutomaton* GameAutomaton::m_singleton = 0;
//GameAutomaton::GameState  GameAutomaton::m_state = INIT;

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
    m_defenseAuto(false)
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
  kDebug() << "~GameAutomaton" << endl;
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
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << state;
  sendMessage(buffer,StateChange);
}

#include <errno.h>
#include <sys/types.h>
#include <signal.h>

bool dnssdAvailable() {
  QFile f("/var/run/mdnsd.pid");
  if (!f.open(QIODevice::ReadOnly)) return false; // no pidfile
  QByteArray line = f.readLine();
  unsigned int pid = line.toUInt();
  if (pid==0) return false; // not a pid
  return (kill(pid,0)==0 || errno==EPERM);
  // signal 0 only checks if process is running, mdnsd is probably owned 
  // by 'nobody' so we will get EPERM, if mdnsd is not running error will be ESRCH
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
  if (m_game == 0)
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

//   kDebug() << "Handling " << stateName() << " ; " << event << " ; " << point << endl;
  if (currentPlayer())
  {
//     kDebug() << "current player=" << currentPlayer()->name() << " is active=" << currentPlayer()->isActive() << endl;
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
    if (m_game->actionNewGame())
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
      #if KDE_IS_VERSION(3,4,0)
      if (dnssdAvailable())
        setDiscoveryInfo("_ksirk._tcp","wow");
      #endif
      int port = 20000;
      bool ok;
      port = KInputDialog::getInteger(
              i18n("KsirK - Network configuration"), 
              i18n("Please type in the port number on which to offer connections:"), 
              port, 0, 32000, 1, 10, &ok, m_game);
      offerConnections(port);
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
    switch ( m_game->attacked(point) )
    {
      case 0:
        kDebug() << "handling attacked value; 0" << endl;
        state(WAIT);
      break;
      case 1:
        kDebug() << "handling attacked value; 1" << endl;
        m_currentPlayerPlayed = true;
        state(WAITDEFENSE);
      break;
      case 2:
        kDebug() << "handling attacked value; 2" << endl;
        m_currentPlayerPlayed = true;
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
    kDebug() << "handling of ATTACK2 finished !" << endl;
  break;
  case EXPLOSION_ANIMATE:
  break;
  case FIGHT_ANIMATE:
  break;
  case FIGHT_BRING:
    if (event == "actionSimultaneousAttackA")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,SimultaneousAttackA);
    }
    else if (event == "actionSimultaneousAttackD")
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,SimultaneousAttackD);
    }
  break;
  case FIGHT_BRINGBACK:
    // no more moving fighter returning home

kDebug () << "$$$$$$$STATE FIGHT_BRINGBACK $$$$$$$$$$$" << m_game->haveAnimFighters() << endl;

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
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      sendMessage(buffer,ActionRecycling);
    }
    else if (event == "actionRecyclingFinished") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
//       stream << currentPlayer()->name();
      m_game->clearGameActionsToolbar(false);
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
    kDebug () << "$$$$$$$STATE INVADE$$$$$$$$$$$" << endl;
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
      stream << quint32(WAIT_RECYCLING);
      sendMessage(buffer,NextPlayerRecycling);
    }
/*      else if (event == "actionRecyclingFinished")
      {
        m_game-> displayNormalGameButtons();
        state(WAIT);
      }*/
    else
    {
      //        if (!event.isEmpty())
//          kError() << "Unhandled event " << event << " during handling of " << stateName() << endl;
    }
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
        m_game->displayInvasionButtons();
        m_currentPlayerPlayed = true;
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
    kDebug () << "$$$$$$$STATE SHIFT2$$$$$$$$$$$" << endl;
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
//     kDebug() << "event = '" << event << "'" << endl;
    /*if  (event == "actionLButtonDown") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << point << quint32(true);
      sendMessage(buffer,PlayerPutsArmy);
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
    else*/ if (event == "actionRecycling") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      sendMessage(buffer,ActionRecycling);
    }
    else if (event == "actionRecyclingFinished") 
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
//       stream << currentPlayer()->name();
      m_game->clearGameActionsToolbar(false);
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
    else
    {
      //        if (!event.isEmpty())
//          std::cerr << "Unhandled event " << event << " during handling of " << stateName() << std::endl;
    }
  break;
  case WAIT:
    if (event == "actionNextPlayer") 
    {
      if ( currentPlayer()->isVirtual()
          || currentPlayer()->isAI()
          || m_currentPlayerPlayed  
          || (KMessageBox::questionYesNo (m_game,
                i18n("%1, you have not played anything this turn.\nDo you really want to lose your turn ?",m_currentPlayer),
                "Really Next Player ?") == KMessageBox::Yes) )
      {
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream << (quint32)NEWARMIES;
        sendMessage(buffer,NextPlayerNormal);
        m_game-> cancelAction();
      }
    }
    /*else if (event == "actionAttack1")
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
    else if (event == "actionMove")
    {
//       kDebug() << "actionMove handling" << endl;
      m_game->displayCancelButton();
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << QString("");
      sendMessage(buffer,FirstCountry);
      QByteArray buffer2;
      QDataStream stream2(&buffer2, QIODevice::WriteOnly);
      stream2 << QString("");
      sendMessage(buffer2,SecondCountry);
      state(SHIFT1);
    }*/
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
    if (event == "actionAttack1")
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
    /*else if (event == "actionMove")
    {
//       kDebug() << "actionMove handling" << endl;
      m_game->displayCancelButton();
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << QString("");
      sendMessage(buffer,FirstCountry);
      QByteArray buffer2;
      QDataStream stream2(&buffer2, QIODevice::WriteOnly);
      stream2 << QString("");
      sendMessage(buffer2,SecondCountry);
      state(SHIFT1);
    }*/
    else if (event == "actionInvade1")
    {
//       kDebug() << "actionInvade1" << endl;
      m_currentPlayerPlayed = true;
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(1);
      sendMessage(buffer,Invade);
      state(WAIT);
    }
    else if (event == "actionInvade5")
    {
      m_currentPlayerPlayed = true;
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(5);
      sendMessage(buffer,Invade);
      state(WAIT);
    }
    else if (event == "actionInvade10")
    {
      m_currentPlayerPlayed = true;
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << quint32(10);
      sendMessage(buffer,Invade);
      state(WAIT);
    }
    else if (event == "actionInvasionFinished")
    {
      m_game-> shiftFinished();
      state(WAIT);
    }
    else if (event == "actionLButtonUp")
    {
	m_game->secondCountryAt(point);

	if (!currentPlayer()-> isAI())
	{
		if (m_game->isMoveValid(point) && m_game->firstCountry()->nbArmies() !=1)
		{
			if (m_game->firstCountry()->nbArmies() > 10)
			{
				m_game->frame()->getMove1Action()->setVisible(true);
				m_game->frame()->getMove5Action()->setVisible(true);
				m_game->frame()->getMove10Action()->setVisible(true);
			}
			else
			{
				if (m_game->firstCountry()->nbArmies() > 5)
				{
					m_game->frame()->getMove1Action()->setVisible(true);
					m_game->frame()->getMove5Action()->setVisible(true);
					m_game->frame()->getMove10Action()->setVisible(false);
				}
				else
				{
					if (m_game->firstCountry()->nbArmies() > 1)
					{
						m_game->frame()->getMove1Action()->setVisible(true);
						m_game->frame()->getMove5Action()->setVisible(false);
						m_game->frame()->getMove10Action()->setVisible(false);
					}
				}
			}
			m_game->frame()->getMoveContextMenu()->exec(QCursor::pos());
		}
		else
		{
			if (m_game->isFightValid(point) && m_game->firstCountry()->nbArmies() != 1)
			{
				if (m_game->firstCountry()->nbArmies() > 3)
				{
					m_game->frame()->getAttack1Action()->setVisible(true);
					m_game->frame()->getAttack2Action()->setVisible(true);
					m_game->frame()->getAttack3Action()->setVisible(true);
					
				}
				else
				{
					if (m_game->firstCountry()->nbArmies() > 2)
					{
						m_game->frame()->getAttack1Action()->setVisible(true);
						m_game->frame()->getAttack2Action()->setVisible(true);
						m_game->frame()->getAttack3Action()->setVisible(false);
					}
					else
					{
						if (m_game->firstCountry()->nbArmies() > 1)
						{
							m_game->frame()->getAttack1Action()->setVisible(true);
							m_game->frame()->getAttack2Action()->setVisible(false);
							m_game->frame()->getAttack3Action()->setVisible(false);
						}
					}
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

void GameAutomaton::gameEvent(const std::string& event, const QPointF& point)
{
  QString ev = event.c_str();
  m_events.push_back(qMakePair(ev, point));
}

/** returns the name of the current state */
QString GameAutomaton::stateName() const
{
  if (m_state < 0 
      || (unsigned int)(m_state) >= sizeof(GameStateNames))
  {
    std::ostringstream oss;
    oss << "Invalid stored state id: " << m_state;
    kError() << oss.str().c_str() << endl;
    return QString::fromUtf8(oss.str().c_str());
  }
  else
  {
    return QString::fromUtf8(GameStateNames[m_state]);
  }
}

void GameAutomaton::saveXml(std::ostream& xmlStream)
{
    xmlStream << "<gameautomaton state=\"" << GameStateNames[m_state] << "\" />" << std::endl;
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
  kDebug() << endl;
  if (player->isVirtual())
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
    << " uid=" << player->userId() << " : " << action << " at " << point << endl;

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
  else if (action == "requestForAck")
  {
    kDebug() << "acknowledging " << point.x() << " / " << m_game->frame()->mapFromScene(point).x() << endl;
    acknowledge(p, point.toPoint().x());
  }
  return false;
}

void GameAutomaton::acknowledge(Player* p, unsigned int ack)
{
  kDebug() << "("<<p->name()<<","<<ack<<")"<<endl;
  if (p->isVirtual())
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << p->id() << ack;
    sendMessage(buffer,Acknowledge);
  }
  else
  {
    p->acknowledge(ack);
  }
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


bool GameAutomaton::setupPlayersNumberAndSkin(bool& networkGame, int& port, uint& newPlayersNumber)
{
  kDebug() << endl;
  std::map< QString, QString > nations = m_game->nationsList();
  if (nations.size() < 2)
  {
    QString mes = "";
    mes = i18nc("@info Report the wrong number of nations given", "Error - 2 nations minimum. Got %1.",nations.size());
    KMessageBox::error(m_game, mes, i18n("Fatal Error!"));
    exit(1);
  }
  // Number of m_players
  QString skinName = m_skin;

  while ((newPlayersNumber < 2) || (newPlayersNumber > nations.size()))
  {
    bool ok;
    NewGameDialogImpl(this, ok, newPlayersNumber, nations.size(), skinName, networkGame, m_useGoals, m_game).exec();
    kDebug() << "Got " << ok << " ; " << newPlayersNumber << " ; " << skinName << endl;
    if (!ok)
    {
      return false;
    }
  }
  setMinPlayers(newPlayersNumber);
  setMaxPlayers(newPlayersNumber);
  m_nbPlayers = newPlayersNumber;
//   kDebug() << "min and max players number set to " << newPlayersNumber << endl;
  
//   kDebug() << "Got skin name: " << skinName << endl;
/*  if (skinName != m_skin)
  {*/
    kDebug() << "Changing skin" << endl;
    m_skin = skinName;
//   }
  
  port = 20000;
  if (networkGame)
  {
// porting    
    KDialog* dialog = new KDialog( m_game );
    dialog->setCaption( i18n("Port and Net players configuration") );
    dialog->setButtons( KDialog::Ok );
    
    QGroupBox* mRemoteGroup=new QGroupBox(i18n("Number of network players"), dialog);
    QSpinBox* spinBox = new QSpinBox(0);
    spinBox->setMinimum(1);
    spinBox->setMaximum(newPlayersNumber);
    KLineEdit* edit = new KLineEdit( );

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(spinBox);
    vbox->addWidget(edit);
    vbox->addStretch(1);
    mRemoteGroup->setLayout(vbox);

    edit->setText(QString::number(port));
    dialog->setMainWidget(mRemoteGroup);
    dialog->exec();
    
    m_networkPlayersNumber = spinBox->value();
    port = edit->text().toInt();
    kDebug() << "There will be " << m_networkPlayersNumber << " network players." << endl;
    #if KDE_IS_VERSION(3,4,0)
    if (dnssdAvailable())
      setDiscoveryInfo("_ksirk._tcp","wow");
    #endif
    dialog->hide();
//     delete dialog;
  }
  return true;
}

void GameAutomaton::setGoalFor(Player* player)
{
  kDebug() << player->name() << endl;
  unsigned int max = m_goals.size();
  unsigned int goalId = Dice::roll(max);
  std::set< Goal* >::iterator it = m_goals.begin();
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
      goal->players().insert(target->id());
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
   if (stateName() == "INIT" || (KMessageBox::warningContinueCancel(m_game,i18n("Do you really want to end your current game and join another ?"),i18n("New game confirmation"),KStandardGuiItem::yes()) == KMessageBox::Continue)) {

      // Set default network parameter
      QString host = "localhost";
      int port = 20000;

      // porting  
      KDialog* dialog = new KDialog(m_game);
      dialog->setCaption( i18n("Join network game configuration" ) );
      dialog->setButtons( KDialog::Ok | KDialog::Cancel );

      QGroupBox* mRemoteGroup=new QGroupBox(i18n("Network configuration"), dialog);
      KLineEdit* hostEdit = new KLineEdit( 0 );
      hostEdit->setText(host);
      KLineEdit* portEdit = new KLineEdit( 0 );
      portEdit->setText(QString::number(port));

      QVBoxLayout *vbox = new QVBoxLayout;
      vbox->addWidget(hostEdit);
      vbox->addWidget(portEdit);
      vbox->addStretch(1);
      mRemoteGroup->setLayout(vbox);

      dialog->setMainWidget(mRemoteGroup);
      QDialog::DialogCode valid = QDialog::DialogCode(dialog->exec());

      if (valid == QDialog::Rejected)
      {
         return false;
      }

      // stop game
      setGameStatus(KGame::End);
      state(INIT);
      savedState(INVALID);

      host = hostEdit->text();
      port = portEdit->text().toInt();

      if (messageServer() != 0)
      {
         QObject::disconnect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
         this,SLOT(slotConnectionToClientBroken(KMessageIO *)));
      }
      kDebug() << "Before connectToServer" << endl;
      bool status = connectToServer(host, port);
      kDebug() << "After connectToServer" << endl;
      connect(messageServer(),SIGNAL(connectionLost(KMessageIO *)),
         this,SLOT(slotConnectionToClientBroken(KMessageIO *)));
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
  kDebug() << "slotPlayerJoinedGame currently " << playerList()->count() << " / " << maxPlayers() << endl;
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
}

bool GameAutomaton::startGame()
{
  kDebug() << "nb players = " << playerList()->count() << " / " << maxPlayers() << endl;
//   kDebug() << "  state is " << GameStateNames[m_state] << endl;
//   kDebug() << "  saved state is " << GameStateNames[m_savedState] << endl;
  if (isAdmin() && int(playerList()->count()) == maxPlayers()
      && gameStatus()!=KGame::Run)
  {
//     m_game->haltTimer();

    if (m_state == INIT && m_savedState == INVALID)
    {
      firstCountriesDistribution();
//       finalizePlayers();

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
    //finalizePlayers();

m_aicannotrunhack = true;
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
//   kDebug() << endl;
  
  std::map< QString, QString > nations = m_game->nationsList();
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    std::map<QString,QString>::iterator nationsIt;
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
  bool computer = player->isAI();
  
  bool found = true;
  KMessageBox::information(m_game, i18n("Please choose another name"), i18n("KsirK - Name already used !"));
  while(found)
  {
    bool emptyName = true;
    while (emptyName)
    {
      mes = i18n("Player number %1, what's your name ?", 1);
      bool network = false;
      QString password;
      KPlayerSetupDialog(this, m_game->theWorld(), 1, nomEntre, network, password, computer, nations, nationName, m_game).exec();
//     kDebug() << "After KPlayerSetupDialog. name: " << nomEntre << endl;
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
  
  std::map< QString, QString > nations = m_game->nationsList();
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    std::map<QString,QString>::iterator nationsIt;
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
  bool computer = player->isAI();
  KMessageBox::information(m_game, i18n("Please choose another nation"), i18n("KsirK - Nation already used !"));
  bool network = false;
  QString password = false;
  KPlayerSetupDialog(this, m_game->theWorld(), 1, nomEntre, network, password, computer, nations, nationName, m_game).exec();
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
  std::map<QString,quint32>::iterator it = m_msgs2ids.find(msg);
  if (it != m_msgs2ids.end())
  {
    return (*it).second;
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
  }
  kDebug() << "END GameAutomaton::slotPropertyChanged " << prop->id() << " (skin is " << m_skinId << ")" << endl;
}

void GameAutomaton::slotClientJoinedGame(quint32 clientid, KGame* /*me*/)
{
  kDebug() << clientid << endl;
  if (isAdmin() && clientid!=gameId())
  {
    QByteArray buffernbp;
    QDataStream streamnbp(&buffernbp, QIODevice::WriteOnly);
    streamnbp << m_nbPlayers;
    sendMessage(buffernbp,NbPlayers,clientid);
    
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (m_game->waitedPlayers().empty())
    {
      sendMessage(buffer,SetupOnePlayer,clientid);
    }
    else
    {
      stream << quint32(m_game->waitedPlayers().size());
      std::vector<PlayerMatrix>::iterator it, it_end;
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
  kDebug() << endl;

//   m_game->haltTimer();
  if (m_state != GAME_OVER)
  {
    if (KMessageBox::questionYesNoCancel(m_game,
        i18n("KsirK - Lost connection to server !\nWhat do you want to do ?"),
        i18n("Starting a new game or exit."), 
        KGuiItem(i18n("New Game")),
        KGuiItem(i18n("Exit")),
        KGuiItem(i18n("Do nothing")))
            == KMessageBox::Yes)
    {
      if (!m_game->actionNewGame())
        exit(0);
    }
    else
    {
      exit(0);
    }
  }
//   else
//   {
//     m_game->haltTimer();
//   }
}
  
void GameAutomaton::slotConnectionToClientBroken(KMessageIO *)
{
  kDebug() << endl;
//   m_game->haltTimer();
  if (m_state != GAME_OVER)
  {
    KMessageBox::information(m_game, 
                            i18n("Lost connection to a client.\nFor the moment, you can only save the game and start a new one or quit.\nThis will be improved in a future version."), 
                            i18n("KsirK - Lost connection to client !"));
    switch ( KMessageBox::warningYesNo( m_game, i18n("Do want to save your game?")) ) 
    {
    case KMessageBox::Yes :
      m_game->slotSaveGame();
      break;
    case KMessageBox::No :;
    default: ;
    }
    if (!m_game->actionNewGame())
      exit(1);
  }
//   else
//   {
//     m_game->haltTimer();
//   }
}

void GameAutomaton::slotNetworkData(int msgid, const QByteArray &buffer, quint32 receiver, quint32 sender)
{
  kDebug() << "msg " << msgid << " ; rec="<<receiver << " snd=" << sender << endl;
  if (m_game == 0)
  {
    exit(0);
  }

  if (msgid < CountryOwner || msgid> DisplayGoals)
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
  PlayerMatrix waitedPlayerDef;
  quint32 nbWaitedPlayers;
  quint32 waitedPlayerId;
  QString waitedPlayerPassword;
  quint32 nbCountries;
  Country* country;
  quint32 msgId;
  quint32 ack;
  QString msg;
  Goal goal(this);
  QString playersNames;
  QPixmap pm;
  if (currentPlayer() != 0)
  {
    pm = currentPlayer()->getFlag()->image(0);
  }

  QByteArray sendBuffer;
  QDataStream sendStream(&sendBuffer, QIODevice::WriteOnly);
  switch (msgid)
  {
  case CountryOwner:
    stream >> countryName >> playerName;
//     kDebug() << "CountryOwner for " << m_game->theWorld()->countryNamed(countryName) << " " << countryName << " and " << playerName << endl;
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
    m_currentPlayer = playerName;
    m_currentPlayerPlayed = false;
    break;
  case RegisterCountry:
    stream >> countryName >> propId >> prop2Id;
    m_nbArmiesIdsNamesCountriesMap.insert(std::make_pair(int(propId),countryName));
    m_namesNbArmiesIdsCountriesMap.insert(std::make_pair(countryName,int(propId)));;
    m_nbAddedArmiesIdsNamesCountriesMap.insert(std::make_pair(int(prop2Id),countryName));
    m_namesNbAddedArmiesIdsCountriesMap.insert(std::make_pair(countryName,int(prop2Id)));
    break;
  case PlayerAvailArmies:
    if (sender == gameId()) break;
    stream >> playerName >> nbArmies;
    playerNamed(playerName)->setNbAvailArmies((unsigned int)nbArmies, false);
    break;
  case KGameWinAvailArmies:
    if (sender == gameId()) break;
    stream >> nbArmies;
    m_game->availArmies(nbArmies);
    break;
  case ChangeItem:
    if (sender == gameId()) break;
    stream >> statusBarId >> logStatus >> messagePartsNb >> msgId;
    messageParts << m_ids2msgs[msgId];
    kDebug() << "Got ChangeItem on " << statusBarId << " ; nb= " << messagePartsNb << endl;
    for (messagePartsCounter = 1; messagePartsCounter < messagePartsNb; messagePartsCounter++)
    {
      int elemType;
      stream >> elemType;
      if (elemType == KMessageParts::Text)
      {
        stream >> messagePart;
        messageParts << messagePart;
//       kDebug() << " message part: " << messagePart << endl;
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
  case DisplayNormalGameButtons:
    m_game->displayNormalGameButtons();
    break;
  case ActionRecycling:
    if (isAdmin())
    {
      m_game->actionRecycling();
      state(NEWARMIES);
    }
    break;
  case ClearGameActionsToolbar:
    if (sender == gameId()) break;
    m_game->clearGameActionsToolbar(false);
    break;
  case DisplayDefenseButtons:
    stream >> playerName;
    if ( (!playerNamed(playerName)->isVirtual()) && (!playerNamed(playerName)->isAI()) && (!this->isDefenseAuto()))
    { //m_game->displayDefenseButtons();
      defCountry = this->game()->secondCountry();
      m_game->displayDefenseWindow();
    }
    break;
  case ActionDefense:
    stream >> nbArmies;
    if (!currentPlayer()->isVirtual())
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
    m_game->getRightDialog()->close();
    m_game->getRightDialog()->displayFightDetails(m_game->firstCountry(),m_game->secondCountry(),m_game->firstCountry()->owner()->getNbAttack(),m_game->secondCountry()->owner()->getNbDefense());
    m_game->centerOnFight();  //center the game on the fight
    if  (m_game->isArena() && !currentPlayer()->isAI() && !currentPlayer()->isVirtual())
    {
      kDebug() << "Attack with arena" << endl;
      // init and display the arena view
      m_game->showArena();
    }
    m_game->initCombatMovement(m_game->firstCountry(), m_game->secondCountry());
    break;
  case AnimCombat:
    m_game->animCombat();
    break;
  case TerminateAttackSequence:
    {
      if (m_game->terminateAttackSequence() && isAdmin())
      {
        // Re-display the world view
        m_game->showMap();

        // update country display
        m_game->firstCountry()-> createArmiesSprites();
        m_game->secondCountry()-> createArmiesSprites();

        setAttackAuto(false);
        setDefenseAuto(false);
        if(!currentPlayer()->isAI())m_game->slideInvade(m_game->firstCountry(), m_game->secondCountry());
        state(INVADE);
      }
      else if (isAdmin())
      {
        // if there is more than 1 army on my country and automatic
        // attack is activated
        if (m_game->firstCountry()->nbArmies() > 1 && isAttackAuto()) {
          // update country display
          m_game->firstCountry()-> createArmiesSprites();
          m_game->secondCountry()-> createArmiesSprites();

          // continue automaticaly attacking by making the same attack
          state(WAIT1);
          if (m_game->firstCountry()->nbArmies() > 3) {
            gameEvent("actionAttack3", m_game->secondCountry()->centralPoint());
          } else if (m_game->firstCountry()->nbArmies() > 2) {
            gameEvent("actionAttack2", m_game->secondCountry()->centralPoint());
          } else {
            gameEvent("actionAttack1", m_game->secondCountry()->centralPoint());
          }

        // else wait user choice
        } else {
          // Re-display the world view
          m_game->showMap();

          // update country display
          m_game->firstCountry()-> createArmiesSprites();
          m_game->secondCountry()-> createArmiesSprites();

          setAttackAuto(false);
          setDefenseAuto(false);
          state(WAIT);
        }
      }
    }
    break;
  case DisplayInvasionButtons:
    m_game->displayInvasionButtons();
    break;
  case DecrNbArmies:
    stream >> countryName >> nbArmies;
    m_game->theWorld()->countryNamed(countryName)->decrNbArmies(nbArmies);
  case DisplayNextPlayerButton:
    m_game->displayNextPlayerButton();
    break;
  case Invade:
    stream >> nbArmies;
    if (m_game-> invade(nbArmies))
      m_game-> incrNbMovedArmies(nbArmies);
    break;


  case SimultaneousAttackA:
    stream >> nbArmies;
    m_game-> simultaneousAttack(nbArmies,0);
    break;

  case SimultaneousAttackD:
    stream >> nbArmies;
    m_game-> simultaneousAttack(nbArmies,1);
    break;

  case Retreat:
    stream >> nbArmies;
    if (m_game-> retreat(nbArmies))
      m_game-> decrNbMovedArmies(nbArmies);
    break;
  case  NextPlayerNormal:
    if (isAdmin())
    {
      stream >> newState;
      if (m_game->nextPlayerNormal())
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
      m_game->clearGameActionsToolbar(false);
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
    messageParts << pm << I18N_NOOP("%1 : %2 armies to place") << (currentPlayer()-> name()) 
      << QString::number(currentPlayer()-> getNbAvailArmies());
    m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    m_game->showMessage(i18n("Now, place your armies in your countries<br/>by clicking in the target countries."));
    break;
  case PlayerRemovesArmy:
    stream >> point;
    m_game->playerRemovesArmy(point);
    break;
  case VoteRecyclingFinished:
    if (isAdmin())
    {
//       stream >> playerName;
      messageParts << I18N_NOOP("%1 choose to end recycling ; there is now %2 players who want so");
      stream >> nbVotes;
      stream >> playerId;
      playersNames = ((Player*)(findPlayer(playerId)))->name();
      if (m_choosedToRecycle.find(playerId) == m_choosedToRecycle.end())
      {
        m_choosedToRecycle.insert(playerId);
      }
      else
      {
        nbVotes--;
      }

      for (quint32 i = 1 ; i < nbVotes; i++)
      {
        stream >> playerId;
        playersNames += QString(", ") + ((Player*)(findPlayer(playerId)))->name();
        if (m_choosedToRecycle.find(playerId) == m_choosedToRecycle.end())
        {
          m_choosedToRecycle.insert(playerId);
        }
        else
        {
          nbVotes--;
        }
      }
//       kDebug() << "VoteRecyclingFinished nb before: " << m_choosedToRecycleNumber << endl;
      m_choosedToRecycleNumber+=nbVotes;
//       kDebug() << "VoteRecyclingFinished nb after : " << m_choosedToRecycleNumber << endl;
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
    
    if (m_game->isArena())
    {
      m_game->animExplosionForArena(m_game->firstCountry(), m_game->secondCountry());
    }
    else
    {
      if (explosing != 0 && explosing != 1 && explosing != 2)
      {
      KMessageBox::information(m_game, i18n("Problem : no one destroyed"), i18n("Ksirk - Error !"));
      }
      else
      {
        m_game->animExplosion(explosing,m_game->firstCountry(), m_game->secondCountry());
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
      KMessageBox::information(m_game, i18n("You entered an invalid password.\nPlease try again."), i18n("KsirK - Invalid password !"));
      m_game->setupOneWaitedPlayer();
    }
    break;
  case SetupCountries:
    stream >> nbCountries;
    for (unsigned int i = 0; i < nbCountries; i++)
    {
      stream >> countryName;
      country = m_game->theWorld()->countryNamed(countryName);
//       kDebug() << "Setting up country n°" << i << " on " << nbCountries << ", " << country << " named " << countryName << endl;
      if (country)
      {
        stream >> country;
      }
    }
    break;
  case AddMsgIdPair:
    stream >> msg >> msgId;
    m_msgs2ids.insert(std::make_pair(msg,msgId));
    m_ids2msgs.insert(std::make_pair(msgId,msg));
    break;
  case CheckGoal:
    if (isAdmin())
    {
//       kDebug() << "CheckGoal: " << endl;
      stream >> playerId;
      player = dynamic_cast< Player* >(findPlayer(playerId));
      if (player)
      {
        if (player->checkGoal())
        {
//           kDebug() << "    goal reached for " << player->name() << endl;
//           kDebug() << "    setting state to " << GameStateNames[GAME_OVER] << endl;
//           m_game->haltTimer();
          setGameStatus(KGame::End);
          state(GAME_OVER);
          sendStream << player->id();
//           kDebug() << "    sending Winner" << endl;
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
      kDebug() << "Got message Acknowledge" << endl;
      stream >> playerId;
      stream >> ack;
      acknowledge(dynamic_cast<Player*>(findPlayer(playerId)), ack);
    break;
  case DisplayGoals:
      kDebug() << "Got message DisplayGoals" << endl;
      QTimer::singleShot(0,this,SLOT(displayGoals()));
    break;
  default: ;
  }
}

void GameAutomaton::finalizePlayers()
{
  kDebug() << endl;
  PlayersArray::iterator it = playerList()->begin();
  PlayersArray::iterator it_end = playerList()->end();
  for (; it != it_end; it++)
  {
    dynamic_cast<Player*>(*it)-> finalize();
  }
  QByteArray buffer;
  sendMessage(buffer,DisplayGoals);
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
      ((Player*)(*it))->setNbAvailArmies((unsigned int)(m_game->theWorld()->getNbCountries() * 2.5 / nbPlayers() ));
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

    m_game->getRightDialog()->displayRecycleDetails(m_game->currentPlayer(),m_game->availArmies());
    
  //    kDebug() << "OUT  KGameWindow::setupPlayers" << endl;
    
  }
}

void GameAutomaton::countriesDistribution()
{
//   kDebug() << "KGameWindow::countriesDistribution" << endl;
  unsigned int initialNbArmies;
  std::list< int > vect;
  std::map<QString,unsigned int> distributedCountriesNumberMap;
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
    std::list< int >::iterator it = vect.begin();
    for (int itPos = 0; itPos < h-1; itPos++) it++;
    
    // affect the country that have the number at this position
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << (m_game->theWorld()->getCountries().at(*it)->name()) << currentPlayer()-> name();
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
    playerNamed((*it)-> name())->decrNbAvailArmies(distributedCountriesNumberMap[(*it)-> name()]);
    playerNamed((*it)-> name())->incrNbCountries(distributedCountriesNumberMap[(*it)-> name()]);
  }
//   kDebug() << "All countries are now distributed." << endl;
  QString nextPlayerName = (*playerList()->begin())-> name();
  m_game->availArmies(initialNbArmies - distributedCountriesNumberMap[nextPlayerName]);
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << (quint32)m_game->availArmies();
  sendMessage(buffer,KGameWinAvailArmies);
  kDebug() << "  Setting status " << nextPlayerName << " / " << m_game->availArmies() << endl;
  QPixmap pm = playerNamed(nextPlayerName)->getFlag()->image(0);
  KMessageParts messageParts;
  messageParts 
    << pm
    << I18N_NOOP("%1 : %2 armies to place")
    << nextPlayerName 
    <<  QString::number( initialNbArmies - distributedCountriesNumberMap[nextPlayerName]);
  kDebug() << "Message parts size= " << messageParts.size() << endl;
  m_game->broadcastChangeItem(messageParts, ID_STATUS_MSG2);
  m_game->showMessage(i18n("Now, place your armies in your countries<br/>by clicking in the target countries."));
  state(INTERLUDE);
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
  country->incrNbAddedArmies(number);
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
    state(FIGHT_BRINGBACK);
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
          i18n("%1, your goal will be displayed. Please make sure that no other player can see it !",(*it)->name()),
          i18n("KsirK - Displaying Goal"));
      dynamic_cast<Player*>(*it)->goal().show();
    }
  }
  m_aicannotrunhack = false;
}

} // closing namespace GameLogic
} // closing namespace Ksirk

#include "gameautomaton.moc"
