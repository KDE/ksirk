// kate: space-indent on; indent-width 2; replace-tabs on;
/* This file is part of KsirK.
   Copyright (C) 2006-2007 Gael de Chalendar <kleag@free.fr>

   This file was initialy part of XFrisk
   Copyright (C) 1995 and later Jean-Claude Colson and Others <who@nowhere.org>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   *   You should have received a copy of the GNU General Public License
   *   along with this program; if not, write to the Free Software
   *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   *   02110-1301, USA
   */
/***********************************************************************
 *
 *   18-8-95  Created by Jean-Claude Colson.
 *
 *   RISK game player.
 *   $Id: aiColson.c,v 1.5 1999/11/07 15:57:29 tony Exp $
 *
 ***********************************************************************/

#include "aiColsonPlayer.h"

#include "onu.h"
#include "goal.h"
#include "continent.h"
#include "gameautomaton.h"
#include "kgamewin.h"
#include "xfriskaiclient.h"
#include "aiplayerio.h"
#include "dice.h"

#include <assert.h>

namespace Ksirk
{

namespace GameLogic
{


AIColsonPlayer::AIColsonPlayer(
        const QString & nomPlayer, unsigned int nbArmies,
        Nationality * myNation,  PlayersArray& players, ONU* world,
        GameAutomaton* game ) :
  AIPlayer(nomPlayer, nbArmies, myNation, players, world, game ),
  m_levelEnemy(0),
  m_initialized(false),
  Attack_SrcCountry(-1),
  Attack_DestCountry(-1),
  m_placeData(0),
  m_nbArmiesToMove(-1)
{
  kDebug() << "AIColsonPlayer constructor" << endl;
}

AIColsonPlayer::~AIColsonPlayer() 
{
}

//////////////////////////////////////////////////////////
// Virtual functions reimplemented from AIPlayer
//////////////////////////////////////////////////////////

std::pair< const Country*, const Country* > AIColsonPlayer::chooseBelligerant()
{
Fortify();
  kDebug() << "AIColsonPlayer::chooseBelligerant" << endl;
  Country* src = 0;
  Country* dest = 0;
  
//   Attack_SrcCountry = -1;
//   Attack_DestCountry = -1;

  // no attack was tempted
  if (!Attack())
  {
    Attack_SrcCountry = -1;
    Attack_DestCountry = -1;
    return std::make_pair(static_cast<Country*>(0), static_cast<Country*>(0));
  }

  if ( (Attack_SrcCountry>=0) && (Attack_SrcCountry<m_world->getCountries().size() ) )
    src = m_world->getCountries().at(Attack_SrcCountry);
  if ( (Attack_DestCountry>=0) && (Attack_DestCountry<m_world->getCountries().size() ) )
    dest = m_world->getCountries().at(Attack_DestCountry);
//   kDebug() << "chose belligerants " << src << " and " << dest << endl;
  return std::make_pair(src,dest);
}

/**
  * Chooses the next action. Attack by default and if no attack is possible,
  * try to move armies and in the last resort, choose next player
  */
void AIColsonPlayer::chooseAttackMoveArmiesOrNextPlayer()
{
  kDebug() << "AIColsonPlayer::chooseAttackMoveArmiesOrNextPlayer() " << endl;
  if (m_game->game()->haveAnimFighters() || m_game->game()->haveMovingArmies())
  {
    return;
  }
  if (!m_initialized)
  {
    finalize();
  }

  if (!attackAction())
  {
    if (!moveArmiesAction())
    {
      nextPlayerAction();
    }
  }
//    kDebug() <<"OUT AIColsonPlayer::chooseAttackMoveArmiesOrNextPlayer()" << endl;
}

/**
  * Chooses a country to receive a new army in dotation
  */
Country* AIColsonPlayer::chooseReceivingCountry()
{
  kDebug() << "AIColsonPlayer::chooseReceivingCountry" << endl;
  if (m_placeData == 0)
  {
    if (!Place())
    {
      return 0;
    }
  }
  Country* res = m_placeData->dest;
  m_placeData->nb--;
  if (m_placeData->nb == 0)
  {
    delete m_placeData;
    m_placeData = 0;
  }
  return res;
}

/**
 * chooses to continue invasion with a certain amount of armies or to stop it
 */
void AIColsonPlayer::chooseInvasionAction()
{
  kDebug() << "AIColsonPlayer::chooseInvasionAction" << endl;
  kDebug() << "    Attack_SrcCountry  = " << Attack_SrcCountry << endl;
  kDebug() << "    Attack_DestCountry = " << Attack_DestCountry << endl;
  if (Attack_SrcCountry == - 1 || Attack_DestCountry == -1)
  {
    return;
  }

  if (m_nbArmiesToMove < 0)
  {
    int nbEnemiesAdjacentToSrc = NbToEqualEnemyAdjacent(m_world->getCountries().at(Attack_SrcCountry));
    int nbEnemiesAdjacentToDest = NbToEqualEnemyAdjacent(m_world->getCountries().at(Attack_DestCountry));
    kDebug() << "    nb on src  = " << RISK_GetNumArmiesOfCountry(Attack_SrcCountry) << endl;
    kDebug() << "    nb adj to src  = " << nbEnemiesAdjacentToSrc << endl;
    kDebug() << "    nb adj to dest = " << nbEnemiesAdjacentToDest << endl;
    int diff = nbEnemiesAdjacentToDest - nbEnemiesAdjacentToSrc;
    kDebug() << "    diff  = " << diff << endl;
  
  
    
    m_nbArmiesToMove = (diff>RISK_GetNumArmiesOfCountry(Attack_SrcCountry)-1)?RISK_GetNumArmiesOfCountry(Attack_SrcCountry)-1:diff;
    if (m_nbArmiesToMove < 0)
      m_nbArmiesToMove = 0; 
    kDebug() << "    moves " << m_nbArmiesToMove << endl;
  }

  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  QPoint point;
  if (m_nbArmiesToMove >= 10) 
  {
    kDebug() << "    choosing actionInvade10" << endl;
    stop(); 
    stream << QString("actionInvade10") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_nbArmiesToMove -= 10;
  }
  else if (m_nbArmiesToMove >= 5) 
  { 
    kDebug() << "    choosing actionInvade5" << endl;
    stop();
    stream << QString("actionInvade5") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_nbArmiesToMove -= 5;
  }
  else if (m_nbArmiesToMove >= 1) 
  { 
    kDebug() << "    choosing actionInvade1" << endl;
    stop();
    stream << QString("actionInvade1") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_nbArmiesToMove--;
  }
  else
  {
    kDebug() << "    choosing actionInvasionFinished" << endl;
    stop();
    stream << QString("actionInvasionFinished") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_nbArmiesToMove = -1;
  }
}

/**
  * makes all what is necessary to prepare and start the moving of armies
  */
bool AIColsonPlayer::moveArmiesAction() 
{
  kDebug() << "AIColsonPlayer::moveArmiesAction" << endl;
  bool res = Move();
  kDebug() << "AIColsonPlayer::moveArmiesAction Move got " << res << endl;
  return res;
}

//////////////////////////////////////////////////////////
// XFrisk AIColson functions
//////////////////////////////////////////////////////////
int AIColsonPlayer::getTotalArmiesOfPlayer(const Player* player)
{
  int nb = 0;
  for (unsigned int i=0; i<m_world->getCountries().size(); i++)
  {
    if (m_world->getCountries().at(i)->owner() == player)
    {
      nb += m_world->getCountries().at(i)->nbArmies();
    }
  }
  return nb;
}


bool AIColsonPlayer::isContinentOfMission(const Player* player, const Continent* continent)
{
  if (player->goal().type() != Goal::Continents)
      return false;
  return (player->goal().continents().find(continent->id()) != player->goal().continents().end());
}

bool AIColsonPlayer::isEnemyPlayer(const Player* player)
{
  return m_isEnemyPlayer[player];
}

bool AIColsonPlayer::isFriendPlayer(const Player* player)
{
return (player==this);
//   return m_isEnemyPlayer[player] < m_levelEnemy;
}

int AIColsonPlayer::getNumEnemy()
{
  kDebug() << "AIColsonPlayer::getNumEnemy" << endl;
  int nb = 0;

  PlayersArray::iterator it = m_game->playerList()->begin();
  PlayersArray::iterator it_end = m_game->playerList()->end();
  for (; it != it_end; it++)
  {
    if (m_isEnemyPlayer[((Player*)(*it))]>= m_levelEnemy)
    {
      nb++;
    }
  }
  if (m_levelEnemy == 1)
      nb--;
  return(nb);
}

bool AIColsonPlayer::isStrongerPlayer(const Player* player)
{
  int nb;

  nb = getTotalArmiesOfPlayer(player);
  nb = nb + nb/5;
  PlayersArray::iterator it = m_game->playerList()->begin();
  PlayersArray::iterator it_end = m_game->playerList()->end();
  for (; it != it_end; it++)
  {
    if (getTotalArmiesOfPlayer(((Player*)(*it))) > nb)
    {
      return false;
    }
  }
  return true;
}

bool AIColsonPlayer::isSmallerPlayer(const Player* player)
{
  int nb = 3 * getTotalArmiesOfPlayer(player);
  PlayersArray::iterator it = m_game->playerList()->begin();
  PlayersArray::iterator it_end = m_game->playerList()->end();
  for (; it != it_end; it++)
  {
    if (getTotalArmiesOfPlayer(((Player*)(*it))) > nb)
    {
      return false;
    }
  }
  return false;
}

bool AIColsonPlayer::isContinentOfPlayer(const Continent* continent, const Player* player)
{
  if (continent == 0)
  {
    return false;
  } 
  return (continent->owner() == player);
}

const Continent* AIColsonPlayer::computeChoiceOfContinent(void)
{
//   kDebug() << "AIColsonPlayer::computeChoiceOfContinent" << endl;
  std::map< const KPlayer*, std::map <const Continent*, int > > piCount;
  std::map< const KPlayer*, std::map <const Continent*, bool > > maxContinent;
  std::map< const KPlayer*, std::map <const Continent*, bool > > posContinent;
  std::map<const KPlayer*, const Continent*> m_piContinent;

  std::vector<Continent*>::iterator continentsIt = (m_world->getContinents().begin());
  std::vector<Continent*>::iterator continentsIt_end = (m_world->getContinents().end());

  PlayersArray::iterator it = m_game->playerList()->begin();
  PlayersArray::iterator it_end = m_game->playerList()->end();
  for (; it != it_end; it++)
  {
    
    continentsIt = m_world->getContinents().begin();
    continentsIt_end = m_world->getContinents().end();
    for (; continentsIt != continentsIt_end; continentsIt++)
    {
      Continent *continent = *continentsIt;
//       kDebug() << "    " << continent << endl;
      if (continent == 0) continue;
//       kDebug() << "    " << continent->name() << endl;
      piCount[((Player*)(*it))][continent] = 0;
      maxContinent[((Player*)(*it))][continent] = false;
      posContinent[((Player*)(*it))][continent] = false;
    }
  }

  for (unsigned int i=0; i<m_world->getCountries().size(); i++)
  {
/*  std::vector<Country*>::iterator countriesIt(m_world->getCountries().begin());
  std::vector<Country*>::iterator countriesIt_end(m_world->getCountries().end());
  for (; countriesIt != countriesIt_end; countriesIt++)
  {*/
    Country* country = m_world->getCountries().at(i);
//     std::cerr << "country " << country << std::endl;
//     std::cerr << "country " << country->name().toUtf8().data() << std::endl;
    if ( country->owner() != 0 && country->continent() != 0)
    {
      piCount[country->owner()][country->continent()]++;
//       std::cerr << "piCount[" << country->owner()->name().toUtf8().data() << "][" << country->continent()->name().toUtf8().data() << "] " << std::endl;
//       std::cerr << " is now " << piCount[country->owner()][country->continent()] << std::endl;
    }
    else 
    {
//       std::cerr << "owner or continent of country " << country->name().toUtf8().data() << " is null..." << std::endl; 
    }
  }

  it = m_game->playerList()->begin();
  it_end = m_game->playerList()->end();
  for (; it != it_end; it++)
  {
    int min = 10000;
    int max = 0;
    int bonus = 0;
    for (continentsIt = m_world->getContinents().begin();
          continentsIt!=continentsIt_end; continentsIt++)
    {
      const Continent* continent = *continentsIt;
      if (piCount[*it][continent] > 0)
      {
        if (min == 0)
        {
          if ( (piCount[*it][continent] == continent->getMembers().size())
                && (continent->getBonus()>bonus))
          {
            std::cerr << "pt2" << std::endl;
            max = piCount[*it][continent];
            bonus = continent->getBonus();
            maxContinent[*it][continent] = true;
            m_piContinent[*it] = continent;
          }
        }
        else if (piCount[*it][continent] == continent->getMembers().size())
        {
          min = 0;
          max = piCount[*it][continent];
          maxContinent[*it][continent] = true;
          m_piContinent[*it] = continent;
        }
        else if (piCount[*it][continent] > max)
        {
          std::vector<Continent*>::iterator jit(m_world->getContinents().begin());
          std::vector<Continent*>::iterator jit_end(m_world->getContinents().end());
          for (; jit!=jit_end; jit++)
          {
            Continent* j = *jit;
            maxContinent[*it][j] = false;
            if (piCount[*it][j] < max-1)
                posContinent[*it][j] = (piCount[*it][continent] < continent->getMembers().size()/2);
            else
                posContinent[*it][j] = true;
          }
          min = continent->getMembers().size() - piCount[*it][continent];
          max = piCount[*it][continent];
          bonus = continent->getBonus();
          maxContinent[*it][continent] = true;
        }
        else if (   (piCount[*it][continent] == max)
                  &&  isContinentOfMission(dynamic_cast<Player*>(*it), continent))
        {
          min = continent->getMembers().size() 
                - piCount[*it][continent];
          bonus = 2 * continent->getBonus();
          maxContinent[*it][continent] = true;
        }
        else if (   (piCount[*it][continent] == max)
                  && (continent->getBonus() > bonus))
        {
          min = continent->getMembers().size() - piCount[*it][continent];
          bonus = continent->getBonus();
          maxContinent[*it][continent] = true;
        }
        else if (piCount[*it][continent] >= continent->getMembers().size()/2)
        {
          posContinent[*it][continent] = true;
        }
      }
    }
  }

  /* Search a continent with no conflict */
  const Continent* continent = 0;
  std::vector<Continent*>::iterator contIt(m_world->getContinents().begin());
  std::vector<Continent*>::iterator contIt_end(m_world->getContinents().end());
  for (;(contIt!=contIt_end) && (continent==0);contIt++)
  {
    const Continent* cont = *contIt;
    
    if (maxContinent[this][cont])
    {
      continent = cont;

      PlayersArray::iterator iit = m_game->playerList()->begin();
      PlayersArray::iterator iit_end = m_game->playerList()->end();
      for (;iit!=iit_end;iit++)
      {
        Player* i = static_cast<Player*>(*iit);
        if (i != this) 
        {
          if ( m_piContinent[i] == cont)
              continent = *(m_world->getContinents().begin());
          else if (    (maxContinent[i][cont])
                    && (piCount[i][cont] > piCount[this][cont]))
            continent = 0;
        }
      }
    }
  }

  if (isFriendPlayer(this) && (continent!=0))
  {
    std::vector<Continent*>::iterator contIt(m_world->getContinents().begin());
    std::vector<Continent*>::iterator contIt_end(m_world->getContinents().end());
#warning continent can not be 0 in this code path - everything in this for loop is dead code
    for (;(contIt!=contIt_end) && (continent==0);contIt++)
    {
      const Continent* cont = *contIt;
      if (cont != continent)
      {
        maxContinent[this][cont] = false;
        posContinent[this][cont] = false;
      }
    }
    PlayersArray::iterator iit = m_game->playerList()->begin();
    PlayersArray::iterator iit_end = m_game->playerList()->end();
    for (;iit!=iit_end;iit++)
    {
      Player* i = static_cast<Player*>(*iit);
      posContinent[i][continent] = false;
    }
    return continent;
  }

  /* Search a continent with no computer conflict */
  continent = 0;
  contIt = m_world->getContinents().begin();
  contIt_end = m_world->getContinents().end();
  for (;(contIt!=contIt_end) && (continent==0);contIt++)
  {
    const Continent* cont = *contIt;
    if (maxContinent[this][cont])
    {
      continent = cont;
      PlayersArray::iterator iit = m_game->playerList()->begin();
      PlayersArray::iterator iit_end = m_game->playerList()->end();
      for (;iit!=iit_end;iit++)
      {
        Player* i = static_cast<Player*>(*iit);
        if (isFriendPlayer(i) && (i != this)) 
        {
          if ( m_piContinent[i] == cont)
              continent = 0;
          else if (    (maxContinent[i][cont])
                    && (piCount[i][cont] > piCount[this][cont]))
              continent = 0;
        }
      }
    }
  }

  if (continent!=0)
  {
    std::vector<Continent*>::iterator contIt(m_world->getContinents().begin());
    std::vector<Continent*>::iterator contIt_end(m_world->getContinents().end());
    for (;contIt!=contIt_end;contIt++)
    {
      const Continent* cont = *contIt;
    //for (cont=0; cont!=m_world->getContinents().count(); cont++)
      if (cont != continent)
      {
        maxContinent[this][cont] = false;
        posContinent[this][cont] = false;
      }
    }
    PlayersArray::iterator iit = m_game->playerList()->begin();
    PlayersArray::iterator iit_end = m_game->playerList()->end();
    for (;iit!=iit_end;iit++)
    {
      Player* pi = static_cast<Player*>(*iit);
      posContinent[pi][continent] = false;
    }
    std::cerr << "pt5" << std::endl;
    return continent;
  }


  /* Search a possible continent with no conflict */
  continent = 0;
  contIt = m_world->getContinents().begin();
  contIt_end = m_world->getContinents().end();
  for (;(contIt!=contIt_end) && (continent==0);contIt++)
  {
    const Continent* cont = *contIt;
    if (posContinent[this][cont])
    {
      continent = cont;
      PlayersArray::iterator iit = m_game->playerList()->begin();
      PlayersArray::iterator iit_end = m_game->playerList()->end();
      for (;iit!=iit_end;iit++)
      {
        Player* pi = static_cast<Player*>(*iit);
        if (isFriendPlayer(pi) && (pi != this)) 
        {
          if ( m_piContinent[pi] == cont)
              continent = 0;
          else if (    (maxContinent[pi][cont])
                    && (piCount[pi][cont] > piCount[this][cont]))
              continent = 0;
        }
      }
    }
  }

  if (continent!=0)
  {
    std::vector<Continent*>::iterator contIt(m_world->getContinents().begin());
    std::vector<Continent*>::iterator contIt_end(m_world->getContinents().end());
    for (;contIt!=contIt_end;contIt++)
    {
      const Continent* cont = *contIt;
    //for (cont=0; cont!=m_world->getContinents().count(); cont++)
      if (cont != continent)
      {
        maxContinent[this][cont] = false;
        posContinent[this][cont] = false;
      }
    }
    PlayersArray::iterator iit = m_game->playerList()->begin();
    PlayersArray::iterator iit_end = m_game->playerList()->end();
    for (;iit!=iit_end;iit++)
    {
      Player* pi = static_cast<Player*>(*iit);
      posContinent[pi][continent] = false;
    }
    std::cerr << "pt6" << std::endl;
    return continent;
  }

  /* Search a possible continent with no computer conflict */
  continent = 0;
  contIt = m_world->getContinents().begin();
  contIt_end = m_world->getContinents().end();
  for (;(contIt!=contIt_end) && (continent==0);contIt++)
  {
    const Continent* cont = *contIt;
    if (posContinent[this][cont])
    {
      continent = cont;
      PlayersArray::iterator iit = m_game->playerList()->begin();
      PlayersArray::iterator iit_end = m_game->playerList()->end();
      for (;iit!=iit_end;iit++)
      {
        Player* pi = static_cast<Player*>(*iit);
        if (isFriendPlayer(pi) && (pi != this)) 
        {
          if ( m_piContinent[pi] == cont)
              continent = 0;
          else if (    (maxContinent[pi][cont])
                    && (piCount[pi][cont] > piCount[this][cont]))
              continent = 0;
        }
      }
    }
  }

  if (continent!=0)
  {
    std::vector<Continent*>::iterator contIt(m_world->getContinents().begin());
    std::vector<Continent*>::iterator contIt_end(m_world->getContinents().end());
    for (;contIt!=contIt_end;contIt++)
    {
      const Continent* cont = *contIt;
      if (cont != continent)
      {
        maxContinent[this][cont] = false;
      }
    }
    std::cerr << "pt7" << std::endl;
    return continent;
  }

  continent = 0;

  /* Search a continent */
  contIt = m_world->getContinents().begin();
  contIt_end = m_world->getContinents().end();
  for (;(contIt!=contIt_end) && (continent==0);contIt++)
  {
    const Continent* cont = *contIt;
    if (piCount[this][cont]>0)
    {
      continent = cont;
      PlayersArray::iterator iit = m_game->playerList()->begin();
      PlayersArray::iterator iit_end = m_game->playerList()->end();
      for (;iit!=iit_end;iit++)
      {
        Player* pi = static_cast<Player*>(*iit);
        if (isFriendPlayer(pi) && (pi != this))
          continent = 0;
      }
    }
  }

  if (continent!=0)
  {
    std::vector<Continent*>::iterator contIt(m_world->getContinents().begin());
    std::vector<Continent*>::iterator contIt_end(m_world->getContinents().end());
    for (;contIt!=contIt_end;contIt++)
    {
      const Continent* cont = *contIt;
      if (cont!=continent)
      {
        maxContinent[this][cont] = false;
      }
    }
    std::cerr << "pt8" << std::endl;
    return continent;
  }

  continent = 0;

  /* Search a continent */
  contIt = m_world->getContinents().begin();
  contIt_end = m_world->getContinents().end();
  for (;(contIt!=contIt_end) && (continent==0);contIt++)
  {
    const Continent* cont = *contIt;
    if (maxContinent[this][cont])
        continent = cont;
  }

//   kDebug() << "AIColsonPlayer::computeChoiceOfContinent for " << name().toUtf8().data()
//     << " is " << continent << endl;
  return continent;
}

/**
  * Gets the continent already selected for this player
  * @param attack This is the number of countries of the selected continent
  * that have at least one adjacent enemy
  */
const Continent* AIColsonPlayer::GetContinentToFortify(int *attack)
{
  kDebug() << "AIColsonPlayer::GetContinentToFortify" << endl;
  const Continent* continent = computeChoiceOfContinent();
  kDebug() << "  computeChoiceOfContinent got " << continent << endl;
  *attack = 0;
  for (int i=0; i!=m_world->getCountries().size(); i++)
  {
    if (    (RISK_GetOwnerOfCountry(i) == this)
           && (RISK_GetContinentOfCountry(i) == continent)
           &&  GAME_IsEnemyAdjacent(i))
    {
      (*attack)++;
    }
  }
  kDebug() << "AIColsonPlayer::GetContinentToFortify found " << continent << endl;
  return(continent);
}

/** 
  * @param attack Number of countries with adjacent ennemies in the selected
  * continent
  */
const Continent* AIColsonPlayer::getContinentToConquier(int *attack)
{
  kDebug() << "AIColsonPlayer::getContinentToConquier" << endl;
  std::map<const Continent*, int> piCount;
  std::map<const Continent*, int> piOppo;
  std::map<const Continent*, int> piAttac;

  /* Init. */
  std::vector<Continent*>::iterator contIt(m_world->getContinents().begin());
  std::vector<Continent*>::iterator contIt_end(m_world->getContinents().end());
  for (;contIt!=contIt_end;contIt++)
  {
    const Continent* cont = *contIt;
    piCount[cont] = 0;
    piOppo[cont] = 0;
    piAttac[cont] = 0;
  }

  /* Count up how many countries the player has in each of the continents */
  std::vector<Country*>::iterator countriesIt(m_world->getCountries().begin());
  std::vector<Country*>::iterator countriesIt_end(m_world->getCountries().end());
  for (; countriesIt != countriesIt_end; countriesIt++)
  {
    Country* country = *countriesIt;
    if (country->owner() == this)
    {
      piCount[country->continent()]++;
      piOppo[country->continent()] -= country->nbArmies();
      if (country->hasAdjacentEnemy())
        piAttac[country->continent()]++;
    }
    else
    {
      if (isEnemyPlayer(country->owner()))
        piOppo[country->continent()] += country->nbArmies();
      else
        piOppo[country->continent()] += 2 * country->nbArmies();
    }
  }

  const Continent* continent = 0;
  int min = 10000;
  int max = 0;
  int bonus = 0;
  contIt = m_world->getContinents().begin();
  contIt_end = m_world->getContinents().end();
  for (; contIt != contIt_end; contIt++)
  {
    const Continent* cont = *contIt;
    kDebug() << "    " << cont << endl;
    if (cont != 0)
      kDebug() << "    " << cont->name() << endl;
    if (isContinentOfMission(this, cont))
    {
      piOppo[cont] -= piOppo[cont]/3;
    }
    if (piCount[cont] < cont->getMembers().size())
    {
      if (piOppo[cont] < min)
      {
        continent = cont;
        min = piOppo[cont];
        max = piCount[cont];
        bonus = cont->getBonus();
      }
    }
  }

  *attack = piAttac[continent];
  return(continent);
}

int AIColsonPlayer::NbEnemyAdjacent(Country* iCountry)
{
  std::map<Player*, int> NumEnemy;
  std::map<Player*, bool> fIsEnemy;
  bool  fIAmStrong;

  Player* player = iCountry->owner();
  fIAmStrong = isStrongerPlayer(player);
  PlayersArray::iterator pit = m_game->playerList()->begin();
  PlayersArray::iterator pit_end = m_game->playerList()->end();
  for (; pit != pit_end; pit++)
  {
    fIsEnemy[(Player*)(*pit)] = false;
  }
  Player* iEnemy = 0;
  Country* destCountry;
  std::vector< Country* >::const_iterator it, it_end;
  it = iCountry->neighbours().begin();
  it_end = iCountry->neighbours().end();
  for (;it != it_end; it++)
    {
      destCountry = *it;
      iEnemy = destCountry->owner();
      if (iEnemy != player)
      {
        if (NumEnemy[iEnemy] < destCountry->nbArmies())
            NumEnemy[iEnemy] = destCountry->nbArmies();
        fIsEnemy[iEnemy] = true;
      }
    }
  int max = 0;
  pit = m_game->playerList()->begin();
  pit_end = m_game->playerList()->end();
  for (; pit != pit_end; pit++)
  {
    Player* i = (Player*)(*pit);
    if ((i != player) && fIsEnemy[i])
    {
      int nb = NumEnemy[iEnemy];
      if (!fIAmStrong && isStrongerPlayer(i))
          nb = nb + 10;
      if (isEnemyPlayer(i))
          nb = nb + 5;
      if (nb > max)
          max = nb;
    }
  }
  return max;
}

int AIColsonPlayer::NbToAverageEnemyAdjacent(Country* iCountry)
{
  int nbe = 0;
  int nb = 0;
  int i = 0;
  std::vector< Country* >::const_iterator neighbourIt, neighbourIt_end;
  neighbourIt = iCountry->neighbours().begin();
  neighbourIt_end = iCountry->neighbours().end();
  for (;neighbourIt != neighbourIt_end; neighbourIt++)
  {
    Country* destCountry = *neighbourIt;
    Player* iEnemy = destCountry->owner();
    if (iEnemy != this)
    {
      nbe -= destCountry->nbArmies();
      nb++;
    }
    i++;
  }
  int nbi = iCountry->nbArmies();
  nbe = (nbe + nbi)/nb;
//   if ((nbi - nbe)< 10)
//       nbe = nbi - 10;
  kDebug() << "NbToAverageEnemyAdjacent of " << iCountry->name() << " is " << nbe << endl;
  return nbe;
}


int AIColsonPlayer::NbToEqualEnemyAdjacent(Country* iCountry)
{
  int nbe = 0;
  std::vector< Country* >::const_iterator neighbourIt, neighbourIt_end;
  neighbourIt = iCountry->neighbours().begin();
  neighbourIt_end = iCountry->neighbours().end();
  for (;neighbourIt != neighbourIt_end; neighbourIt++)
  {
    Country* destCountry = *neighbourIt;
    Player* iEnemy = destCountry->owner();
    if (iEnemy != this)
    {
      nbe += destCountry->nbArmies();
    }
  }
  int nbi = iCountry->nbArmies();
  int nb = (nbe - nbi);
  kDebug() << "NbToEqualEnemyAdjacent of " << iCountry->name() << " is " << nb << endl;
  return nb;
}


/************************************************************************/

bool AIColsonPlayer::ComputerAttack(int destCountry, bool die, int dif)
{
  kDebug() << "AIColsonPlayer::ComputerAttack " << destCountry << endl;

  if (Attack_SrcCountry!=-1 && Attack_DestCountry!=-1
      && (RISK_GetOwnerOfCountry(Attack_DestCountry) != this)
          && (RISK_GetNumArmiesOfCountry(Attack_SrcCountry) > 1)
          && (die || (   RISK_GetNumArmiesOfCountry(Attack_SrcCountry)
                       > RISK_GetNumArmiesOfCountry(Attack_DestCountry))))
  {
    kDebug() << "    Retry attack" << endl;
    return true;
  }

  int srcCountry = -1;
  int max = RISK_GetNumArmiesOfCountry(destCountry) + dif;
  int i = 0;
  while ((i < 6) && (RISK_GetAdjCountryOfCountry(destCountry, i) != -1))
  {
    int iCountry = RISK_GetAdjCountryOfCountry(destCountry, i);
    if ( (RISK_GetOwnerOfCountry(iCountry) == this)
        && (RISK_GetNumArmiesOfCountry(iCountry) > max) )
    {
      max = RISK_GetNumArmiesOfCountry(iCountry);
      srcCountry = iCountry;
    }
    i++;
  }
  if (srcCountry == -1)
  {
    Attack_SrcCountry = -1;
    Attack_DestCountry = -1;
    return false;
  }
  kDebug() << "    srcCountry = " << srcCountry << endl;

  Attack_SrcCountry = srcCountry;
  Attack_DestCountry = destCountry;
//   AI_Attack (srcCountry, destCountry, ATTACK_DOORDIE, DICE_MAXIMUM, iMove);
  return true;
}

bool AIColsonPlayer::Fortify()
{
  kDebug() << "AIColsonPlayer::Fortify 1" << endl;

  int nb;
  const Continent* continent = GetContinentToFortify(&nb);
  
  kDebug() << "AIColsonPlayer::Fortify 2" << endl;
  if (nb > 0)
  {
    nb = rand() % nb;
    for ( int iCountry = 0;
          iCountry < m_world->getCountries().size();
          iCountry++)
    {
      if ((RISK_GetOwnerOfCountry(iCountry) == this) &&
          (RISK_GetContinentOfCountry(iCountry) == continent) &&
          GAME_IsEnemyAdjacent(iCountry))
      {
        if (nb <= 0)
        {
          AI_Place (iCountry, 1);
          return true;
        }
        nb--;
      }
    }
  }
  
  kDebug() << "AIColsonPlayer::Fortify 3" << endl;
  for ( int iCountry = 0;
        iCountry < m_world->getCountries().size();
        iCountry++)
  {
    if (    (RISK_GetOwnerOfCountry(iCountry) == this)
          &&  GAME_IsEnemyAdjacent(iCountry))
    {
      AI_Place (iCountry, 1);
      return true;
    }
  }
  
  kDebug() << "AIColsonPlayer::Fortify 4" << endl;
  for ( int iCountry = 0;
        iCountry < m_world->getCountries().size();
        iCountry++)
  {
    if (RISK_GetOwnerOfCountry(iCountry) == this)
    {
      AI_Place (iCountry, 1);
      return true;
    }
  }
  return false;
}

bool AIColsonPlayer::Place()
{
  kDebug() << "AIColsonPlayer::Place" << endl;

  /* Try to destroy a weak enemy player. 
   * NOTE this could be an error
   * if destroying this player is another player goal... */
  kDebug() << "AIColsonPlayer::Place 1. Try to destroy a enemy player" << endl;
  
  for (int iCountry = 0; iCountry < m_world->getCountries().size(); iCountry++)
  {
    if ((RISK_GetOwnerOfCountry(iCountry) == this) &&
          GAME_IsEnemyAdjacent(iCountry))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1))
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
        if (    (iEnemy != this) && isEnemyPlayer(iEnemy)
              && (getTotalArmiesOfPlayer(iEnemy)
                  < (   RISK_GetNumArmiesOfPlayer(this)
                      + RISK_GetNumArmiesOfCountry(iCountry) - 5))
              && (RISK_GetNumArmiesOfCountry(iCountry) < (2*RISK_GetNumArmiesOfCountry(destCountry))+3) )
        {
          kDebug() << " iCountry: " << RISK_GetNumArmiesOfCountry(iCountry)
            << " ; destCountry: " << RISK_GetNumArmiesOfCountry(destCountry)
            << endl;
          AI_Place (iCountry, 1);
          return true;
        }
        i++;
      }
    }
  }

  int nbCountriesWithAdjacentEnemies;
  const Continent* continent = GetContinentToFortify(&nbCountriesWithAdjacentEnemies);
  /* Try to conquier an entire continent, attack enemy */
  kDebug() << "AIColsonPlayer::Place 2. Try to conquier an entire continent, attack enemy" << endl;
  
  for (int destCountry = 0; 
       destCountry < m_world->getCountries().size();
       destCountry++)
  {
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (iEnemy != this) && isEnemyPlayer(iEnemy)
          && (RISK_GetContinentOfCountry(destCountry) == continent))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(destCountry, i) != -1))
      {
        int iCountry = RISK_GetAdjCountryOfCountry(destCountry, i);
        if ( (RISK_GetOwnerOfCountry(iCountry) == this)
            && (RISK_GetNumArmiesOfCountry(iCountry) < (1.5*RISK_GetNumArmiesOfCountry(destCountry))+3)
            )
        {
          AI_Place (iCountry, 1);
          return true;
        }
        i++;
      }
    }
  }

  /* Try to destroy a player */
  kDebug() << "AIColsonPlayer::Place 3. Try to destroy a player" << endl;
  for (int iCountry = 0;
       iCountry < m_world->getCountries().size();
       iCountry++)
  {
    if ((RISK_GetOwnerOfCountry(iCountry) == this) &&
          GAME_IsEnemyAdjacent(iCountry))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1))
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
        if (    (iEnemy != this)
              && (getTotalArmiesOfPlayer(iEnemy)
                  < (   RISK_GetNumArmiesOfPlayer(this)
                      + RISK_GetNumArmiesOfCountry(iCountry) - 5))
              && (RISK_GetNumArmiesOfCountry(iCountry) < (1.5*RISK_GetNumArmiesOfCountry(destCountry))+3)

            )
        {
          AI_Place (iCountry, 1);
          return true;
        }
        i++;
      }
    }
  }

  /* Try to conquier an entire continent */
  kDebug() << "AIColsonPlayer::Place 4. Try to conquier an entire continent" << endl;
  for (int destCountry = 0; 
       destCountry < m_world->getCountries().size();
       destCountry++)
  {
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (RISK_GetContinentOfCountry(destCountry) == continent)
          && (iEnemy != this))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(destCountry, i) != -1))
      {
        int iCountry = RISK_GetAdjCountryOfCountry(destCountry, i);
        if (RISK_GetOwnerOfCountry(iCountry) == this
            && (RISK_GetNumArmiesOfCountry(iCountry) < (1.5*RISK_GetNumArmiesOfCountry(destCountry))+3)
            )
        {
          AI_Place (iCountry, 1);
          return true;
        }
        i++;
      }
    }
  }

  /* Try to defend an entire continent */
  kDebug() << "AIColsonPlayer::Place 5. Try to defend an entire continent" << endl;
  if ( (nbCountriesWithAdjacentEnemies > 0) 
        && (RISK_GetNumArmiesOfPlayer(this) > 0) )
  {
    int min = 0;
    bool boool = false;
    int destCountry = -1;
    for (int iCountry = 0; 
         iCountry < m_world->getCountries().size();
         iCountry++)
    {
      if (    (RISK_GetOwnerOfCountry(iCountry) == this)
            && (RISK_GetContinentOfCountry(iCountry) == continent)
            &&  GAME_IsEnemyAdjacent(iCountry))
      {
        int nb = NbToAverageEnemyAdjacent(m_world->getCountries().at(iCountry));
        if (nb < min)
        {
          min = nb ;
          destCountry = iCountry;
          boool = true;
        }
      }
    }
    if (boool)
    {
      AI_Place (destCountry, 1);
      return true;
    }
  }

  kDebug() << "AIColsonPlayer::Place 6. Try to conquier an entire continent, attack enemy" << endl;
  continent = getContinentToConquier(&nbCountriesWithAdjacentEnemies);
  /* Try to conquier an entire continent, attack enemy */
  for (int iCountry = 0;
       iCountry < m_world->getCountries().size();
       iCountry++)
  {
    if ((RISK_GetOwnerOfCountry(iCountry) == this) &&
        (RISK_GetContinentOfCountry(iCountry) == continent) &&
        GAME_IsEnemyAdjacent(iCountry))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1))
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
        if (    (iEnemy != this) && isEnemyPlayer(iEnemy)
              && (RISK_GetContinentOfCountry(destCountry) == continent)
              && (RISK_GetNumArmiesOfCountry(iCountry) < (1.5*RISK_GetNumArmiesOfCountry(destCountry))+3)
            )
        {
          kDebug() << "iCountry " << RISK_GetNumArmiesOfCountry(iCountry)
                    << " ; destCountry " 
                    << RISK_GetNumArmiesOfCountry(destCountry) << endl;
          AI_Place (iCountry, 1);
          return true;
        }
        i++;
      }
    }
  }

  kDebug() << "AIColsonPlayer::Place 7. Try to conquier an entire continent" << endl;
  /* Try to conquier an entire continent */
  for (int   iCountry = 0;
       iCountry < m_world->getCountries().size();
       iCountry++)
  {
    if ((RISK_GetOwnerOfCountry(iCountry) == this) &&
        (RISK_GetContinentOfCountry(iCountry) == continent) &&
          GAME_IsEnemyAdjacent(iCountry))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1))
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
        if (    (iEnemy != this)
              && (RISK_GetContinentOfCountry(destCountry) == continent)
              && (RISK_GetNumArmiesOfCountry(iCountry) < (1.5*RISK_GetNumArmiesOfCountry(destCountry))+3)
            )
        {
          AI_Place (iCountry, 1);
          return true;
        }
        i++;
      }
    }
  }

  kDebug() << "AIColsonPlayer::Place 8. Try to defend an entire continent" << endl;
  /* Try to defend an entire continent */
  if (nbCountriesWithAdjacentEnemies > 0)
  {
    int min = 2; // a la place de 1000
    bool boool = false;
    int destCountry = -1;
    for (int iCountry = 0;
         iCountry < m_world->getCountries().size();
         iCountry++)
    {
      if (    (RISK_GetOwnerOfCountry(iCountry) == this)
          && (RISK_GetContinentOfCountry(iCountry) == continent)
          &&  GAME_IsEnemyAdjacent(iCountry))
      {
        int nb = NbToAverageEnemyAdjacent(m_world->getCountries().at(iCountry));
        if (nb < min)
        {
//           min = RISK_GetNumArmiesOfCountry(iCountry);
          min = nb;
          destCountry = iCountry;
          boool = true;
        }
      }
    }
    if (boool)
    {
      AI_Place (destCountry, 1);
      return true;
    }
  }

  kDebug() << "AIColsonPlayer::Place 9. Try to prepare an enemy attack, find a lowest defence" << endl;
  /* Try to prepare an enemy attack, find a lowest defence */
  int selected = 0;
  int myDefenceDelta = std::numeric_limits<int>::max();
  bool boool9 = false;
  for (int iCountry = 0;
       iCountry < m_world->getCountries().size(); 
       iCountry++)
  {
    if (    (RISK_GetOwnerOfCountry(iCountry) == this)
        &&  GAME_IsEnemyAdjacent(iCountry))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1))
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        int delta = RISK_GetNumArmiesOfCountry(iCountry) - RISK_GetNumArmiesOfCountry(destCountry);
        Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
        if (    (RISK_GetContinentOfCountry(destCountry) == continent)
             && (iEnemy != this) && isEnemyPlayer(iEnemy)
             && ( (   RISK_GetNumArmiesOfCountry(iCountry) < 
                      (1.5*RISK_GetNumArmiesOfCountry(destCountry))+3 ) 
                   || (RISK_GetNumArmiesOfCountry(iCountry)==1) ) 
              && (delta < myDefenceDelta ) )
        {
          selected = iCountry;
          myDefenceDelta = delta;
          boool9 = true;
        }
        i++;
      }
    }
  }
  if (boool9)
  {
    AI_Place (selected, 1);
    return true;
  }

  kDebug() << "AIColsonPlayer::Place 10.Try to prepare an enemy attack" << endl;
  /* Try to prepare an enemy attack */
  bool boool10 = false;
  myDefenceDelta = std::numeric_limits<int>::max();
  for (int iCountry = 0; 
       iCountry < m_world->getCountries().size();
       iCountry++)
  {
//     kDebug() << "    looking at iCountry " << iCountry << endl;
//     kDebug() << "    owner " << RISK_GetOwnerOfCountry(iCountry) << endl;
    if (    (RISK_GetOwnerOfCountry(iCountry) == this)
          &&  GAME_IsEnemyAdjacent(iCountry))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1))
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        int delta = RISK_GetNumArmiesOfCountry(iCountry) - RISK_GetNumArmiesOfCountry(destCountry);
        Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
