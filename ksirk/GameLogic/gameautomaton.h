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

#ifndef KSIRK_GAMELOGIC_GAMEAUTOMATON_H
#define KSIRK_GAMELOGIC_GAMEAUTOMATON_H

#include "eventslistproperty.h"

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kgame.h>
#include <libkdegamesprivate/kgame/kplayer.h>
#include <libkdegamesprivate/kgame/kgameio.h>
#include <libkdegamesprivate/kgame/kgamemessage.h>
#include <libkdegamesprivate/kgame/kgameproperty.h>
#include <libkdegamesprivate/kgame/kmessageio.h>
#include <libkdegamesprivate/kgamesvgdocument.h>

#include <QPointF>
#include <QString>
#include <QSvgRenderer>
#include <QMap>
#include <QTextStream>

namespace Ksirk {
  
  class KGameWindow;
  
namespace GameLogic {

class Player;
class Country;
class Goal;

/**
  * All the messages that can be received.
  */
enum KsirkMessagesIds {
    CountryOwner = KGameMessage::IdUser+1, // 257
    PlayerPutsArmy, // 258
    StateChange, // 259
    PlayerChange, // 260
    RegisterCountry, // 261
    PlayerAvailArmies, // 262
    ResetPlayersDistributionData, // 263
    ChangeItem, // 264
    DisplayRecyclingButtons, // 265
    ClearHighlighting, // 266
    ActionRecycling, // 267
    ClearGameActionsToolbar, // 268
    DisplayDefenseButtons, // 269
    ActionDefense, // 270
    FirstCountry, // 271
    SecondCountry, // 272
    InitCombatMovement, // 273
    AnimCombat, // 274
 // 275
    TerminateAttackSequence, // 276
    DecrNbArmies, // 277
    StartLocalCurrentAI, // 278
    Invade, // 279
    Retreat, // 280
    NextPlayerNormal, // 281
    NextPlayerRecycling, // 282
    ShowArmiesToPlace, // 283
    PlayerPutsInitialArmy, // 284
    PlayerRemovesArmy, //285
    VoteRecyclingFinished, // 286
    CancelShiftSource, // 287
    ChangePlayerNation, // 288
    ChangePlayerName, // 289
    StartGame, // 290
    SetNation, // 291
    SetBarFlagButton, // 292
    FinishMoves, // 293
    AnimExplosion, // 294
    SetupOnePlayer, // 295
    SetupWaitedPlayer, // 296
    ValidateWaitedPlayerPassword, // 297
    ValidPassword, // 298
    InvalidPassword, // 299
    SetupCountries, // 300
    AddMsgIdPair, // 301
    CheckGoal, // 302
    SetGoalFor, // 303
    GoalForIs, // 304
    Winner, // 305
    NbPlayers, // 306
    FinalizePlayers, // 307
    Acknowledge, // 308
    DisplayGoals, // 309
    DisplayFightResult, // 310
    MoveSlide, // 311
    InvasionFinished, // 312
    AttackAuto, // 313
    DisplayRecycleDetails, // 314
    CurrentPlayerPlayed, // 315
    NewGameSetupMsg, // 316
    UnusedLastMessageId
};

/** Messages formats:
  * 
  * CountryOwner: country name (QString), owner (QString) 
  * 
  */


/**
  * This is the central class of the game. As a KGame it handles game status,
  * save/load, etc. In addition, it handles communication with other clients on
  * the network.
  * 
  * It is an automaton as its behavior depends on its state and on messages 
  * received from other parts of the game or from other clients on the network.
  * It is also a singleton accessible from anywhere in the process.
  * @author Gael de Chalendar
  */
class GameAutomaton : public KGame
{
Q_OBJECT
    
    friend class GameSequence;

public:
  /**
    * This hackish member is here to allow AIs to run only when goals have been
    * displayed. In fact, the game state should be set to KGame::Run at this
    * moment but if it is done there, then the IO messages are not transmited. I
    * don't understand why. Is it a KGame bug ?
    * As it is a hugly hack, it is made huglier by making it public...
    * @TODO Remove this hack
    */
  bool m_aicannotrunhack;
  
