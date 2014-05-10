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
  m_placeData(0)
{
  kDebug();
}

AIColsonPlayer::~AIColsonPlayer() 
{
  delete m_placeData;
}

//////////////////////////////////////////////////////////
// Virtual functions reimplemented from AIPlayer
//////////////////////////////////////////////////////////

QPair< const Country*, const Country* > AIColsonPlayer::chooseBelligerant()
{
  kDebug();
  Fortify();
  Country* src = 0;
  Country* dest = 0;
  
//   Attack_SrcCountry = -1;
//   Attack_DestCountry = -1;

  // no attack was tempted
  if (!Attack())
  {
    Attack_SrcCountry = -1;
    Attack_DestCountry = -1;
    return qMakePair<const Country*, const Country*>(static_cast<Country*>(0), static_cast<Country*>(0));
  }

  if ( (Attack_SrcCountry>=0) && (Attack_SrcCountry<m_world->getCountries().size() ) )
    src = m_world->getCountries().at(Attack_SrcCountry);
  if ( (Attack_DestCountry>=0) && (Attack_DestCountry<m_world->getCountries().size() ) )
    dest = m_world->getCountries().at(Attack_DestCountry);
//   kDebug() << "chose belligerants " << src << " and " << dest;
  return qMakePair<const Country*, const Country*>(src,dest);
}

/**
  * Chooses the next action. Attack by default and if no attack is possible,
  * try to move armies and in the last resort, choose next player
  */
void AIColsonPlayer::chooseAttackMoveArmiesOrNextPlayer()
{
  kDebug();
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
//    kDebug() <<"OUT AIColsonPlayer::chooseAttackMoveArmiesOrNextPlayer()";
}

/**
  * Chooses a country to receive a new army in dotation
  */
Country* AIColsonPlayer::chooseReceivingCountry()
{
  kDebug();
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
  kDebug();
  kDebug() << "    Attack_SrcCountry  = " << Attack_SrcCountry;
  kDebug() << "    Attack_DestCountry = " << Attack_DestCountry;
  if (Attack_SrcCountry == - 1 || Attack_DestCountry == -1)
  {
    stop();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    QPointF point;
    stream << QString("actionInvasionFinished") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_toMove = std::numeric_limits< unsigned int>::max();
    return;
  }

  if (m_toMove == std::numeric_limits< unsigned int>::max())
  {
    int nbEnemiesAdjacentToSrc = NbToEqualEnemyAdjacent(m_world->getCountries().at(Attack_SrcCountry));
    int nbEnemiesAdjacentToDest = NbToEqualEnemyAdjacent(m_world->getCountries().at(Attack_DestCountry));
    kDebug() << "    nb on src  = " << RISK_GetNumArmiesOfCountry(Attack_SrcCountry);
    kDebug() << "    nb adj to src  = " << nbEnemiesAdjacentToSrc;
    kDebug() << "    nb adj to dest = " << nbEnemiesAdjacentToDest;
    int diff = nbEnemiesAdjacentToDest - nbEnemiesAdjacentToSrc;
    kDebug() << "    diff  = " << diff;
  
  
    
    m_toMove = (diff>RISK_GetNumArmiesOfCountry(Attack_SrcCountry)-1)?RISK_GetNumArmiesOfCountry(Attack_SrcCountry)-1:(diff<0?0:diff);
    kDebug() << "    moves " << m_toMove;
  }

  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  QPoint point;
  kDebug() << "Moves *****************" << m_toMove;
  if (m_toMove >= 10)
  {
    kDebug() << "    choosing actionInvade10";
    stop(); 
    stream << QString("actionInvade10") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_toMove -= 10;
  }
  else if (m_toMove >= 5)
  { 
    kDebug() << "    choosing actionInvade5";
    stop();
    stream << QString("actionInvade5") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_toMove -= 5;
  }
  else if (m_toMove >= 1)
  { 
    kDebug() << "    choosing actionInvade1";
    stop();
    stream << QString("actionInvade1") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_toMove--;
  }
  else
  {
    kDebug() << "    choosing actionInvasionFinished";
    stop();
    stream << QString("actionInvasionFinished") << point;
    aiPlayerIO()->sendInput(stream,true);
    m_toMove = std::numeric_limits< unsigned int>::max();
  }
}

/**
  * makes all what is necessary to prepare and start the moving of armies
  */
bool AIColsonPlayer::moveArmiesAction() 
{
  kDebug();
  bool res = Move();
  kDebug() << "Move got " << res;
  return res;
}

