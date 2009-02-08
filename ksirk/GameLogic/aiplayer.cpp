/***************************************************************************
                          aiplayer.cpp  -  description
                             -------------------
    begin                : sam sep 14 2002
    copyright            : (C) 2002 by Gael de Chalendar
    email                : kleag@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301, USA
 ***************************************************************************/

/* Local includes */
#include "aiplayer.h"
#include "aiplayerio.h"
#include "dice.h"
#include "nationality.h"
#include "onu.h"
#include "kgamewin.h"
#include "gameautomaton.h"

#include <kgame/kplayer.h>


/* Includes for KDE */
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <QUuid>
#include <QMutexLocker>
#include <QMap>


/* Includes for the STL */
// #include <utility>
// #include <map>

// #include <stdlib.h>

namespace Ksirk
{

namespace GameLogic
{

/**
  * Constructor with simple initializations
  */
AIPlayer :: AIPlayer(
        const QString & nomPlayer,
        unsigned int nbArmies,
         Nationality * myNation,
         PlayersArray& players,
         ONU* world,
         GameAutomaton* game ) :
    Player(game, nomPlayer, nbArmies, myNation) ,
  allPlayers(players),
  m_world(world),
  m_game(game),
  m_src(0), m_dest(0),
  m_toMove(std::numeric_limits< unsigned int>::max()),
  m_hasVoted(false),
  m_actionWaitingStart(false),
  m_thread(*this)
{
  m_thread.setStopMe(true);
//   kDebug() << "AIPlayer constructor" << endl;
}

AIPlayer::~AIPlayer()
{
  kDebug() << name();
  m_thread.terminate();
  m_thread.wait();
}

/**
  * This function is called whenever the player should choose an action (
  * attack, defense, etc.). It has the responsibility to choose the correct
  * action depending on the state of the game.
  */
void AIPlayer::actionChoice(GameLogic::GameAutomaton::GameState state)
{
  kDebug() << name() << ": (state is " << m_game-> stateName() << ", current player is "<<m_game-> currentPlayer()->name()<<")" << endl;
  if (m_game->m_aicannotrunhack)
  {
    kDebug() << "HACK HACK AIPlayer " << name()   
        << ": game says AIs cannot run..." << endl;
    return;
  }
  if (m_game->game()->haveAnimFighters() )
  {
    return;
  }
  if (!m_waitedAck.isEmpty())
  {
    kDebug() << Player::name() << " waiting to receive ack " << m_waitedAck;
    return;
  }
  QPointF point;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  if ( (m_game-> currentPlayer() == this)
    || (state == GameLogic::GameAutomaton::WAITDEFENSE && (m_game-> currentPlayer() != this))
    ||  (state == GameLogic::GameAutomaton::WAIT_RECYCLING && m_game-> currentPlayer()->isVirtual() ) )
  {
    kDebug() << name()  << " : choosing my action";
    switch (state)
    {
      case GameLogic::GameAutomaton::WAITDEFENSE :
        chooseDefenseAction();
        break;
      case GameLogic::GameAutomaton::WAIT_RECYCLING :
        chooseWetherToRecycle();
        break;
      case GameLogic::GameAutomaton::INVADE :
        chooseInvasionAction();
        break;
      case GameLogic::GameAutomaton::WAIT :
//         kDebug() << "WAIT " << name() << endl;
        if (!m_actionWaitingStart)
          chooseAttackMoveArmiesOrNextPlayer();
  //if (m_src != 0 && m_dest != 0)
        //{
    //stream << QString("actionLButtonDown") << m_src->centralPoint();
    //aiPlayerIO()->sendInput(stream,true);
          
  //}
        break;
      case GameLogic::GameAutomaton::WAIT1:
        m_actionWaitingStart = false;

        chooseInvasionAction();
        if (m_src != 0 && m_dest != 0
            && m_src->owner() == m_dest->owner())
        {
          stream << QString("actionLButtonUp") << m_dest->centralPoint();
          m_src = 0;
          m_dest = 0;
          aiPlayerIO()->sendInput(stream,true);
          //m_game->state(GameAutomaton::ATTACK2);
        }

  
    /*stream << QString("actionAttack2") << point;
    
      kError() << "The attacker tries to attack with a number of armies different of 1, 2 or 3: that's impossible!" << endl;
      //exit();*/
  
        break;
      case GameLogic::GameAutomaton::INIT: 
        break;
      case GameLogic::GameAutomaton::INTERLUDE: ;
      case GameLogic::GameAutomaton::NEWARMIES :
        placeArmiesAction();
        break;
      case GameLogic::GameAutomaton::ATTACK2 :
        /*if (m_dest != 0)
        {
          //stream << QString("actionLButtonUp") << m_dest->centralPoint();
          m_src = 0;
          m_dest = 0;
          //aiPlayerIO()->sendInput(stream,true);
        }*/
        break;
      case GameLogic::GameAutomaton::SHIFT1 :
        if (m_src != 0 && m_dest != 0)
        {
          /*stream << QString("actionLButtonDown") << m_src->centralPoint();
          aiPlayerIO()->sendInput(stream,true);
          stream2 << QString("actionLButtonUp") << m_dest->centralPoint();
  //         m_toMove = 0;
          aiPlayerIO()->sendInput(stream2,true);*/
        }
        break;
      case GameLogic::GameAutomaton::SHIFT2 :
        if (m_dest != 0)
        {
          chooseNbToMoveOrStop();
        }
        break;
      case GameLogic::GameAutomaton::GAME_OVER :
        stop();
        break;
      default :;
//         kDebug() << "OTHER STATE:" << state << " "  << name() << endl;
    }
    requestAck();
    if (m_game-> currentPlayer() != this)
      m_actionWaitingStart = false;
  }
//    kDebug() << "OUT" << endl;
}

/**
  * Chooses the next action. In the current basic setting, chooses at random
  * between the three possibilities. For each, chooses randomly the parameters.
  * If the randomly chosen parameters end by an impossible action,  continue
  * with next player.
  */
void AIPlayer::chooseAttackMoveArmiesOrNextPlayer()
{
//     kDebug() << "AIPlayer::chooseAttackMoveArmiesOrNextPlayer() " << endl;
  m_actionWaitingStart = true;
  if ( m_game->game()->haveMovingArmies())
  {
    return;
  }
  unsigned int dice = Dice::roll(12);
//     kDebug() << "AIPlayer Dice rolled on " << dice << endl;
  switch ( dice )
  {
    case 1: ; case 2:; case 3:; case 4:;case 5: ; case 6:;case 7:;case 8:;case 9: ;case 10: ; case 11: // attack
    {
      if (!attackAction())
      {
        nextPlayerAction();
      }
    }
    break;
/*        case 11 : // next player
            nextPlayerAction();
        break;
*/
    case 12 : // move armies
      moveArmiesAction();
    break;
  }
//    kDebug() <<"OUT AIPlayer::chooseAttackMoveArmiesOrNextPlayer()" << endl;
}

/**
  * Returns a pair of countries where the attacker have enough armies to attack
  * and the defender is a ennemy neighbour of the attacker
  */
std::pair<const Country*, const Country*> AIPlayer::chooseBelligerant()
{
  QMultiMap<const Country*, const Country*> candidates;

//     kDebug() << name() << " : AIPlayer::chooseBelligerant()" << endl;
    // Builds the list of countries of the player that have enough armies and a good neighbour
  QList<Country*> list = countries();
  if ( ! list.empty() )
  {
    QList<Country*>::iterator outer = list.begin();
    const Country* candidateSource;
//        kDebug() << name() << "  choosing belligerants, candidate sources " << endl;
    while ( ( outer != list.end()) && ( (candidateSource = *outer) != 0 ) )
    {
//            kDebug() << name() << "  choosing belligerants, looking at candidate source : " << candidateSource-> name() << endl;
            // Enough armies
      if ( candidateSource-> nbArmies() > 1 )
      {
//                kDebug() << name() << "  choosing belligerants, candidate source has enough armies." << endl;
//                kDebug() << name() << "  choosing belligerants, candidate targets " << endl;
        for ( int inner = 0; inner < m_world-> getCountries().size(); inner++)
        {
          const Country* candidateTarget = m_world-> getCountries().at(inner);
//                    kDebug() << name() << "  choosing belligerants, looking at candidate target : " << candidateTarget-> name() << endl;
                    // Enemy neigbour
          if (            ( candidateTarget != candidateSource ) 
                  &&     (candidateSource-> owner() != candidateTarget-> owner())
                  &&     (candidateSource-> communicateWith(candidateTarget) )
                  &&     !(candidateTarget->name().contains("NULL") ) )
          {
//                        kDebug() << name() << "  choosing belligerants, adding target / source : "
//                                << candidateSource-> name() << " / " << candidateTarget-> name() << endl;
              candidates.insert(candidateSource, candidateTarget);
          }
        }
      }
      ++outer;
    }
    if ( candidates.size() == 0 )
    {
//            kDebug() << name() << " OUT AIPlayer::chooseBelligerant() ; map size = 0 ; it isn't possible to attack." << endl;
      return (std::make_pair(static_cast< Country*>(0), static_cast< Country*>(0)));
    }
    uint which = Dice::roll(candidates.size()) - 1;
//        kDebug() << "Which = " << which <<endl;
    QMultiMap<const Country*, const Country*>::const_iterator it;
    unsigned int i = 0;
    for ( it = candidates.constBegin(); it != candidates.constEnd() ; it++, i++ )
    {
      if (which == i )
      {
//                kDebug() << "OUT AIPlayer::chooseBelligerant() : " << endl ;
         return std::make_pair(it.key(), it.value());
      }
    }
  }
//    kDebug() << "OUT AIPlayer::chooseBelligerant() : do I own no country ???" << endl;
  return (std::make_pair(static_cast< Country* >(0), static_cast< Country* >(0)));
}

/**
  * Chooses a country to receive a new army in dotation. random choice in the 
  * list of the player countries that have at least one neigbour belonging to 
  * someone else
  */
Country* AIPlayer::chooseReceivingCountry()
{
//     kDebug() << "AIPlayer::chooseReceivingCountry()" << endl;
  QList<Country*> myCountries = countries();
  if (myCountries.size() == 0)
  {
    kError() << "AIPlayer::chooseReceivingCountry() EMPTY LIST" << endl;
    return 0;
  }
  QList<Country*> withNeighbours;
  
  for (int i = 0 ; i < myCountries.size(); i++)
  {
    if ( ( m_world-> neighboursNotBelongingTo(*(myCountries.at(i)), static_cast< const Player * >(this) ) ).size() )
        withNeighbours.push_back(myCountries.at(i));
  }

  int which = Dice::roll(withNeighbours.size()) - 1;
  if (which == -1)
  {
    kDebug() << Player::name() << " has no enemy neighbour... should not happen." << endl;
  }
//    kDebug() << "\tChoosed: " << list.at(which)-> name() << endl;
    
  return withNeighbours.at(which);
//    kDebug() << "OUT AIPlayer::chooseReceivingCountry()" << endl;
}

/** 
  * Returns true (an AIPlayer is an AI)
  */
bool AIPlayer::isAI() const
{
  return true;
}

void AIPlayer::MyThread::run()
{
  kDebug() << me.name();
  stopMe = false;
  while ( ! stopMe )
  {
    me.actionChoice(me.m_game->state());
    msleep( 500 );
  }
  kDebug() << "OUT";
}

/** set stopMe to true in order for the run method to return */
void AIPlayer::stop()
{
  m_thread.setStopMe(true);
}

/**
  * make all what is necessary to prepare and launch an attack
  */
bool AIPlayer::attackAction() 
{
//   kDebug() << "AIPlayer::attackAction" << endl;
  std::pair<const Country* , const Country* > srcDest = chooseBelligerant();
  if ( (srcDest.first == 0) || (srcDest.second == 0) )
  {
//     kDebug() << "AIPlayer::attackAction: no attack available" << endl;
//       nextPlayerAction(); 
      return false;
  }
  m_src = srcDest.first;
  m_dest = srcDest.second;

  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << QString("actionLButtonDown") << m_src->centralPoint();
  aiPlayerIO()->sendInput(stream,true);

  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  stream2 << QString("actionLButtonUp") << m_dest->centralPoint();
  aiPlayerIO()->sendInput(stream2,true);

  uint srcNbArmies = m_src->nbArmies();
  kDebug() << Player::name()  << " : ATTACK" << endl;
  kDebug() << "    " << Player::name()  << " : attacks from "  << m_src-> name() 
                  << " (" << srcNbArmies << " armies)" << endl;
  kDebug() << "    " << Player::name()  << " : attacks " << m_dest-> name() << endl;
      
  uint nbAttack = 0;
  if (srcNbArmies == 1)
  {
    kError() << "AI player " << Player::name() << " country " << m_src->nbArmies() << "have only one army. Should not be chosen to attack." << endl;
    m_thread.exit();
  }
  if (srcNbArmies >= 2) {nbAttack = 1;}
  if (srcNbArmies >= 3) {nbAttack = 2;}
  if (srcNbArmies >= 4) {nbAttack = 3;}
  m_nbAttack = nbAttack;
  kDebug() << "    " << Player::name()  << " : attacks with " << nbAttack << " armies." << endl;

  QPointF point;
  QByteArray buffer3;
  QDataStream stream3(&buffer3, QIODevice::WriteOnly);
  switch (nbAttack)
  {
    case 1: 
    stream3 << QString("actionAttack1") << point;
    break;
    case 2: 
    stream3 << QString("actionAttack2") << point;
    break;
    case 3: 
    stream3 << QString("actionAttack3") << point;
    break;
    default:
      kError() << "The attacker tries to attack with a number of armies different of 1, 2 or 3: that's impossible!" << endl;
      m_thread.exit();
  }
  aiPlayerIO()->sendInput(stream3,true);
//   requestAck();

  kDebug() << "AIPlayer " << Player::name()  << " : attackAction : "  << m_src-> name() << " " << m_dest-> name()
    << " " << nbAttack << endl;
  stop();
  return true;
}

/**
  * makes all what is necessary to prepare and start the moving of armies
  */
bool AIPlayer::moveArmiesAction()
{
  kDebug() << "AIPlayer::moveArmiesAction" << endl;
  QList<Country*> srcList = countries() ;
  if (srcList.size() == 0) 
  {
    nextPlayerAction(); 
    return false;
  }
  uint which = Dice::roll(srcList.size()) - 1;
  Country* osrc =  srcList.at(which);
//    kDebug() << "AIPlayer::moveArmiesAction() MOVEARMIES 1" << endl;
  if (osrc-> nbArmies() <= 1) 
  {
    nextPlayerAction();
    return false;
  }
//    kDebug() << "AIPlayer::moveArmiesAction() MOVEARMIES 2" << endl;
  QList<Country*> destList( m_world-> neighboursBelongingTo(*osrc, this) );
//    kDebug() << "AIPlayer::moveArmiesAction() MOVEARMIES 3" << endl;
  if (destList.size() == 0) 
  {
    nextPlayerAction();
    return false;
  }
  which = Dice::roll(destList.size()) - 1 ;
//    kDebug() << "AIPlayer::moveArmiesAction() MOVEARMIES 4" << endl;
  Country* odest =  destList.at(which);
  m_src = osrc;
  m_dest = odest;

  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << QString("actionLButtonDown") << osrc->centralPoint();
  aiPlayerIO()->sendInput(stream,true);

  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  stream2 << QString("actionLButtonUp") << odest->centralPoint();
  aiPlayerIO()->sendInput(stream2,true);
//   requestAck();
  
  kDebug() << "AIPlayer ****************" << Player::name()  << " : moveAction : "  << osrc-> name() << " " << odest-> name() << endl;

  return true;
}

/**
 * chooses a country where to place a new army
 */
void AIPlayer::placeArmiesAction() 
{
  kDebug() << "AIPlayer::placeArmiesAction " << Player::name() << " ; nb to place: " << getNbAvailArmies() << endl;
  if (getNbAvailArmies() > 0)
  {
    m_hasVoted = false;
    const Country* receiver = chooseReceivingCountry();
    if (receiver == 0)
    {
      QString msg = i18n("Error - No receiving country selected while computer player %1 had still %2 armies to place. This is bug probably #2232 at www.gna.org.", Player::name(), getNbAvailArmies());
      kError() << msg;
      KMessageBox::error(0, msg, i18n("Fatal Error"));
      m_thread.exit();
      m_thread.wait();
    }
    kDebug() << "Placing an army in " << receiver->name() 
        << " ; point=" << receiver->centralPoint() << endl;
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << QString("actionLButtonDown") << receiver->centralPoint();
    aiPlayerIO()->sendInput(stream,true);

//     requestAck();
  }
  else if (m_game->state() != GameAutomaton::INTERLUDE)
  {
    kDebug() << "No more armies to place: next player" << endl;
    stop();
    QPointF point;
    m_game->gameEvent("actionNextPlayer", point);
  }
  else
  {
    chooseWetherToRecycle();
  }
  
}

/**
 * Takes the decision to recycle armies or not
 */
void AIPlayer::chooseWetherToRecycle() 
{
  kDebug() << Player::name();
  if (m_game->allLocalPlayersComputer())
  {
    if (!m_hasVoted)
    {
      kDebug() << "Voting for end of recycling" << endl;
      QPointF p;
      m_game->gameEvent( "actionRecyclingFinished", p );
      m_hasVoted = true;
    }
    else
    {
      kDebug() << "Has already voted" << endl;
    }
  }
  else
  {
    kDebug() << "There is local non computer players; let them vote." << endl;
  }
  stop();
}

/**
 * chooses to continue invasion with a certain amount of armies or to stop it
 */
void AIPlayer::chooseInvasionAction()
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  kDebug() << QString("AIPlayer::chooseInvasionAction") << endl;
  int nbArmiesToMove = Dice::roll(m_game->game()-> firstCountry()-> nbArmies()) - 1;
  QPointF point;
  while (nbArmiesToMove >= 10) 
  {
    stop(); 
    stream << QString("actionInvade10") << point;
    nbArmiesToMove -= 10;
  }
  while (nbArmiesToMove >= 5) 
  { 
    stop();
    stream << QString("actionInvade5") << point;
    nbArmiesToMove -= 5;
  }
  while (nbArmiesToMove >= 1) 
  { 
    stop();
    stream << QString("actionInvade1") << point;
    nbArmiesToMove--;
  }
  stream << QString("actionInvasionFinished") << point;
  stop();
  aiPlayerIO()->sendInput(stream,true);
//   requestAck();
}

/**
  * chooses whether to defend with one or two armies. Always chooses the maximum possible
  */
void AIPlayer::chooseDefenseAction()
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  QPointF point;
  if ((m_game-> currentPlayer() == this) && ((!m_game->isDefenseAuto()) || (m_game->game()->secondCountry() != m_game->getDefCountry())))
  {
    kDebug() << "AIPlayer::chooseDefenseAction waiting defense of another one; nothing to do." << endl;
    m_game->setDefenseAuto(false);
  }
  else
  {
    kDebug() << "AIPlayer::chooseDefenseAction " << Player::name() << endl;
    switch (m_game-> currentPlayer()-> getNbAttack())
    {
      case 1:
      stream << QString("slotDefense1") << point;
//         m_game->slotDefense1();
      break;
      case 2: ; case 3:
        if (m_game-> game()-> secondCountry()-> nbArmies() > 1)
          stream << QString("slotDefense2") << point;
//       m_game->slotDefense2();
        else
          stream << QString("slotDefense1") << point;
//       m_game->slotDefense1();
      break;
      default:
        kError() << "The attacker attacks with a number of armies different of 1, 2 or 3: that's impossible!" << endl;
        m_thread.exit();
    }
    stop();
    aiPlayerIO()->sendInput(stream,true);
//     requestAck();
  }
}