  /**
    * The State enum enumerates all the possible states of the game. The
    * behavior of various method will vary in function of it
    */
  enum GameState {
    INIT, 
    INTERLUDE, // Waiting state after initial distribution of armies at game beginning
    NEWARMIES, 
    WAIT, // Basic state waiting a player action
    WAIT1,
    WAIT_RECYCLING, 
    ATTACK,
    ATTACK2, 
    INVADE, 
    SHIFT1, 
    SHIFT2, 
    FIGHT_BRING,
    FIGHT_ANIMATE, 
    FIGHT_BRINGBACK, 
    WAITDEFENSE, 
    EXPLOSION_ANIMATE, 
    WAIT_PLAYERS, 
    GAME_OVER, // Game finished
    INVALID,
    STARTING_GAME // when displaying the new game ui
  };

  enum NetworkGameType {
    None,
    Socket,
    Jabber
  };
  
  GameAutomaton();
  
  ~GameAutomaton() override;
  
  /** Creates the singleton member if necessary and associate it to the given
    * KGameWindow */
  void init(KGameWindow* gw);

  //@{
  /** Accessors to the game state. */
  GameState state() const;
  void state(GameState state);
  //@}

  /** 
    * Enques the given event the mouse being positioned at the given point
    * @param event The event to register
    * @param point The point where the event occurred
    */
  void gameEvent(const QString& event, const QPointF& point);
  
  /** returns the name of the current state */
  QString stateName() const;
    
  /**
    * Saves all the game parameters with a XML format
    * @param xmlStream the stream on which to write the XML
    */
  void saveXml(QTextStream& xmlStream);

  /** 
    * Retrives the name of the current skin.
    * @return The name of the current skin.
    */
  const QString& skin() const;
  
  /** 
    * Sets the new skin name.
    * @param newSkin The name of the new skin.
    */
  void skin(const QString& newSkin);
  
  /**
    * Reimplementation of a KGame virtual method. Not sure if it is really
    * necessary... 
    *
    * Chooses the next player to make a move by setting setTurn(true,true)
    * in the current player object. If a last player is given then the new
    * player will be chosen so that it is not the last player, i.e. the
    * players a swapped. If no last player is given then the current player
    * is reactiveted
    * @param last - the last player who made a move
    * @param unused
    * @return the player whose turn it will be
    */
  KPlayer* doNextPlayer(KPlayer *last,bool /*exclusive*/);
  
  /** 
    * Retrives the number of player playing from the network.
    * @return the number of players playing from the network.
    */
  inline unsigned int networkPlayersNumber() {return m_networkPlayersNumber;}
  inline void setNetworkPlayersNumber(unsigned int nb) {m_networkPlayersNumber = nb;}
  
  /** Retrives the game window. This one still contains too much game code 
    * that shouldn't be in a GUI class. */
  inline Ksirk::KGameWindow* game() {return m_game;}
  inline const Ksirk::KGameWindow* game() const {return m_game;}
  
  /**
    * Reimplementation of a KGame method. It creates players of different 
    * subclasses of KPlayer, depending on the rtti parameter.
    * @param rtti The class of the created player will depend of this value: 
    * Player if 1; AIPlayer (AIColsonPlayer if 2).
    * @param unused
    * @param isvirtual Will create a local or a (distant) virtual player 
    * depending on this value.
    */
  KPlayer* createPlayer(int rtti, int /**io*/, bool isvirtual) override;
  
  /**
   * @return A pointer to the given named player ; 0 if there is no such player
   */
  Player* playerNamed(const QString& playerName);
    
  //@{
  /** Accessors to the current player */
  Player* currentPlayer();
  void currentPlayer(Player* player);
  //@}

  //@{
  /** Accessors to the information about the fact that the current player 
    * already played or not */
  inline bool currentPlayerPlayed() {return m_currentPlayerPlayed;}
  inline void currentPlayerPlayed(bool val) {m_currentPlayerPlayed = val;}
  //@}

  //@{
  /** Accessors to the saved player, this is the player that will first play 
    * after loading a saved game. */
  inline void savedPlayer(const QString& player) {m_savedPlayer = player;}
  inline void savedState(GameState state) {m_savedState = state;}
  //@}

  //@{
  /** Accessors to the map associating messages to their ids */
  inline QMap<quint32,QString>& ids2msgs() {return m_ids2msgs;}
  inline QMap<QString,quint32>& msgs2ids() {return m_msgs2ids;}
  //@}

  //@{
  /** Accessors to the messages and their ids */
  quint32 idForMsg(const QString& msg);
  QString& msgForId(quint32 id) ;
  //@}

  //@{
  /** Accessors to the goals known by this game. */
  inline const QList< Goal* >& goals() const {return m_goals;}
  inline QList< Goal* >& goals() {return m_goals;}
  //@}

