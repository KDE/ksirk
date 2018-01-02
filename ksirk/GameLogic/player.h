/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gaël de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
*   02110-1301, USA
*/

/*  begin                : Wed Jul 18 2001 */
#define KDE_NO_COMPAT

#ifndef PLAYER_H
#define PLAYER_H

#include "KsirkGlobalDefinitions.h"
#include "GameLogic/distributiondata.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/goal.h"
#include "nationality.h"
#include "Sprites/animsprite.h"

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kplayer.h>
#include <libkdegamesprivate/kgame/kgame.h>
#include <libkdegamesprivate/kgame/kgameproperty.h>
#include <libkdegamesprivate/kgame/kgamepropertyhandler.h>

#include <QString>
#include <QMutex>

namespace Ksirk
{

// class Goal;

namespace GameLogic
{

class Nationality;

/**
  * The class Player represents a player and its associated data
  */
class Player : public KPlayer
{
  Q_OBJECT
public:

  /**
    * Constructor with simple initializations
    */
  Player(GameAutomaton* automaton, const QString &nomPlayer, unsigned int nbArmies, Nationality* myNation);

  virtual ~Player() {}

  /**
    * The idendification of the player. Overwrite this in
    * classes inherting KPlayer to run time identify them.
    *
    * @return 0 for default KPlayer.
  */
  virtual int rtti() const {return 1;}
  
  /**
    * comparison on the players' names
    */
  virtual bool operator==(const Player& player) const;

  //@{
  /**
    * Accessors to the variables
    */
  void setNbAvailArmies(unsigned int nb, bool transmit = true);
  unsigned int getNbAvailArmies();
  void setNbAttack(unsigned int nb);
  unsigned int getNbAttack();
  void setNbDefense(unsigned int nb);
  unsigned int getNbDefense();
  void setNbCountries(unsigned int nb);
  unsigned int getNbCountries() const;
  //@}

  //@{
  /**
    * Add/Remove nb armies to the number of available armies (defaults to 1)
    */
  void incrNbAvailArmies(unsigned int nb=1);
  void decrNbAvailArmies(unsigned int nb=1);
  //@}
  void putArmiesInto(int nb, int country);
  void removeArmiesFrom(int nb, int country);
  bool canRemoveArmiesFrom(int nb, int country);
  
  //@{
  /**
    * Add/Remove nb countries to the player (defaults to 1)
    */
  void incrNbCountries(unsigned int nb=1);
  void decrNbCountries(unsigned int nb=1);
  //@}

  /** Returns the flag associated to the nationality of the player */
  const AnimSprite* getFlag() const;

  /** Returns the file name of the flag sprite associated to the nationality of 
    * the player */
  const QString& flagFileName() const;

  /**
    * This function is called whenever the player should choose an action
    * (attack, defence, etc.). If the player is human, this method do nothing
    * and so is empty. Its inherited version, in AIPlayer will have an activity
    */
  virtual void actionChoice(GameLogic::GameAutomaton::GameState /*state*/) {}

  /**
    * Returns false (a Player is not an AI)
    */
  virtual bool isAI() const;

  /**
    * Saves this player as XML. Used in game saving.
    * @param xmlStream The stream on which to write the XML
    */
  virtual void saveXml(QTextStream& xmlStream);
  
  //@{
  /** Functions used to write and read data to and from a stream for network 
    * transmission */
  virtual bool   load (QDataStream &stream);
  virtual bool   save (QDataStream &stream);
  //@}

  //@{
  /** Accessors for this player's nationality */
  Nationality* getNation();
  void setNation(const QString& nationName);
  //@}

  //@{
  /** Accessors for this player's password */
  inline void setPassword(const QString& password) {m_password = password;}
  inline const QString& getPassword() const {return m_password.value();}
  //@}
  
  //@{
  /** Accessors for this player's goal */
  inline const Goal& goal() const {return m_goal;}
  inline Goal& goal() {return m_goal;}
  void goal(const Goal& goal);
  //@}

  /**
    * Checks if this player's goal is reached or not.
    * @return true if the goal of this player is fulfilled, false otherwise
    */
  bool checkGoal();
  
  /**
    * @return The list of the countries owned by this player
    */
  QList<Country*> countries() const;
  
  #define AUTHOR "Kleag"
  /** Method added during the porting of the AIColsonPlayer AI from XFrisk */
  virtual QString author() {return AUTHOR;}

  /** 
    * Called once when all players are created/loaded/joined and when the
    * game can start. Allows to initialize AIs with public data about other
    * players.
    */
  virtual void finalize() {}

  /** 
    * Called by the game automaton to acknowledge the reception of a message
    * by the master
    * @param ack an id to acknowledge if it is the waited one
    * @return true if the ack received was the waited one; false otherwise.
    */
  bool acknowledge(const QString& ack);

  void reset();

protected:
  /** 
    * Saving of private data 
    * @param xmlStream The stream on which to write the XML
    */
  void innerSaveXml(QTextStream& xmlStream);
  
  GameAutomaton* m_automaton;
  /**
    * Number of armies used for an attack
    * (  0 <> 3, < nbArmies of the country )
    */
  KGamePropertyUInt m_nbAttack;

  /**
    * Number of countries owned by the player
    */
  KGamePropertyUInt m_nbCountries;

  /**
    * Number of armies the player can distribute
    */
//   KGamePropertyUInt m_nbAvailArmies;
//   unsigned int m_nbAvailArmies;

  /**
    * Number of armies used for defense
    * (  0 <> 2, <= nbArmies used for the attack, < nbArmies of the country )
    */
  KGamePropertyUInt m_nbDefense;

  /**
    * The nation chosen by the player
    */
  Nationality *m_nation;

  /**
    * The encrypted password of the player
    */
  KGamePropertyQString m_password;

  /**
    * The goal to be reached by this player
    */
  Goal m_goal;

  /**
    * Allows to initialize the nation name of the player even if the nation 
    * object is not already initialized
    */
  QString m_delayedInitNationName;

  QString m_waitedAck;

  QMutex m_waitedAckMutex;
  
private:
  void setFlag();
  
  AnimSprite* m_flag;

  DistributionData m_distributionData;
};

typedef KGame::KGamePlayerList PlayersArray;

/**
  * A structure to store data concerning a player. Used during loading when the
  * actual player is waited from the network.
  */
struct PlayerMatrix
{
  PlayerMatrix(GameLogic::GameAutomaton* automaton): goal(automaton){}

  QString name;
  
  unsigned int nbAttack;
  
  unsigned int nbCountries;
  
  unsigned int nbAvailArmies;
  
  unsigned int nbDefense;
  
  QString nation;
  
  QString password;
  
  QList<QString> countries;
  
  GameLogic::GameAutomaton::GameState state;
  
  bool isAI;
  
  Goal goal;
};

QDataStream& operator<<(QDataStream& stream, PlayerMatrix& p);

QDataStream& operator>>(QDataStream& stream, PlayerMatrix& p);

} // closing namespace GameLogic

} // closing namespace Ksirk

#endif // PLAYER_H