/**
 * makes what is necessary to finish my turn
 */
void AIPlayer::nextPlayerAction()
{
  kDebug() << "AIPlayer::nextPlayerAction" << endl;
  QPointF point;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << QString("actionNextPlayer") << point;
  aiPlayerIO()->sendInput(stream,true);
//   requestAck();
  stop();
}

void AIPlayer::saveXml(std::ostream& xmlStream)
{
  xmlStream << "<player ai=\"true\" ";
  innerSaveXml(xmlStream);
  xmlStream << " />" << std::endl;
}

/** 
 * Makes the choice of nb armies to move during an invasion or an end of turn
 * moving 
 */
void AIPlayer::chooseNbToMoveOrStop()
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  QPointF point;
  if (m_toMove == std::numeric_limits< unsigned int>::max())
  {
    if (m_src == 0)
    {
      m_toMove = 0;
    }
    else
    {
      m_toMove = Dice::roll(m_src->nbArmies() - 1);
    }
  }
  if (m_toMove >= 10)
  {
    stream << QString("actionInvade10") << point;
    m_toMove -= 10;
  }
  else if (m_toMove >= 5)
  {
    stream << QString("actionInvade5") << point;
    m_toMove -= 5;
  }
  else if (m_toMove >= 1)
  {
    stream << QString("actionInvade1") << point;
    m_toMove -= 1;
  }
  else // m_toMove == 0
  {
    stream << QString("actionInvasionFinished") << point;
    m_toMove = std::numeric_limits< unsigned int>::max();
    stop();
  }
  aiPlayerIO()->sendInput(stream,true);
}

AIPlayerIO* AIPlayer::aiPlayerIO()
{
  KGameIOList* iolist = ioList();
  for ( int i = 0; i < iolist->count(); ++i )
    if ( iolist->at(i) && iolist->at(i)->rtti() == AIPLAYERIO )
      return dynamic_cast<AIPlayerIO*>(iolist->at(i));
  
  return 0;
}

void AIPlayer::requestAck()
{
  QMutexLocker locker(&m_waitedAckMutex);
  m_waitedAck = QUuid::createUuid().toString();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  QPointF p;
  kDebug() << name() << " sending a request for ack " << m_waitedAck << endl;
  stream << QString("requestForAck") << p << m_waitedAck;
  aiPlayerIO()->sendInput(stream,true);
}

} // closing namespace GameLogic
} // closing namespace Ksirk

#include "aiplayer.moc"