//////////////////////////////////////////////////////////
// XFrisk AIColson functions
//////////////////////////////////////////////////////////
int AIColsonPlayer::getTotalArmiesOfPlayer(const Player* player)
{
  int nb = 0;
  for (int i=0; i<m_world->getCountries().size(); i++)
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
  return (player->goal().continents().contains(continent->name()));
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
  kDebug();
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
//   kDebug();
  std::map< const KPlayer*, std::map <const Continent*, int > > piCount;
  std::map< const KPlayer*, std::map <const Continent*, bool > > maxContinent;
  std::map< const KPlayer*, std::map <const Continent*, bool > > posContinent;
  std::map<const KPlayer*, const Continent*> m_piContinent;

  QList<Continent*>::iterator continentsIt = (m_world->getContinents().begin());
  QList<Continent*>::iterator continentsIt_end = (m_world->getContinents().end());

  PlayersArray::iterator it = m_game->playerList()->begin();
  PlayersArray::iterator it_end = m_game->playerList()->end();
  for (; it != it_end; it++)
  {
    
    continentsIt = m_world->getContinents().begin();
    continentsIt_end = m_world->getContinents().end();
    for (; continentsIt != continentsIt_end; continentsIt++)
    {
      Continent *continent = *continentsIt;
//       kDebug() << "    " << continent;
      if (continent == 0) continue;
//       kDebug() << "    " << continent->name();
      piCount[((Player*)(*it))][continent] = 0;
      maxContinent[((Player*)(*it))][continent] = false;
      posContinent[((Player*)(*it))][continent] = false;
    }
  }

  for (int i=0; i<m_world->getCountries().size(); i++)
  {
/*  QList<Country*>::iterator countriesIt(m_world->getCountries().begin());
  QList<Country*>::iterator countriesIt_end(m_world->getCountries().end());
  for (; countriesIt != countriesIt_end; countriesIt++)
  {*/
    Country* country = m_world->getCountries().at(i);
//     kDebug() << "country " << country << ;
//     kDebug() << "country " << country->name().toUtf8().data();
    if ( country->owner() != 0 && country->continent() != 0)
    {
      piCount[country->owner()][country->continent()]++;
//       kDebug() << "piCount[" << country->owner()->name().toUtf8().data() << "][" << country->continent()->name().toUtf8().data() << "] ";
//       kDebug() << " is now " << piCount[country->owner()][country->continent()];
    }
    else 
    {
//       kDebug() << "owner or continent of country " << country->name().toUtf8().data() << " is null...";
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
            kDebug() << "pt2";
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
          QList<Continent*>::iterator jit(m_world->getContinents().begin());
          QList<Continent*>::iterator jit_end(m_world->getContinents().end());
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
  QList<Continent*>::iterator contIt(m_world->getContinents().begin());
  QList<Continent*>::iterator contIt_end(m_world->getContinents().end());
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
    QList<Continent*>::iterator contIt(m_world->getContinents().begin());
    QList<Continent*>::iterator contIt_end(m_world->getContinents().end());
#ifdef __GNUC__
#warning continent can not be 0 in this code path - everything in this for loop is dead code
#endif
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
    QList<Continent*>::iterator contIt(m_world->getContinents().begin());
    QList<Continent*>::iterator contIt_end(m_world->getContinents().end());
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
    kDebug() << "pt5";
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
    QList<Continent*>::iterator contIt(m_world->getContinents().begin());
    QList<Continent*>::iterator contIt_end(m_world->getContinents().end());
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
    kDebug() << "pt6";
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
    QList<Continent*>::iterator contIt(m_world->getContinents().begin());
    QList<Continent*>::iterator contIt_end(m_world->getContinents().end());
    for (;contIt!=contIt_end;contIt++)
    {
      const Continent* cont = *contIt;
      if (cont != continent)
      {
        maxContinent[this][cont] = false;
      }
    }
    kDebug() << "pt7";
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
    QList<Continent*>::iterator contIt(m_world->getContinents().begin());
    QList<Continent*>::iterator contIt_end(m_world->getContinents().end());
    for (;contIt!=contIt_end;contIt++)
    {
      const Continent* cont = *contIt;
      if (cont!=continent)
      {
        maxContinent[this][cont] = false;
      }
    }
    kDebug() << "pt8";
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
//     << " is " << continent;
  return continent;
}

/**
  * Gets the continent already selected for this player
  * @param attack This is the number of countries of the selected continent
  * that have at least one adjacent enemy
  */
const Continent* AIColsonPlayer::GetContinentToFortify(int *attack)
{
  kDebug();
  const Continent* continent = computeChoiceOfContinent();
  kDebug() << "  computeChoiceOfContinent got " << continent;
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
  kDebug() << "found " << continent;
  return(continent);
}

/** 
  * @param attack Number of countries with adjacent ennemies in the selected
  * continent
  */
const Continent* AIColsonPlayer::getContinentToConquier(int *attack)
{
  kDebug();
  std::map<const Continent*, int> piCount;
  std::map<const Continent*, int> piOppo;
  std::map<const Continent*, int> piAttac;

  /* Init. */
  QList<Continent*>::iterator contIt(m_world->getContinents().begin());
  QList<Continent*>::iterator contIt_end(m_world->getContinents().end());
  for (;contIt!=contIt_end;contIt++)
  {
    const Continent* cont = *contIt;
    piCount[cont] = 0;
    piOppo[cont] = 0;
    piAttac[cont] = 0;
  }

  /* Count up how many countries the player has in each of the continents */
  QList<Country*>::iterator countriesIt(m_world->getCountries().begin());
  QList<Country*>::iterator countriesIt_end(m_world->getCountries().end());
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
  contIt = m_world->getContinents().begin();
  contIt_end = m_world->getContinents().end();
  for (; contIt != contIt_end; contIt++)
  {
    const Continent* cont = *contIt;
    kDebug() << "    " << cont;
    if (cont != 0)
      kDebug() << "    " << cont->name();
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
      }
    }
  }

  *attack = piAttac[continent];
  return(continent);
}

