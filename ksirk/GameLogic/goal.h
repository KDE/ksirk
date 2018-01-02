/* This file is part of KsirK.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef KSIRK_GAMELOGICGOAL_H
#define KSIRK_GAMELOGICGOAL_H

// #include <set>
#include <QTextStream>

#include <qdatastream.h>

namespace Ksirk {

namespace GameLogic {

class GameAutomaton;
class Player;

/**
  * This is a representation of a goal to be reached by a player.
  *
  * A goal can be a combination of a number of countries to conquier, some 
  * continents to conquier entirely or some players to eliminate.
  * @author Gael de Chalendar
  */
class Goal
{
public:
  /**
    * The different kind of goals
    */
  enum GoalType {
    NoGoal,     /**< The goal is to conquer all the world. */
    GoalPlayer, /**< The goal is to eliminate one player. */
    Countries,  /**< The goal is to conquier a given number of countries. */
    Continents	/**< The goal is to conquier a few continents. */
  };

  
  /**
    * The different kind of goal displaying
    */
  enum DisplayType {
    GoalDesc = 1,   /**< The description of the goal is displayed. */
    GoalAdvance = 2 /**< The advance step. */
  };
  
  Goal(GameAutomaton* automaton);
  
  /** Copy constructor */
  Goal(const Goal& goal);
  
  /** Default destructor */
  ~Goal();

  //@{
  /** Accessors for the goal type */
  inline GoalType type() const {return m_type;}
  inline GoalType type() {return m_type;}
  inline void type(GoalType type) {m_type = type;}
  //@}

  //@{
  /** Accessors for the goal description */
  inline const QString& description() const {return m_description;}
  inline QString& description() {return m_description;}
  inline void description(const QString& desc) {m_description = desc;}
  //@}

  //@{
  /** Accessors for the number of countries to conquier to reach this goal */
  inline unsigned int nbCountries() const {return m_nbCountries;}
  inline unsigned int nbCountries() {return m_nbCountries;}
  inline void nbCountries(unsigned int nb) {m_nbCountries = nb;}
  //@}

  //@{
  /** Accessors for the minimal number of armies to put on each country to 
    * reach this goal */
  inline unsigned int nbArmiesByCountry() const {return m_nbArmiesByCountry;}
  inline unsigned int nbArmiesByCountry() {return m_nbArmiesByCountry;}
  inline void nbArmiesByCountry(unsigned int nb) {m_nbArmiesByCountry = nb;}
  //@}

  //@{
  /** Accessors for the list of continents to conquier to reach this goal */
  inline QList<QString>& continents() {return m_continents;}
  inline const QList<QString>& continents() const {return m_continents;}
  //@}

  //@{
  /** Accessors for the list of players to eliminate to reach this goal */
  inline QList<QString>& players() {return m_players;}
  inline const QList<QString>& players() const {return m_players;}
  //@}

  //@{
  /** Accessors for the player concerned by this goal */
  inline const Player* player() const {return m_player;}
  inline Player* player() {return m_player;}
  inline void player(Player*  p) {m_player = p;}
  //@}

  /** 
    * Test if this goal is reached for the given player
    * @param player The player to test this goal for
    * @return true if this goal is reached for the given player ; false otherwise.
    */
  bool checkFor(const Player* player) const;
  
  /** 
    * Test if countries conditions of this goal are reached for the given player
    * @param player The player to test this goal countries conditions for
    * @return true if the countries conditions of this goal are reached for the 
    * given player ; false otherwise.
    */
  bool checkCountriesFor(const Player* player) const;

  /** 
    * Test if continents conditions of this goal are reached for the given player
    * @param player The player to test this goal continents conditions for
    * @return true if the continents conditions of this goal are reached for the 
    * given player ; false otherwise.
    */
  bool checkContinentsFor(const Player* player) const;

  /**
    * Displays this goal in a message dialog
    * @param displayType the manner to display this goal: final goal or advance.
    */
  void show(int displayType = GoalDesc);
    
  
  /**
    * Saves a XML representation of the goal for game saving purpose
    * @param xmlStream The stream to write on
    */
  void saveXml(QTextStream& xmlStream) const;
  
  /**
    * Builds this goal's description.
    * @param displayType the manner to display this goal: final goal (default) or advance.
    * @return a string containing this goal's description.
    */
  QString message(int displayType = GoalDesc) const;

  GameAutomaton* m_automaton;
  private:
    /** Default constructor */
  Goal();

  GoalType m_type;
  QString m_description;
  unsigned int m_nbCountries;
  unsigned int m_nbArmiesByCountry;
  QList<QString> m_continents;
  QList<QString> m_players;
  Player* m_player;
};

QDataStream& operator<<(QDataStream& stream, const Goal& goal);
QDataStream& operator>>(QDataStream& stream, Goal& goal);

}

}

#endif