//         kDebug() << "        looking at destCountry " << destCountry << endl;
//         kDebug() << "        iEnemy " << RISK_GetOwnerOfCountry(destCountry) << endl;
//         kDebug() << "        isEnemyPlayer? " << isEnemyPlayer(iEnemy) << endl;
//         kDebug() << "    delta / myDefenceDelta " << delta << " / " << myDefenceDelta << endl;
        if ((iEnemy != this) && isEnemyPlayer(iEnemy) 
              && (delta < myDefenceDelta ) )
        {
//           kDebug() << "    GOT IT " << iCountry << endl;
          myDefenceDelta = delta;
          boool10 = true;
          selected = iCountry;
        }
        i++;
      }
    }
  }
  if (boool10)
  {
    AI_Place (selected, 1);
    //AI_Place (iCountry, RISK_GetNumArmiesOfPlayer(this));
    return true;
  }

  // cannot reach this line ???
//   assert(false);

  kDebug() << "AIColsonPlayer::Place 11.Try to prepare an attack" << endl;
  /* Try to prepare an attack */
  for (int iCountry = 0;
       iCountry < m_world->getCountries().size();
       iCountry++)
  {
    if (    (RISK_GetOwnerOfCountry(iCountry) == this)
          &&  GAME_IsEnemyAdjacent(iCountry))
    {
      AI_Place (iCountry, 1);
      return true;
    }
  }

  kDebug() << "AIColsonPlayer::Place 12.Try to place" << endl;
  /* Try to place */
  for (int iCountry = 0; 
       iCountry < m_world->getCountries().size(); 
       iCountry++)
  {
    if (RISK_GetOwnerOfCountry(iCountry) == this)
    {
      AI_Place (iCountry, 1);
      return true;
    }
  }
  return false;
}

