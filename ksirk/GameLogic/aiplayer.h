/* This file is part of KsirK.
   Copyright (C) 2002-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "GameLogic/gameautomaton.h"
#include "GameLogic/player.h"
#include "GameLogic/country.h"

#include <qthread.h>

#include <utility>


namespace Ksirk
{

namespace GameLogic
{

class Nationality;
class ONU;
class AIPlayerIO;
class GameAutomaton;

/**
  * This class represents a computer player. It holds all strategic routines.
  * @author Gael de Chalendar (aka Kleag)
  */
class AIPlayer : public Player
{
Q_OBJECT

public: 
  /** 
    * Constructor with simple initializations
    */
  AIPlayer(
          const QString & nomPlayer, unsigned int nbArmies,
          Nationality * myNation,  PlayersArray& players, ONU* world,
          GameAutomaton* game );

  /** Default destructor. */
  virtual ~AIPlayer();

  /**
    * The idendification of the player. Overwrite this in
    * classes inherting KPlayer to run time identify them.
    *
    * @return 2 for this class.
  */
  virtual int rtti() const {return 2;}
  
  /**
    * Returns true (an AIPlayer is an AI)
    */
  virtual bool isAI() const;

    /** set stopMe to true in order for the run method to return */
  void stop();
    
  /**
    * Saves this AI player as XML. Used in game saving.
    * @param xmlStream The stream on which to write the XML
    */
  virtual void saveXml(std::ostream& xmlStream);

  bool isRunning () const {return m_thread.isRunning();}

public Q_SLOTS:
  void start ( QThread::Priority priority = QThread::InheritPriority ) {m_thread.start(priority);}

protected:
  /** 
    * This function is called whenever the player should choose an action 
    * (attack, defense, etc.). It has the responsibility to choose the correct
    * action depending on the state of the game.
    */
  virtual void actionChoice(GameLogic::GameAutomaton::GameState state);

  /** Returns a pair of countries where the attacker have enough armies to 
    * attack and the defender is a ennemy neighbour of the attacker */
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
    * chooses to continue invasion with a certain amount of armies or to stop it
    */
  virtual void chooseInvasionAction();
  
  /**
    * make all what is necessary to prepare and launch an attack
    * @return true if was able to prepare an attack ; false otherwise
    */
  bool attackAction();
  
  /**
    * makes all what is necessary to prepare and start the moving of armies
    */
  virtual bool moveArmiesAction();
  
  /**
    * makes what is necessary to finish my turn
    */
  void nextPlayerAction();
  
protected: // Private attributes
  class MyThread: public QThread
  {
  protected:
    virtual void run ();
    
  public:
    MyThread(AIPlayer& p) : me(p) {}
    void setStopMe ( bool value ) { stopMe = value; }
  private:
    /** indicates to the thread if the run method should return */
    bool stopMe;
    AIPlayer& me;
  };

  AIPlayerIO* aiPlayerIO();
  
/**
    * Pointer to the players. Information about them is necessary decide of
    * a strategy
    */
  PlayersArray& allPlayers;

  /**
    * Pointer to the World to consult it in order to decide the actions
    */
  ONU* m_world;

  /**
    * a pointer to the game. Necessary to be able to access the number of
    * attackers, etc. This solution is not very pretty... but an important
    * architectural change should be done to avoid it (@todo).
    */
  GameAutomaton* m_game;

 /**
   * a pointer to the game attribute defenseAuto
   *
   */
 // GameAutomaton* m_defenseAuto;

  /** pointers to the source and target country of an attack */
  const Country* m_src;
  const Country* m_dest;

  /** number of armies to move during an invasion or an end of turn moving */
  unsigned int m_toMove;
    
  bool m_hasVoted;
  bool m_actionWaitingStart;

  MyThread m_thread;

private: // Private methods
  /**
    * chooses whether to defend with one or two armies. Always chooses the maximum possible
    */
  void chooseDefenseAction();
  
  /**
    * Takes the decision to recycle armies or not
    */
  void chooseWetherToRecycle();
  
  /**
    * chooses a country where to place a new army
    */
  void placeArmiesAction();
  
  /** Makes the choice of nb armies to move during an invasion or an end of turn moving */
  void chooseNbToMoveOrStop();
  
  /** Starts the timer of this player that will make it "think" all 200 ms. */
  void run();

  void requestAck();
};

} // closing namespace GameLogic
} // closing namespace Ksirk
#endif