  /** If the game use goals, return true else (all players have to conquier the 
    * world) return false. */
  inline bool useGoals() {return m_useGoals;}
  inline void setUseGoals(bool value) {m_useGoals = value;}
  
  //@{
  /** Accessors to the number of players of this game. */
  inline quint32 nbPlayers() const {return m_nbPlayers;}
  inline void nbPlayers(quint32 nb) {m_nbPlayers = nb;}
  //@}
  
  /**
    * Ask the user how much players there will be in the game and what skin to 
    * use. Called during new game initialization.
    * @param networkGame If the user choose to setup a network game, this 
    * parameter will be true after closing the dialog else it will be false.
    * @param port If the user choose to setup a network game, this parameter 
    * will contain the port on which we will wait for connections.
    * @param newPlayersNumber Will contain the number players of the new game.
    */
  bool setupPlayersNumberAndSkin(NetworkGameType netGameType);
  
    /**
     * Create an IO device like Mouse or Keyboard for the given player
     * and plug it into the player
     * @param player - the player who should get the IO device
     * @param io - the IO code for which a device needs to be created
     */
  void createIO(KPlayer *player,KGameIO::IOMode io);
  
  /** @return true if all players are played by computer ; false otherwise */
  bool allComputerPlayers();
  
  /** @return true if all local players are played by computer ; false otherwise */
  bool allLocalPlayersComputer();
    
  /**
   * Gets a local player.
   * @return Returns a local player... Any of them. Or 0 if there is no local
   * player.
   */
  Player* getAnyLocalPlayer();
  
  //@{
  /** Some accessors to data really necessary... */
  inline QMap< int, QString >& nbArmiesIdsNamesCountriesMap() {return m_nbArmiesIdsNamesCountriesMap;}
  inline QMap< QString, int >& namesNbArmiesIdsCountriesMap() {return m_namesNbArmiesIdsCountriesMap;}
//   inline QMap< int, QString >& nbAddedArmiesIdsNamesCountriesMap() {return m_nbAddedArmiesIdsNamesCountriesMap;}
  inline QMap< QString, int >& namesNbAddedArmiesIdsCountriesMap() {return m_namesNbAddedArmiesIdsCountriesMap;}
  //@}

  void movingFigthersArrived();
  void movingArmiesArrived();
  void movingArmyArrived(Country* country, unsigned int number);
  void firingFinished();
  void explosionFinished();

  void moveSlide();

  /**
    * Change the automatic attack state.
    * @param activated new state
    */
  void setAttackAuto(bool activated);

  /**
    * Get the automatic attack state.
    * @return state
    */
  inline bool isAttackAuto() {return m_attackAuto;}

  /**
    * Change the automatic defense state.
    * @param activated new state
    */
  void setDefenseAuto(bool activated);

  /**
    * Get the automatic defense state.
    * @return state
    */
  bool isDefenseAuto();

  bool finishSetupPlayersNumberAndSkin();

  inline int port() const {return m_port;}

  void askForJabberGames();

  bool startingGame() const;

  QSvgRenderer& rendererFor(const QString& skinName);
  KGameSvgDocument& svgDomFor(const QString& skinName);

  inline NetworkGameType networkGameType() {return m_netGameType;}

  bool joinJabberGame(const QString& nick);

  void removeAllPlayers();
  // Bug 308527.
  void removeAllGoals();

  void newGameNext();
  
  bool connectToServ();

  void checkGoal(Player* player = nullptr);
  
Q_SIGNALS:
  void newJabberGame(const QString&, int, const QString&);
    
public Q_SLOTS:
  /** Reacts to the current state eventualy processing one queued event */
  GameState run();
  
  /**
    * This slot connects to a main signal of the KGame. It will be called
    * whenever a KGame property changes its value. This slot can then
    * used to build up the event distribution of your game, i.e. you react
    * here to all events generated by the game (NewGame, ...)
    * Which property is changed you best find out via the property id.
    * For example: if (prop-id() == myvariable.id()) ...
    *
    * @param prop - the property 
    * @param game - the game object (unused)
    */
  void slotPropertyChanged(KGamePropertyBase *prop,KGame * game);
  
  /**
    * This slot is called whenever a player joins the game.
    * @param player The player that joined the game.
    */
  void slotPlayerJoinedGame(KPlayer* player);
  