/** @return KsirK change: true if an attack have been tempted; false otherwise */
bool AIColsonPlayer::AttackEnemy()
{
  kDebug() << "AIColsonPlayer::AttackEnemy" << endl;

  int nbCountriesWithAdjacentEnemies;
  const Continent* continent = GetContinentToFortify(&nbCountriesWithAdjacentEnemies);

  if (Attack_SrcCountry!=-1 && Attack_DestCountry != -1
      && (RISK_GetOwnerOfCountry(Attack_DestCountry)!=this)
      && ComputerAttack (Attack_DestCountry, true,
                            (RISK_GetNumArmiesOfCountry(Attack_DestCountry) < 5)?1:
                                ((nbCountriesWithAdjacentEnemies > 4)?RISK_GetNumArmiesOfCountry(Attack_DestCountry):3)))
  {
    kDebug() << "Attack tempted again." << endl;
    return true;
  }


  kDebug() << "AIColsonPlayer::AttackEnemy 1 Try to conquier an entire continent, attack player of other species " << endl;
  /* Try to conquier an entire continent, attack player of other species */
  for ( int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
//     kDebug() << "destCountry=" << destCountry << " / m_world->getCountries().size()=" << m_world->getCountries().size() << endl;
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (RISK_GetContinentOfCountry(destCountry) == continent)
          && (iEnemy != this) && isEnemyPlayer(iEnemy))
    {
      if (ComputerAttack (destCountry, true,
                                (RISK_GetNumArmiesOfCountry(destCountry) < 5)?1:
                                    ((nbCountriesWithAdjacentEnemies > 4)?RISK_GetNumArmiesOfCountry(destCountry):3)))
      {
        destCountry = 0;
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 2 Try to conquier an entire continent: " << continent << endl;
  /* Try to conquier an entire continent */
  
  for (int destCountry = 0; destCountry < m_world->getCountries().size(); destCountry++)
  {
    if (     (RISK_GetContinentOfCountry(destCountry) == continent)
        && (RISK_GetOwnerOfCountry(destCountry) != this))
    {
      if (ComputerAttack (destCountry, true,
                          (RISK_GetNumArmiesOfCountry(destCountry) < 3)?1:50))
      {
        destCountry = 0;
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy should abandon ?" << endl;
  if (    !isContinentOfPlayer(continent, this)
       && (m_numTurn[this] <= 2)
       && (allPlayers.count() > m_world->getContinents().size()/2))
  {
    kDebug() << "AIColsonPlayer::AttackEnemy No attack tried" << endl;
    Attack_SrcCountry = -1;
    Attack_DestCountry = -1;
    return false;
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 3 Try to destroy a human player" << endl;
  /* Try to destroy a human player */
  continent = getContinentToConquier(&nbCountriesWithAdjacentEnemies);

  
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    if (    (RISK_GetOwnerOfCountry(destCountry) != this)
          && (!RISK_GetOwnerOfCountry(destCountry)->isAI())
          && (RISK_GetNumCountriesOfPlayer(RISK_GetOwnerOfCountry(destCountry)) == 1))
    {
        if (ComputerAttack (destCountry, true,
                                  (nbCountriesWithAdjacentEnemies > 2)?10:2))
        {
          kDebug() << "Attack tempted." << endl;
          return true;
        }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 4 Try to destroy a enemy player" << endl;
  /* Try to destroy a enemy player */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    if (   isEnemyPlayer(RISK_GetOwnerOfCountry(destCountry))
          && (RISK_GetNumCountriesOfPlayer(RISK_GetOwnerOfCountry(destCountry)) == 1))
    {
      if (ComputerAttack (destCountry, true,
                                (nbCountriesWithAdjacentEnemies > 2)?20:2))
      {
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy Try to destroy a player" << endl;
  /* Try to destroy a player */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    if (    (RISK_GetOwnerOfCountry(destCountry) != this)
          && (RISK_GetNumCountriesOfPlayer(RISK_GetOwnerOfCountry(destCountry)) == 1))
    {
      if (ComputerAttack (destCountry, true,
                                (nbCountriesWithAdjacentEnemies> 2)?20:2))
      {
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 5 Try to conquier an entire continent, attack player of other species" << endl;
  /* Try to conquier an entire continent, attack player of other species */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    if (    (RISK_GetContinentOfCountry(destCountry) == continent)
          && (RISK_GetOwnerOfCountry(destCountry) != this)
          &&  isEnemyPlayer(RISK_GetOwnerOfCountry(destCountry)))
    {
      if (ComputerAttack (destCountry, true,
                                (RISK_GetNumArmiesOfCountry(destCountry) < 5)?1:
                                    ((nbCountriesWithAdjacentEnemies > 4)?RISK_GetNumArmiesOfCountry(destCountry):3)))
      {
        destCountry = 0;
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 6 Try to conquier an entire continent" << endl;
  /* Try to conquier an entire continent */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    if (    (RISK_GetContinentOfCountry(destCountry) == continent)
          && (RISK_GetOwnerOfCountry(destCountry) != this))
    {
      if (ComputerAttack (destCountry, true,
                          (RISK_GetNumArmiesOfCountry(destCountry) < 3)?1:50))
      {
        destCountry = 0;
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 7 Try to attack a stronger human player for a card" << endl;
  /* Try to attack a stronger human player for a card */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (iEnemy != this) && isEnemyPlayer(iEnemy)
          && (!iEnemy->isAI())
          && isStrongerPlayer(RISK_GetOwnerOfCountry(destCountry)))
    {
      if (ComputerAttack (destCountry, false,
                          (RISK_GetNumArmiesOfCountry(destCountry) < 3)?1:((nbCountriesWithAdjacentEnemies > 2)?10:2)))
      {
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 8 Try to attack an human player for a card" << endl;
  /* Try to attack an human player for a card */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (iEnemy != this) && isEnemyPlayer(iEnemy)
          && (!iEnemy->isAI()))
    {
      if (ComputerAttack (destCountry, false,
                          (RISK_GetNumArmiesOfCountry(destCountry) < 3)?1:((nbCountriesWithAdjacentEnemies > 2)?10:2)))
      {
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 9 Try to attack enemy player for a card" << endl;
  /* Try to attack enemy player for a card */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (iEnemy != this) && isEnemyPlayer(iEnemy))
        if (ComputerAttack (destCountry, false,
                            (RISK_GetNumArmiesOfCountry(destCountry) < 3)?1:((nbCountriesWithAdjacentEnemies > 2)?10:2)))
    {
      {
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 10 Try to attack for a card, attack a stronger player" << endl;
  /* Try to attack for a card, attack a stronger player */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (iEnemy != this)
          && isStrongerPlayer(iEnemy))
    {
      if (ComputerAttack (destCountry, false,
                          (RISK_GetNumArmiesOfCountry(destCountry) < 2)?1:100))
      {
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy 11 Try to attack for a card" << endl;
  /* Try to attack for a card */
  for (int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
    if (RISK_GetOwnerOfCountry(destCountry) != this)
    {
      if (ComputerAttack (destCountry, false,
                          RISK_GetNumArmiesOfCountry(destCountry)
//                           (RISK_GetNumArmiesOfCountry(destCountry) < 2)?1:100
                          ))
      {
        kDebug() << "Attack tempted." << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::AttackEnemy No attack tried" << endl;
  // No attack tried
  Attack_SrcCountry = -1;
  Attack_DestCountry = -1;
  return false;
}

bool AIColsonPlayer::Attack()
{
  kDebug() << "AIColsonPlayer::Attack" << endl;
  int enemyAlive;

//   if (RISK_GetAttackModeOfPlayer (player) != ACTION_DOORDIE)
//       RISK_SetAttackModeOfPlayer (player, ACTION_DOORDIE);
//   if (RISK_GetDiceModeOfPlayer (player) != ATTACK_AUTO)
//       RISK_SetDiceModeOfPlayer (player, ATTACK_AUTO);

  enemyAlive = getNumEnemy();
  while ((enemyAlive == 0) && (m_levelEnemy >0))
    {
      m_levelEnemy--;
      enemyAlive = getNumEnemy();
    }

  return AttackEnemy();
}

void AIColsonPlayer::HowManyArmiesToMove(int *nb)
{
  if ((Attack_SrcCountry == -1) || (Attack_DestCountry == -1))
      return;

  if (!GAME_IsEnemyAdjacent(Attack_SrcCountry))
      *nb = 0;
  else if (!GAME_IsEnemyAdjacent(Attack_DestCountry))
      *nb = *nb;
  else
      *nb = *nb/2;
  Attack_SrcCountry  = -1;
  Attack_DestCountry = -1;
}

int AIColsonPlayer::FindEnemyAdjacent(int iCountry, int distance)
{
  if (m_enemyAdjacent.find(std::make_pair(iCountry,distance))!=m_enemyAdjacent.end())
  {
    return m_enemyAdjacent[std::make_pair(iCountry,distance)];
  }
  kDebug() << "AIColsonPlayer::FindEnemyAdjacent(" << iCountry << ", " << distance << ")" << endl;
  int i, min, res, dest;

  Player* iPlayer = RISK_GetOwnerOfCountry(iCountry);
  min = 100000;
  for (i=0; i!=6 && RISK_GetAdjCountryOfCountry(iCountry, i)!=-1; i++)
  {
    dest = RISK_GetAdjCountryOfCountry(iCountry, i);
    if (RISK_GetOwnerOfCountry(dest) == iPlayer)
    {
      if (distance <= 3)
      {
        res = FindEnemyAdjacent(dest, distance + 1);
        m_enemyAdjacent.insert(std::make_pair(std::make_pair(dest,distance+1),res));

        if (res < min)
            min = res;
      }
    }
    else
    {
      min = 0;
    }
  }

  return (min + 1);
}

int AIColsonPlayer::GAME_FindEnemyAdjacent(int iCountry)
{
  kDebug() << "AIColsonPlayer::GAME_FindEnemyAdjacent(" << iCountry << ")" << endl;
  m_enemyAdjacent.clear();
  int i, min, res, dest, destCountry;
  Player* iPlayer = RISK_GetOwnerOfCountry(iCountry);
  destCountry = -1;
  min = 100000;
  for (i=0; i!=6 && RISK_GetAdjCountryOfCountry(iCountry, i)!=-1; i++)
  {
    kDebug() << "  i = " << i << endl;
    dest = RISK_GetAdjCountryOfCountry(iCountry, i);
    if (RISK_GetOwnerOfCountry(dest) == iPlayer)
    {
      res = FindEnemyAdjacent(dest, 0);
      m_enemyAdjacent.insert(std::make_pair(std::make_pair(dest,0),res));
      if (res < min)
      {
        min = res;
        destCountry = dest;
      }
    }
    else
    {
      min = 0;
    }
  }

  return (destCountry);
}


bool AIColsonPlayer::Move()
{
  kDebug() << "AIColsonPlayer::Move 1" << endl;

  /* Try to move an unused max army in a frontier */
  int iCountry = -1;
  int max = 1;
  for (int i=0; i < m_world->getCountries().size();i++)
  {
    kDebug() << "AIColsonPlayer::Move 1: " << i << endl;
    if (    (RISK_GetOwnerOfCountry(i) == this)
          && (RISK_GetNumArmiesOfCountry(i) > max)
          && !GAME_IsEnemyAdjacent(i))
    {
      max = RISK_GetNumArmiesOfCountry(i);
      iCountry = i;
    }
  }
  kDebug() << "AIColsonPlayer::Move 1p iCountry=" << iCountry << endl;
  if (iCountry>=0)
  {
    int destCountry = GAME_FindEnemyAdjacent(iCountry);
    kDebug() << "  found adjacent enemy " << destCountry << endl;
    if (destCountry >= 0)
    {
      AI_Move (iCountry, destCountry,
                  RISK_GetNumArmiesOfCountry(iCountry)-1);
      kDebug() << "    Moved " << RISK_GetNumArmiesOfCountry(iCountry)-1 << endl;
      return true;
    }
  }

  kDebug() << "AIColsonPlayer::Move 2" << endl;
  /* Try to move an unused army in a country witch have a frontier */
  for (int iCountry = 0;iCountry < m_world->getCountries().size();iCountry++)
  {
    if (    (RISK_GetOwnerOfCountry(iCountry) == this)
          && (RISK_GetNumArmiesOfCountry(iCountry) > 2)
          && !GAME_IsEnemyAdjacent(iCountry))
    {
      int destCountry = GAME_FindEnemyAdjacent(iCountry);
      if (destCountry >= 0)
      {
        AI_Move (iCountry, destCountry,
                  RISK_GetNumArmiesOfCountry(iCountry)-1);
      kDebug() << "    Moved " << RISK_GetNumArmiesOfCountry(iCountry)-1 << endl;
        return true;
      }
    }
  }

  kDebug() << "AIColsonPlayer::Move 3" << endl;
  for (int iCountry = 0;(iCountry < m_world->getCountries().size()); iCountry++)
  {
    if (    (RISK_GetOwnerOfCountry(iCountry) == this)
          && (RISK_GetNumArmiesOfCountry(iCountry) > 2)
          && !GAME_IsEnemyAdjacent(iCountry))
    {
      for (int i = 0; (i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1);i++)
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        if (RISK_GetNumArmiesOfCountry(destCountry) == 1)
        {
          AI_Move (iCountry, destCountry,
                    RISK_GetNumArmiesOfCountry(iCountry)/2);
          kDebug() << "    Moved " << RISK_GetNumArmiesOfCountry(iCountry)/2 << endl;
          return true;
        }
      }
    }
  }
  kDebug() << "    Nothing Moved " << endl;

  return false;
}

// void AIColsonPlayer::ExchangeCards(const Player* player)
// {
//   int i, this, nb, typ, piCards[4], nbCards[4], piCardValues[MAX_CARDS];
//   bool fOptimal, fStronger, fSmaller;
// 
//   fStronger = isStrongerPlayer(player);
//   fSmaller = isSmallerPlayer(player);
//   nb = RISK_GetNumCardsOfPlayer(player);
//   do
//     {
//       piCards[0]=piCards[1]=piCards[2]=piCards[3]=-1;
//       nbCards[0]=nbCards[1]=nbCards[2]=nbCards[3]=0;
//       fOptimal = false;
//       for (i=0; i<nb; i++)
//         {
//           piCardValues[i] = RISK_GetCardOfPlayer(player, i);
//           /* Set the type of the card */
//           if (piCardValues[i]<m_world->getCountries().size())
//             {
//               typ = piCardValues[i] % 3;
// 	      nbCards[typ]++;
//               if (RISK_GetOwnerOfCountry(piCardValues[i]) == player)
//         	  piCards[typ] = i;
//               else if (piCards[typ] == -1)
//          	  piCards[typ] = i;
//             }
// 	  else  /* Joker */
//             {
// 	      piCards[3] = i;
// 	      nbCards[3]++;
// 	    }
//         }
//       if ((nbCards[0]>0)&&(nbCards[1]>0)&&(nbCards[2]>0))
//         {
//           AI_ExchangeCards(piCards);
//           fOptimal = true;
//         }
//       else if ((nbCards[3]>=2)&&((nbCards[0]>0)||(nbCards[1]>0)||(nbCards[2]>0)))
//         {
//           if (nbCards[2]>1)
//             {
//               piCards[1] = piCards[3];
//               j = 0;
//             }
//           else if (nbCards[1]>1)
//             {
//               piCards[0] = piCards[3];
//               j = 2;
//             }
//           else if (nbCards[0]>1)
//             {
//               piCards[2] = piCards[3];
//               j = 1;
//             }
//           else if (nbCards[2]>0)
//             {
//               piCards[1] = piCards[3];
//               j = 0;
//             }
//           else if (nbCards[1]>0)
//             {
//               piCards[0] = piCards[3];
//               j = 2;
//             }
//           else
//             {
//               piCards[2] = piCards[3];
//               j = 1;
//             }
//           i = 0;
//           while (i<nb)
//             {
//               if (piCardValues[i] >= m_world->getCountries().size())
//                 {
//                   piCards[j]=i;
//                   i = nb;
//                 }
//               else
//                   i++;
//             }
//           AI_ExchangeCards(piCards);
//           fOptimal = true;
//         }
//       else if (!fStronger && (nbCards[0]>=3))
//         {
//           j = 0;
//           for (i=0; i<nb; i++)
//               if ((piCardValues[i] % 3) == 0)
//                   piCards[j++]=i;
//           AI_ExchangeCards(piCards);
//         }
//       else if (fSmaller && (nbCards[1]>=3))
//         {
//           j = 0;
//           for (i=0; i<nb; i++)
//               if ((piCardValues[i] % 3) == 1)
//                   piCards[j++]=i;
//           AI_ExchangeCards(piCards);
//         }
//       else if (fSmaller && (nbCards[2]>=3))
//         {
//           j = 0;
//           for (i=0; i<nb; i++)
//               if ((piCardValues[i] % 3) == 2)
//                   piCards[j++]=i;
//           AI_ExchangeCards(piCards);
//         }
//       else if (nb >= 5)
//         {
//           if ((nbCards[0]>0)&&(nbCards[1]>0)&&(nbCards[3]>0))
//               piCards[2] = piCards[3];
//           else if ((nbCards[0]>0)&&(nbCards[3]>0)&&(nbCards[2]>0))
//               piCards[1] = piCards[3];
//           else if ((nbCards[3]>0)&&(nbCards[1]>0)&&(nbCards[2]>0))
//               piCards[0] = piCards[3];
//           else if (nbCards[0]>=3)
//             {
//               j = 0;
//               for (i=0; i<nb; i++)
//                   if ((piCardValues[i] % 3) == 0)
//                       piCards[j++]=i;
//             }
//           else if (nbCards[1]>=3)
//             {
//               j = 0;
//               for (i=0; i<nb; i++)
//                   if ((piCardValues[i] % 3) == 1)
//                       piCards[j++]=i;
//             }
//           else if (nbCards[2]>=3)
//             {
//               j = 0;
//               for (i=0; i<nb; i++)
//                   if ((piCardValues[i] % 3) == 2)
//                       piCards[j++]=i;
//             }
//           else
//             {
//               piCards[0] = 0;
//               piCards[1] = 1;
//               piCards[2] = 2;
//             }
//           AI_ExchangeCards(piCards);
//         }
//       nb = RISK_GetNumCardsOfPlayer(player);
//     }
//   while ((fOptimal && (nb >= 3)) || (nb >= 5));
// }

// void AIColsonPlayer::COLSON_Play(void *pData, int iCommand, void *pArgs)
// {
//   switch(iCommand)
//     {
//     case AI_INIT_ONCE:
//     {
//       /* InitClient((int)pArgs); */
//     }
//     break;
// 
//     case AI_INIT_TURN:
//     {
//       m_numTurn[m_game->currentPlayer()]++;
//     }
//     break;
// 
//     case AI_FORTIFY:
//     {
//       Fortify();
//     }
//     break;
// 
//     case AI_PLACE:
//     {
//       Place();
//     }
//     break;
// 
//     case AI_ATTACK:
//     {
//       Attack();
//     }
//     break;
// 
//     case AI_MOVE_MANUAL:
//     {
//       HowManyArmiesToMove((int *)pArgs);
//     }
//     break;
// 
//     case AI_MOVE:
//     {
//       Move();
//     }
//     break;
// 
//     case AI_EXCHANGE_CARDS:
//     {
// //         ExchangeCards(m_game->currentPlayer());
//     }
//     break;
// 
//     case AI_SERVER_MESSAGE:
//     {
//     }
//     break;
// 
//   }
// 
// //   return pData;
// }

void AIColsonPlayer::finalize()
{
  kDebug() <<"AIColsonPlayer::finalize" << endl; 

  for (unsigned int i=0; i<m_game->playerList()->count(); i++)
  {
    m_isEnemyPlayer[(Player*)m_game->playerList()->at(i)] = 0;
  }
  PlayersArray::iterator it = m_game->playerList()->begin();
  PlayersArray::iterator it_end = m_game->playerList()->end();
  for (; it != it_end; it++)
  {
    Player* player = (Player*)*it;
    m_numTurn[player] = 0;
    if (dynamic_cast<AIColsonPlayer*>(player)!=0)
        m_isEnemyPlayer[player] = 1;
    else if (player->author() == AUTHOR)
        m_isEnemyPlayer[player] = 2;
    else if (player->isAI())
        m_isEnemyPlayer[player] = 3;
    else
        m_isEnemyPlayer[player] = 4;
  }
  m_levelEnemy = 3;
  computeChoiceOfContinent();
  m_initialized = true;
  kDebug() <<"    init done." << endl; 
}

int AIColsonPlayer::AI_Place(int iCountry, int iNumArmies)
{
  kDebug() << "AIColsonPlayer::AI_Place " << iNumArmies 
            << " on country number " << iCountry << endl;
  m_placeData = new PlaceData();
  m_placeData->dest = m_game->game()->theWorld()->getCountries()[iCountry];
  m_placeData->nb = iNumArmies;
  return 0;
}

int AIColsonPlayer::AI_Move(int iSrcCountry, int iDstCountry, int iNumArmies)
{
  kDebug() << "AIColsonPlayer::AI_Move(" << iSrcCountry << ", " << iDstCountry << ", " << iNumArmies << ")" << endl;
  m_src = m_game->game()->theWorld()->getCountries()[iSrcCountry];
  m_dest = m_game->game()->theWorld()->getCountries()[iDstCountry];
  m_toMove = iNumArmies;
  kDebug() << "AIColsonPlayer::AI_Move " << iNumArmies << " armies from " 
      << m_src->name() << " to " << m_dest->name() << endl;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream <<  QString("actionMove") << m_src->centralPoint();
  aiPlayerIO()->sendInput(stream,true);
  stop();
  return 0;
}


//////////////////////////////////////////////////////////
// Tool functions reimplemented from other parts of XFrisk
//////////////////////////////////////////////////////////

Player* AIColsonPlayer::RISK_GetOwnerOfCountry(int i)
{
  return m_world->getCountries().at(i)->owner();
}

Continent* AIColsonPlayer::RISK_GetContinentOfCountry(int i)
{
  return m_world->getCountries().at(i)->continent();
}


bool AIColsonPlayer::GAME_IsEnemyAdjacent(int i)
{
  kDebug() << "AIColsonPlayer::GAME_IsEnemyAdjacent " << i << endl;
  return m_game->game()->theWorld()->getCountries()[i]->hasAdjacentEnemy();
}

int AIColsonPlayer::RISK_GetNumArmiesOfCountry(int i)
{
  return m_world->getCountries().at(i)->nbArmies();
}


/** Returns the position in the countries list of the ith adjacent country of country iCountry */
int AIColsonPlayer::RISK_GetAdjCountryOfCountry(int iCountry, unsigned int j)
{
  Country* country = m_world->getCountries().at(iCountry);
  if (j >= country->neighbours().size())
  {
    return -1;
  }
  Country* neighbour = country->neighbours()[j];
  for (unsigned int i=0; i<m_world->getCountries().size(); i++)
  {
    if (m_world->getCountries().at(i) == neighbour)
    {
      return i;
    }
  }
  return -1;
}

int AIColsonPlayer::RISK_GetNumArmiesOfPlayer(Player* player)
{
  return player->getNbAvailArmies();
}

int AIColsonPlayer::RISK_GetNumCountriesOfPlayer(const Player* player)
{
  return player->countries().size();
}


} // closing namespace GameLogic

} // closing namespace Ksirk