int AIColsonPlayer::NbEnemyAdjacent(Country* iCountry)
{
  std::map<Player*, unsigned int> NumEnemy;
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
  QList<Country*>::const_iterator it, it_end;
  it = iCountry->neighbours().constBegin();
  it_end = iCountry->neighbours().constEnd();
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
  QList<Country*>::const_iterator neighbourIt, neighbourIt_end;
  neighbourIt = iCountry->neighbours().constBegin();
  neighbourIt_end = iCountry->neighbours().constEnd();
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
  kDebug() << "NbToAverageEnemyAdjacent of " << iCountry->name() << " is " << nbe;
  return nbe;
}


int AIColsonPlayer::NbToEqualEnemyAdjacent(Country* iCountry)
{
  int nbe = 0;
  QList<Country*>::const_iterator neighbourIt, neighbourIt_end;
  neighbourIt = iCountry->neighbours().constBegin();
  neighbourIt_end = iCountry->neighbours().constEnd();
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
  kDebug() << "NbToEqualEnemyAdjacent of " << iCountry->name() << " is " << nb;
  return nb;
}


/************************************************************************/

bool AIColsonPlayer::ComputerAttack(int destCountry, bool die, int dif)
{
  kDebug() << destCountry;

  if (Attack_SrcCountry!=-1 && Attack_DestCountry!=-1
    && (RISK_GetOwnerOfCountry(Attack_SrcCountry) == this)
    && (RISK_GetOwnerOfCountry(Attack_DestCountry) != this)
    && (RISK_GetNumArmiesOfCountry(Attack_SrcCountry) > 1)
          && (die || (   RISK_GetNumArmiesOfCountry(Attack_SrcCountry)
                       > RISK_GetNumArmiesOfCountry(Attack_DestCountry))))
  {
    kDebug() << "    Retry attack";
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
  kDebug() << "    srcCountry = " << srcCountry;

  Attack_SrcCountry = srcCountry;
  Attack_DestCountry = destCountry;
//   AI_Attack (srcCountry, destCountry, ATTACK_DOORDIE, DICE_MAXIMUM, iMove);
  return true;
}

bool AIColsonPlayer::Fortify()
{
  kDebug() << "1";

  int nb;
  const Continent* continent = GetContinentToFortify(&nb);
  
  kDebug() << "2";
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
  
  kDebug() << "3";
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
  
  kDebug() << "4";
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
  kDebug();

  /* Try to destroy a weak enemy player. 
   * NOTE this could be an error
   * if destroying this player is another player goal... */
  kDebug() << "1. Try to destroy a enemy player";
  
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
            << " ; destCountry: " << RISK_GetNumArmiesOfCountry(destCountry);
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
  kDebug() << "2. Try to conquier an entire continent, attack enemy";
  
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
  kDebug() << "3. Try to destroy a player";
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
  kDebug() << "4. Try to conquier an entire continent";
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
  kDebug() << "5. Try to defend an entire continent";
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

  kDebug() << "6. Try to conquier an entire continent, attack enemy";
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
                    << RISK_GetNumArmiesOfCountry(destCountry);
          AI_Place (iCountry, 1);
          return true;
        }
        i++;
      }
    }
  }

  kDebug() << "7. Try to conquier an entire continent";
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

  kDebug() << "8. Try to defend an entire continent";
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

  kDebug() << "9. Try to prepare an enemy attack, find a lowest defence";
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

  kDebug() << "10. Try to prepare an enemy attack";
  /* Try to prepare an enemy attack */
  bool boool10 = false;
  myDefenceDelta = std::numeric_limits<int>::max();
  for (int iCountry = 0; 
       iCountry < m_world->getCountries().size();
       iCountry++)
  {
//     kDebug() << "    looking at iCountry " << iCountry;
//     kDebug() << "    owner " << RISK_GetOwnerOfCountry(iCountry);
    if (    (RISK_GetOwnerOfCountry(iCountry) == this)
          &&  GAME_IsEnemyAdjacent(iCountry))
    {
      int i = 0;
      while ((i < 6) && (RISK_GetAdjCountryOfCountry(iCountry, i) != -1))
      {
        int destCountry = RISK_GetAdjCountryOfCountry(iCountry, i);
        int delta = RISK_GetNumArmiesOfCountry(iCountry) - RISK_GetNumArmiesOfCountry(destCountry);
        Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
//         kDebug() << "        looking at destCountry " << destCountry;
//         kDebug() << "        iEnemy " << RISK_GetOwnerOfCountry(destCountry);
//         kDebug() << "        isEnemyPlayer? " << isEnemyPlayer(iEnemy);
//         kDebug() << "    delta / myDefenceDelta " << delta << " / " << myDefenceDelta;
        if ((iEnemy != this) && isEnemyPlayer(iEnemy) 
              && (delta < myDefenceDelta ) )
        {
//           kDebug() << "    GOT IT " << iCountry;
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

  kDebug() << "11. Try to prepare an attack";
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

  kDebug() << "12. Try to place";
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
  kDebug();

  int nbCountriesWithAdjacentEnemies;
  const Continent* continent = GetContinentToFortify(&nbCountriesWithAdjacentEnemies);

  if (Attack_SrcCountry!=-1 && Attack_DestCountry != -1
    && (RISK_GetOwnerOfCountry(Attack_SrcCountry)==this)
    && (RISK_GetOwnerOfCountry(Attack_DestCountry)!=this)
    && ComputerAttack (Attack_DestCountry, true,
                            (RISK_GetNumArmiesOfCountry(Attack_DestCountry) < 5)?1:
                                ((nbCountriesWithAdjacentEnemies > 4)?RISK_GetNumArmiesOfCountry(Attack_DestCountry):3)))
  {
    kDebug() << "Attack tempted again.";
    return true;
  }


  kDebug() << "1 Try to conquier an entire continent, attack player of other species ";
  /* Try to conquier an entire continent, attack player of other species */
  for ( int destCountry = 0;
        destCountry < m_world->getCountries().size();
        destCountry++)
  {
//     kDebug() << "destCountry=" << destCountry << " / m_world->getCountries().size()=" << m_world->getCountries().size();
    Player* iEnemy = RISK_GetOwnerOfCountry(destCountry);
    if (    (RISK_GetContinentOfCountry(destCountry) == continent)
          && (iEnemy != this) && isEnemyPlayer(iEnemy))
    {
      if (ComputerAttack (destCountry, true,
                                (RISK_GetNumArmiesOfCountry(destCountry) < 5)?1:
                                    ((nbCountriesWithAdjacentEnemies > 4)?RISK_GetNumArmiesOfCountry(destCountry):3)))
      {
        destCountry = 0;
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "2 Try to conquier an entire continent: " << continent;
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "should abandon ?";
  if (    !isContinentOfPlayer(continent, this)
       && (m_numTurn[this] <= 2)
       && (allPlayers.count() > m_world->getContinents().size()/2))
  {
    kDebug() << "No attack tried";
    Attack_SrcCountry = -1;
    Attack_DestCountry = -1;
    return false;
  }

  kDebug() << "3 Try to destroy a human player";
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
          kDebug() << "Attack tempted.";
          return true;
        }
    }
  }

  kDebug() << "4 Try to destroy a enemy player";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "Try to destroy a player";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "5 Try to conquier an entire continent, attack player of other species";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "6 Try to conquier an entire continent";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "7 Try to attack a stronger human player for a card";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "8 Try to attack an human player for a card";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "9 Try to attack enemy player for a card";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "10 Try to attack for a card, attack a stronger player";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "11 Try to attack for a card";
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
        kDebug() << "Attack tempted.";
        return true;
      }
    }
  }

  kDebug() << "No attack tried";
  // No attack tried
  Attack_SrcCountry = -1;
  Attack_DestCountry = -1;
  return false;
}

