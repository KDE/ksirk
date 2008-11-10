// kate: space-indent on; indent-width 2; replace-tabs on;
/** This file is part of KsirK.
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

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

#ifndef KSIRK_GAMELOGIC_AICOLSONPLAYER_H
#define KSIRK_GAMELOGIC_AICOLSONPLAYER_H

#include "aiplayer.h"
// #include "aiClient.h"
// #include "client.h"
// #include "debug.h"
// #include "types.h"
// #include "riskgame.h"
// #include "game.h"
// #include "utils.h"

#include <map>

#define MAX_CARDS       10

namespace Ksirk
{

namespace GameLogic
{
class Player;
class Continent;
class GameAutomaton;

/**
  * The AIColsonPlayer class is an AI player ported from the risk clone XFrisk, 
  * this program being programed in C. I have just made it work, adapting some
  * data structures, but it remains a lot of work to port it to a plain object
  * oriented class and also to make it work with the goals (at least).
  * @author Jean-Claude Colson (original C version)
  * @author Gaël de Chalendar (aka Kleag) (C++ version for KsirK)
  */
class AIColsonPlayer : public AIPlayer
{
public:
  /** 
    * Constructor with simple initializations
    */
  AIColsonPlayer(
          const QString & nomPlayer, unsigned int nbArmies,
          Nationality * myNation,  PlayersArray& players, ONU* world,
          GameAutomaton* game );

  /** Default destructor. */
  virtual ~AIColsonPlayer();


  virtual QString author() {return "Jean-Claude COLSON";}

  /** 
    * Returns a pair of countries where the attacker have enough armies to 
    * attack and the defender is a ennemy neighbour of the attacker.
    */
  virtual std::pair< const Country*, const Country* > chooseBelligerant();

  /**
    * Chooses the next action. In the current basic setting, chooses at random
    * between the three possibilities. For each, chooses randomly the
    * parameters.If the randomly chosen parameters end by an impossible
    * action, continue with next player.
    */
  virtual void chooseAttackMoveArmiesOrNextPlayer();

  /**
    * Chooses a country to receive a new army in dotation
    */
  virtual Country* chooseReceivingCountry();
  
  /**
    * makes all what is necessary to prepare and start the moving of armies
    */
  virtual bool moveArmiesAction();
  
  /**
    * computes the next continent to try to conquer or defend
    */
  const Continent* computeChoiceOfContinent(void);
  
  /** 
    * Called once when all players are created/loaded/joined and when the
    * game can start. Allows to initialize AIs with public data about other
    * players.
    */
  virtual void finalize();

  /**
    * chooses to continue invasion with a certain amount of armies or to stop it
    */
  virtual void chooseInvasionAction();
  
private:

  struct PlaceData
  {
    Country* dest;
    uint nb;
  };



  Player* RISK_GetOwnerOfCountry(int i);
  Continent* RISK_GetContinentOfCountry(int i);
  int RISK_GetNumArmiesOfCountry(int i);
  int RISK_GetNumArmiesOfPlayer(Player* player);
  /** 
    * Returns the position in the countries list of the ith adjacent country of
    * country iCountry 
    */
  int RISK_GetAdjCountryOfCountry(int iCountry, int i);
  int RISK_GetNumCountriesOfPlayer(const Player* player);
  bool GAME_IsEnemyAdjacent(int i);
  int FindEnemyAdjacent(int iCountry, int distance);
  int GAME_FindEnemyAdjacent(int country);



//   void COLSON_Play(void *pData, int iCommand, void *pArgs);
  
  /* The species */
  // DefineSpecies(
  // 	      COLSON_Play,
  //               "Ordinateur",
  // 	      AUTHOR,
  // 	      "0.01",
  //               "Machine violente."
  // 	      )
  
  int getTotalArmiesOfPlayer(const Player* player);
  
  bool isContinentOfMission(const Player* player, const Continent* continent);
  
  bool isEnemyPlayer(const Player* player);
  
  bool isFriendPlayer(const Player* player);
  
  int getNumEnemy();
  
  bool isStrongerPlayer(const Player* player);
  
  bool isSmallerPlayer(const Player* player);
  
  bool isContinentOfPlayer(const Continent* continent, const Player* player);
  
  const Continent* GetContinentToFortify(int *attack);
  
  const Continent* getContinentToConquier(int *attack);
  
  int NbEnemyAdjacent(Country* iCountry);
  
  int NbToEqualEnemyAdjacent(Country* iCountry);

  int NbToAverageEnemyAdjacent(Country* iCountry);

  bool ComputerAttack(int destCountry, bool die, int dif);
  
  bool Fortify();
  
  bool Place();
  
  bool AttackEnemy();
  
  bool Attack();
  
  void HowManyArmiesToMove(int *nb);
  
  bool Move();
  
  int AI_Place(int iCountry, int iNumArmies);
  int AI_Move(int iSrcCountry, int iDstCountry, int iNumArmies);

//   void ExchangeCards(const Player* player);

  std::map<const Player*, int> m_numTurn;
  std::map<const Player*, int> m_isEnemyPlayer;
  int m_levelEnemy;
  
  /** false while finalize has not been called */
  bool m_initialized;

  int Attack_SrcCountry;
  int Attack_DestCountry;

  PlaceData* m_placeData;

  int m_nbArmiesToMove;
  
  /** 
    * (country,dist)->country : used to find a nearest enemy and to avoid 
    * useless recursions
    */
  std::map<pair<int,int>,int> m_enemyAdjacent;
};

} // closing namespace GameLogic

} // closing namespace Ksirk

#endif