  /**
    * Called when data is received from the network.
    * @param msgid Identifies the message.
    * @param buffer Contains the data associated to this message.
    * @param receiver Id of the client destination of this message.
    * @param sender Id of the client sender of this message.
    */
  void slotNetworkData(int msgid, const QByteArray &buffer, 
          quint32 receiver, quint32 sender);
  
  /**
    * Called when a network client connects to this game.
    * @param clientid The id of the client connecting.
    * @param unused
    */
  void slotClientJoinedGame(quint32 clientid, KGame* me);
    
  /**
    * Called when the network connection to the server is broken.
    */
  void slotConnectionToServerBroken();
  
  /**
    * Called when the network connection to a client is broken.
    */
  void slotConnectionToClientBroken(KMessageIO *);

  Country* getDefCountry();

private Q_SLOTS:
  void displayGoals();
  
  
protected:
  friend class Ksirk::KGameWindow;
  
  /**
    * Main method of the KGame which has to be overwritten in our
    * class. It will receive all input a player makes, either via
    * network, via mouse or even a computer player.
    * @param msg - the datastream message containing the information
    * @param player - the player who did the input
    */
  bool playerInput(QDataStream &msg,KPlayer *player) override;

    
  /**
    * Random distribution of countries between players at beginning of game.
    * This is different than in the real board game where countries are chosen
    * one by one by each player. This is in order to quick up game beginning.
    */
  void firstCountriesDistribution();
  
  void setGoalFor(Player* player);
  
private:
  GameAutomaton(const GameAutomaton& /*ga*/) : KGame() {};

  void countriesDistribution();

  void sendCountries();
  
  bool joinNetworkGame();
    
  void changePlayerName(Player* player);
  
  void changePlayerNation(Player* player);
  
  bool startGame();

  void finalizePlayers();
  
  void activateNeededAIPlayers();

  void resetPlayersDistributionData();

  void actionNextPlayer();
  
  GameState m_state;
//   KGameProperty< GameState > m_state;
//     int m_stateId;
  
  Ksirk::KGameWindow *m_game;
  
  EventsListProperty m_events;
  
  static const char* GameStateNames[] ;
  
  static const char* KsirkMessagesIdsNames[] ;
  
  KGamePropertyQString m_skin;
  
  unsigned int m_networkPlayersNumber;
  
  int m_skinId;

  /** @brief The name of the current player */
  QString m_currentPlayer;

  /** @brief false until the current player has played something */

  bool m_currentPlayerPlayed;
//   int m_currentPlayerId;
  
  QMap< int, QString > m_nbArmiesIdsNamesCountriesMap;
  QMap< QString, int > m_namesNbArmiesIdsCountriesMap;
//   QMap< int, QString > m_nbAddedArmiesIdsNamesCountriesMap;
  QMap< QString, int > m_namesNbAddedArmiesIdsCountriesMap;

  quint32 m_choosedToRecycleNumber;
  
  QString m_savedPlayer;
  GameState m_savedState;
  
  QMap<quint32,QString> m_ids2msgs;
  QMap<QString,quint32> m_msgs2ids;
  
  QList< Goal* > m_goals;
  bool m_useGoals;
  
  quint32 m_nbPlayers;

  QList<int> m_choosedToRecycle;

  // tell us if the automatic attack is enabled
  bool m_attackAuto;

  // tell us if the automatic defense is enabled
  struct AutoDefenseStruct
  {
    AutoDefenseStruct() : value(false), firstCountry(nullptr), secondCountry(nullptr) {}
    AutoDefenseStruct(bool v) : value(v), firstCountry(nullptr), secondCountry(nullptr) {}
    bool isDefenseAuto(Country* first, Country* second)
    {
      if (firstCountry!=first || secondCountry!=second)
      {
        firstCountry = nullptr;
        secondCountry = nullptr;
        value = false;
      }
      return value;
    }
    bool value;
    Country* firstCountry;
    Country* secondCountry;
  };
  AutoDefenseStruct m_defenseAuto;

  // Save Defense country
  Country * defCountry;

  int m_port;
  
  bool m_startingGame;

  QMap<QString, QSvgRenderer*> m_renderers;
  QMap<QString, KGameSvgDocument> m_svgDoms;

  NetworkGameType m_netGameType;
};

QDataStream& operator>>(QDataStream& s, GameAutomaton::GameState& state);
  
} // closing namespace GameLogic
} // closing namespace Ksirk

#endif // KSIRK_GAMELOGIC_GAMEAUTOMATON_H