bool AIColsonPlayer::Attack()
{
  kDebug();
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
  kDebug() << iCountry << distance;
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
  kDebug() << iCountry;
  m_enemyAdjacent.clear();
  int i, min, res, dest, destCountry;
  Player* iPlayer = RISK_GetOwnerOfCountry(iCountry);
  destCountry = -1;
  min = 100000;
  for (i=0; i!=6 && RISK_GetAdjCountryOfCountry(iCountry, i)!=-1; i++)
  {
    kDebug() << "  i = " << i;
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
  kDebug();

  /* Try to move an unused max army in a frontier */
  int iCountry = -1;
  int max = 1;
  for (int i=0; i < m_world->getCountries().size();i++)
  {
    kDebug() << "1: " << i;
    if (    (RISK_GetOwnerOfCountry(i) == this)
          && (RISK_GetNumArmiesOfCountry(i) > max)
          && !GAME_IsEnemyAdjacent(i))
    {
      max = RISK_GetNumArmiesOfCountry(i);
      kDebug() << "country " << i << ", num armies:" << max;
      iCountry = i;
    }
  }
  kDebug() << "1p iCountry=" << iCountry << "; max=" << max;
  if (iCountry>=0)
  {
    int destCountry = GAME_FindEnemyAdjacent(iCountry);
    kDebug() << "  found adjacent enemy " << destCountry;
    if (destCountry >= 0)
    {
      AI_Move (iCountry, destCountry,
                  RISK_GetNumArmiesOfCountry(iCountry)-1);
      kDebug() << "    Moved " << RISK_GetNumArmiesOfCountry(iCountry)-1;
      return true;
    }
  }

  kDebug() << "2";
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
      kDebug() << "    Moved " << RISK_GetNumArmiesOfCountry(iCountry)-1;
        return true;
      }
    }
  }

  kDebug() << "3";
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
          kDebug() << "    Moved " << RISK_GetNumArmiesOfCountry(iCountry)/2;
          return true;
        }
      }
    }
  }
  kDebug() << "    Nothing Moved ";

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
  kDebug(); 

  for (int i=0; i<m_game->playerList()->count(); i++)
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
  kDebug() <<"    init done.";
}

int AIColsonPlayer::AI_Place(int iCountry, int iNumArmies)
{
  kDebug() << iNumArmies 
            << " on country number " << iCountry;
  if (m_placeData != 0)
  {
    delete m_placeData;
  }
  m_placeData = new PlaceData();
  m_placeData->dest = m_game->game()->theWorld()->getCountries()[iCountry];
  m_placeData->nb = iNumArmies;
  return 0;
}

int AIColsonPlayer::AI_Move(int iSrcCountry, int iDstCountry, int iNumArmies)
{
  kDebug() << iSrcCountry << ", " << iDstCountry << ", " << iNumArmies;
  m_src = m_game->game()->theWorld()->getCountries()[iSrcCountry];
  m_dest = m_game->game()->theWorld()->getCountries()[iDstCountry];
  m_toMove = iNumArmies;
  kDebug() << iNumArmies << " armies from "
      << m_src->name() << " to " << m_dest->name();
  
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << QString("actionLButtonDown") << m_src->centralPoint();
  aiPlayerIO()->sendInput(stream,true);

  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  stream2 << QString("actionLButtonUp") << m_dest->centralPoint();
  aiPlayerIO()->sendInput(stream2,true);

  Attack_SrcCountry = m_game->game()->theWorld()->getCountries().indexOf(const_cast<Country*>(m_src));
  Attack_DestCountry = m_game->game()->theWorld()->getCountries().indexOf(const_cast<Country*>(m_dest));

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
  kDebug() << i;
  return m_game->game()->theWorld()->getCountries()[i]->hasAdjacentEnemy();
}

int AIColsonPlayer::RISK_GetNumArmiesOfCountry(int i)
{
  return m_world->getCountries().at(i)->nbArmies();
}


/** Returns the position in the countries list of the ith adjacent country of country iCountry */
int AIColsonPlayer::RISK_GetAdjCountryOfCountry(int iCountry, int j)
{
  Country* country = m_world->getCountries().at(iCountry);
  if (j >= country->neighbours().size())
  {
    return -1;
  }
  Country* neighbour = country->neighbours()[j];
  for (int i=0; i<m_world->getCountries().size(); i++)
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

