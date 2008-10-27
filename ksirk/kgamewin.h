/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef KGAMEWIN_H
#define KGAMEWIN_H

#include "krightdialog.h"
#include "KsirkGlobalDefinitions.h"
#include "decoratedgameframe.h"
#include "fightArena.h"
#include "mainMenu.h"
#include "GameLogic/onu.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/player.h"
#include "GameLogic/country.h"
#include "Sprites/animspriteslist.h"

// include files for Qt
#include <QPointF>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QGroupBox>
#include <QSplitter>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
// include files for KDE
#include <ksharedconfig.h>
#include <KXmlGuiWindow>
#include <KStandardDirs>
#include <KShortcut>
// include files for kde games
#include <kgame/kgamechat.h>

// #include <kdialogbase.h>

class QAction;
class KGameChat;
class KGamePopupItem;
class KToolBar;
class KDialog;
class KAction;

class QEvent;
class QDockWidget;
class QGraphicsScene;

namespace Phonon
{
  class MediaObject;
}

namespace Ksirk
{

  // forward declaration of the KsirK classes
  class DecoratedGameFrame;
  class FightArena;
  class mainMenu;
  class AnimSpritesGroup;
  class KRightDialog;
  class NewGameDialogImpl;
  
namespace Sprites
{
  class ArrowSprite;
}

namespace GameLogic
{
  class ONU;
  class KMessageParts;
  class GameAutomaton;
  class Player;
}

/**
  * This is the main window. Due to the history of KsirK, this GUI class 
  * contain really too much code about game logics. This will change in the
  * future.
  *
  * @author Gael de Chalendar (aka Kleag)
  * @version $Id: kgamewin.h 243 2007-02-24 00:22:58Z kleag $
  */
class KGameWindow: public KXmlGuiWindow
{
  Q_OBJECT

public:
  enum MessageShowingType {OnConfig, ForceShowing};
  enum InvasionType {Invasion, Moving};
  enum FightType {Attack, Defense};
    
  /**
    * Create the window and initializes its members
    */
  KGameWindow(QWidget* parent=0);
  
  /**
    * Deletes the background and the pool
    */
  ~KGameWindow();
    
  /** Returns the game graphics view */
  DecoratedGameFrame* frame() {return m_frame;}
/** Returns the game graphics scene*/
//  QGraphicsScene* graphicsscene() {return m_
    
  /** Returns the arena graphics view */
  FightArena* arena() {return m_arena;}

  /** Returns the menu graphics view */
  mainMenu* mMenu() {return m_mainMenu;}
    
  /**
    * Ask all the sprites to repaint themselves
    */
//   void paint();
  
  /**
    * Returns the country inside which the argument point is. 0 if none.
    */
  GameLogic::Country* clickIn(const QPointF& Point);

  /**
    * Loads a new skin.
    */
  void newSkin(const QString& onuDefinitionFileName = "");
  
/************** METHODS BELOW ARE DEFINED IN gestionSprites.cpp **************/
  /**
    * Prepares the sprites to be moved : removes the nb necessary sprites from
    * source, creates the moving sprites and gives them their destination, etc
    */
  bool initArmiesMovement(unsigned int nb, GameLogic::Country* src, GameLogic::Country* dest);

  AnimSprite* initArmiesMultipleCombat(unsigned int nb,
      GameLogic::Country* src,
      GameLogic::Country* dest, QPointF);

  QPointF determinePointDepartArena(GameLogic::Country *pays, int relativePos);

  void determinePointArrivee(
      GameLogic::Country *paysAttaquant,
      GameLogic::Country *paysDefenseur,
      QPointF& pointArriveeAttaquant,
      QPointF& pointArriveeDefenseur);

  void determinePointArriveeForArena(
      GameLogic::Country *paysAttaquant,
      GameLogic::Country *paysDefenseur,
      int relative,
      QPointF& pointArriveeAttaquant,
      QPointF& pointArriveeDefenseur);


  /**
    * Initializes the sprites that will fight for the attacker and the
    * defender. Prepares them for moving
    */
  void initCombatMovement(GameLogic::Country *attacker, 
    GameLogic::Country *defender);
    
  /**
    * Prepare the fighting animation : replace the sprites sequence by the
    * sequence of explosion
    */

  void animCombat();
  /** 
    *  Center the map on the fight so that the user can see what's happening 
  */
  void centerOnFight();

  /**
    * Replaces the animated fighters by the simple cannon image
    */
  void stopCombat();

  /**
    * Replaces the sequence of destroyed cannon(s) by the explosion sequence.
    */
  void animExplosion(int,GameLogic::Country *attacker, 
    GameLogic::Country *defender);

  void animExplosionForArena(GameLogic::Country *attacker, 
    GameLogic::Country *defender);

  /**
    * When all the explosion sequence has been shown for all explosing
    * devices, removes these sprites
    */
  void stopExplosion();

  /**
    * Set up the eventual survivor(s) to come back home
    */
  void initCombatBringBackForArena(GameLogic::Country *, GameLogic::Country *);

  /**
    * Tests if there is currently moving armies (infantrymen, cavalrymen,
    * cannons or fighters)
    * @return true if there is moving armies; false otherwise
    */
  bool haveMovingArmies() const {return !m_animSpritesGroups.empty();}

  void setNbAttack (int nb);

  void setNbDefense (int nb);


/************** END OF METHODS DEFINED IN gestionSprites.cpp *****************/

  /**
    * Prepares the players for the game with human interaction. Return true
    * if successful or false if failure or cancel
    */
  bool setupPlayers();
  bool setupOnePlayer();
  bool setupOneWaitedPlayer();
  bool createWaitedPlayer(quint32 waitedPlayerId);
    
  /**
    * Do the distribution of armies for all players in function of their
    * number of countries
    */
  void distributeArmies();

  /**
    * Computes the number of armies to be distributed to p at the beginning of
    * the turn, function of the number of countries he owns
    * @todo : this method should be a method of Player
    */
  int nbNewArmies(GameLogic::Player *p);

  /**
    * Changes the owner of the attacked country if its number of armies
    * becomes negative or null. Handle the end of the game of the
    * winning conditions are fulfilled.
    */
  bool attackEnd();

  /**
    * Computes the results of the fight
    */
  void resolveAttack();

  //@{
  /**
    * Display and removes various toolbar buttons in function of the state
    * of the game.
    */
  void displayOpenGameButton();
  void displayNextPlayerButton();
  void displayRecyclingButtons();
  void displayNormalGameButtons();
  void displayDefenseButtons();
  void displayInvasionButtons();
  void displayCancelButton();
  void displayDefenseWindow();
  //@}

  void clearGameActionsToolbar(bool send = true);

  /**
    * Updates the flag in the statusbar with the one of the current player
    */
  void setBarFlagButton(const GameLogic::Player* player = 0);

  /**
    * @brief Shortcut for "statusBar()-> changeItem(text, id)"
    */
  void changeItem ( const QString& text, int id = ID_NO_STATUS_MSG, bool log = true);
  
  /**
   * @brief Receives pixmaps and strings to internationalize and display in a 
   * collection with stream API.
   * 
   * When encountering a serie of strings, the first one is the pattern and 
   * those following are the fillers. The end of the collection or a pixmap
   * signals that all fillers have been given.
   * @note No check is made (e.g. on the number of fillers)
   *
   * @param strings @b IN/OUT <I>KsirK::GameLogic::KMessageParts&</I> 
   * the collection holding strings to display.
   * @param id @b IN <I>int</I> 
   * The id of the status bar where to display the internationalized message.
   */
  void changeItem(Ksirk::GameLogic::KMessageParts& strings, 
                  int id = ID_NO_STATUS_MSG, bool log = true);
  
  /**
   * @brief Receives strings to internationalize and display in a collection 
   * with stream API and broadcasts the message to all clients.
   * @param strings @b IN/OUT <I>KsirK::GameLogic::KMessageParts&</I> 
   * the collection holding strings to display.
   * @param id @b IN <I>int</I> 
   * The id of the status bar where to display the internationalized message.
   */
  void broadcastChangeItem(Ksirk::GameLogic::KMessageParts& strings, 
                            int id = ID_NO_STATUS_MSG, bool log = true );
  
  /**
    * @brief Reconnect the mouse events signals to their slots to allow human players
    * to play
    */
  void reconnectMouse();

  /**
    * Disconnects the mouse events signals from their slots to avoid human
    * player actions when it is the turn of the AI
    */
  void disconnectMouse();

  /** Returns the current state of the game */
  GameLogic::GameAutomaton::GameState getState() const;

  /** Return true if the state of the game is the argument; false otherwise */
  bool isMyState(GameLogic::GameAutomaton::GameState state) const;
 
  /** @brief sets the current player to be the one pointed by the argument. 
    * Makes associated actions:
    *
    * Changes the flag in the status bar.
    * @return Returns true in case of success; false otherwise.
    */
//   bool setCurrentPlayer(const GameLogic::Player* player);
    
  /** sets the current player to be the first one. Makes associated actions. */
  int setCurrentPlayerToFirst();
  
  /** 
    * Sets the current player to be the next one in the list. Makes the 
    * associated actions :
    * Changes the flag in the status bar,...
    * @return 0 in case of success; non-zero otherwise. For example, it returns 
    * 1 the current player was the last one 
    */
  int setCurrentPlayerToNext(bool restartRunningAIs = true);
  
  
  bool playerPutsArmy(const QPointF& point, bool removable);
  bool playerPutsInitialArmy(const QPointF& point);
  bool playerRemovesArmy(const QPointF& point);

  /** Sets the attacker country to be the one at the given point */
  bool attacker(const QPointF& point);

  /** Sets the attacked country to be the one at the given point */
  unsigned int attacked(const QPointF& point);

  /** Sets the first country in a fight to be the one at the given point. */
  bool firstCountryAt(const QPointF& point);

  /** Sets the second country in a fight to be the one at the given point. */
  bool secondCountryAt(const QPointF& point);

  /**
    * @brief setups window for recycling
    */
  void initRecycling();

  /** Test if there is some sprites animated for a fight */
  bool haveAnimFighters() const;

  /** Clears all animated sprites lists and shows the post-fight buttons */
  bool terminateAttackSequence();

  /** Called whenever a player choses to end the recycling. */
  bool nextPlayerRecycling();
  
  /** 
    * @return if true next state will be NEWARMIES else it will be WAIT
    */
  bool nextPlayerNormal();
  
  /** Called whenever a player choses to attack with nb armies. */
  void attack(unsigned int nb);

  /** Called whenever a player choses to defend with nb armies. */
  void defense(unsigned int nb);

  /** Called whenever a player choses the cancel button. */
  void cancelAction();

  /** 
    * Called whenever a player choses to cancel the started end of turn move of
    * armies. 
    */
  void cancelShiftSource();

  /** Called when the user clicks the new game button. */
  bool actionNewGame();

  /** Called when the user clicks the open game button. */
  bool actionOpenGame();

  /** Called when the user clicks the recycling button. */
  void actionRecycling();

  /** */
  void postActionRecycling();

  /** Called when the user clicks the recycling finished button. */
  void actionRecyclingFinished();
  
  /**
    * Tests if the move finishing at the given point is valid.
    * @return true if the move is valid; false otherwise. 
    */
  bool isMoveValid(const QPointF& point);

   /**
    * Tests if the fight finishing at the given point is valid.
    * @return true if the fight is valid; false otherwise. 
    */
  bool isFightValid(const QPointF& point);
  
  /**
    * @brief Accessor to the world
    * @return A pointer to the world
    */
  GameLogic::ONU* theWorld();

  /**
    * @brief Adds a player
    */
  GameLogic::Player* addPlayer(const QString& playerName,
        unsigned int nbAvailArmies, 
        unsigned int nbCountries, 
        const QString& nationName, 
                 bool isAI,
                 const QString& password = "",
                 unsigned int nbAttack = 0,
                 unsigned int nbDefense = 0);

  //@{
  /** 
    * accessors and manipulators of the number of armies moved during an invasion 
    * or an end of turn move 
    */
  int nbMovedArmies();
  void incrNbMovedArmies(unsigned int nb = 1);
  void decrNbMovedArmies(unsigned int nb = 1);
  //@}

  //@{
  /** 
    * causes the move to/from the first country stored from/to the second one
    * during an invasion or an end of turn move 
    */
  bool invade(unsigned int nb = 1);
  AnimSprite* simultaneousAttack(int nbArmies, FightType type);
  bool retreat(unsigned int nb = 1);
  //@}

  //@{
  /**
    * causes the end of an invasion or an end of turn move
    */
  void invasionFinished();
  void shiftFinished();
  //@}

  /** @return true if the given player is the last one ; false otherwise */
  bool isLastPlayer(const GameLogic::Player& player);

  inline GameLogic::GameAutomaton* automaton() {return m_automaton;}
  
  /**
    * Gets the background world map sprite. Gives access to the scene and, furthermore, 
    * the background size, thus giving hints for positioning and annimation.
    */
  inline BackGnd* backGndWorld() {return m_backGnd_world;}
  inline const BackGnd* backGndWorld() const {return m_backGnd_world;}
  
  /**
    * Gets the background arena sprite. Gives access to the scene and, furthermore, 
    * the background size, thus giving hints for positioning and annimation.
    */
  inline BackGnd* backGndArena() {return m_backGnd_arena;}
  inline const BackGnd* backGndArena() const {return m_backGnd_arena;}
  
  /**
    * Gets the current background sprite. Gives access to the scene and, furthermore, 
    * the background size, thus giving hints for positioning and annimation.
    */
  BackGnd* backGnd();
  
  //@{
  /** 
    * Accessors to the number of available armies. This one is inherited from a
    * long time ago and should probably be replaced by available armies local 
    * to players. 
    */
  inline void availArmies(unsigned int nb) {m_nbAvailArmies = nb;}
  inline unsigned int availArmies() {return m_nbAvailArmies;}
  //@}

  //@{
  /** 
    * Accessors to the firstly and secondly clicked countries that will become 
    * the attacker and the defender countries (or the source and the target) if 
    * the move is valid
    */
  void firstCountry(GameLogic::Country* country);
  void secondCountry(GameLogic::Country* country);
  GameLogic::Country* firstCountry();
  GameLogic::Country* secondCountry();
  //@}

  /** 
    * Forces all moving sprites to finish  their move be clearing moving 
    * sprites collections. 
    */
  void finishMoves();
    
  /** 
    * Returns the list of players definitions whose connection from the network
    * is waited after loading a saved game. 
    */
  inline QList<GameLogic::PlayerMatrix>& waitedPlayers() {return m_waitedPlayers;}

  /** Displays the buttons associated to the given game state. */
  void displayButtonsForState(GameLogic::GameAutomaton::GameState state);

  /** 
    * The game is over. The given player is the winner. Displays the dialog 
    * stating this fact, depending on if this player is local or distant. 
    */
  void winner(const GameLogic::Player* player);

  /** Returns a list of the nations names associated to their flag's file name. */
  QMap< QString, QString > nationsList();
  
  /** Returns a pointer to the chat widget used to chat and to display messages. */
  inline KGameChat* chatWidget() {return m_chatDlg;}
  
  /** Returns a pointer to the title chat message label used to display messages. */
  inline QLabel* titleChatMessage() {return m_titleChatMsg;}

  void showMessage(const QString& message, quint32 delay=5, MessageShowingType forcing=OnConfig);

  /**
    * Replace the map widget by the arena widget.
    */
  void showArena();

  /**
    * Replace the arena widget by the map widget.
    */
  void showMap();

  /**
    * Replace the mainMenu widget by the map widget.
    */
  void showMainMenu();

  /**
    * The three types of central widget possible. This enum is used for knowing witch
    * is currently displayed (see m_currentDisplayedWidget var below).
    */
  enum widgetType {mainMenuType, mapType,arenaType};

  /**
    * Give type of the central widget currently displayed.
    * @return current widget type
    */
  widgetType currentWidgetType();

  /**
    * Give the central widget currently displayed.
    * @return current widget
    */
  QGraphicsView* currentWidget();
 /**
    * Return the right dialog
    */
  KRightDialog * getRightDialog();

  /** Arena state */
  bool isArena(); 
 
  enum DiceColor {Blue,Red};

  QPixmap getDice(DiceColor color, int num);

  /**
    * Returns the current player
    */
  GameLogic::Player* currentPlayer();

  void slideInvade(GameLogic::Country *,GameLogic::Country *, InvasionType invasionType = Invasion);

  void setNextPlayerActionEnabled(bool value);
  void setSaveGameActionEnabled(bool value);

  void updateScrollArrows();

  bool newGameDialog(
                     unsigned int maxPlayers,
                     const QString& skin);

  bool finishSetupPlayers();

  protected:

  /**
    * Connected to the frame timer, it manages the behavior of the game in
    * function of the value of the state.
    *
    * Fundamental method !!!
    */
//   void slotTimerEvent();

//   void resizeEvent ( QResizeEvent * event );

  /**
    * Add the main toolbar buttons
    */
  void initActions();

  /**
    * Prepares the status bar
    */
  void initStatusBar();

  /**
    * creates and display the main frame with the background
    */
  void initView();

  /**
    * Add a button to the toolbar
    */
  void addAButton(const QString& fileName, 
      const char* slot, 
      const QString& txt, 
      const  KShortcut& shortcut, 
      bool isTemp = false, 
      const QString& toolBarName = "gameActionsToolBar");

  /**
    * Reimplementation of the inherited function : starts the timer.
    */
  void enterEvent(QEvent* ev);

  /**
    * Reimplementation of the inherited function : stops the timer.
    */
  void leaveEvent(QEvent* ev);

  void clear();
  
  /**
    * Reimplementation of the inherited function called when a window close event arise
    */
  bool queryClose();
  bool queryExit();
    
  /** Shows some explanations on how to start playing */
  void explain();

  void reduceChat();
  void unreduceChat();

  
public Q_SLOTS:

  virtual void mouseMoveEvent ( QMouseEvent * event );

  void slideMove(int v);
  void slideReleased();  
  void slideClose();
  //@{
  /**
   * The slots associated to the keys
   * Will initiate different actions depending on the game state
   */
  void slotKey1();
  void slotKey2();
  void slotKey3();
  //@}
  
  //@{
  /**
    * The slots associated to the buttons
    */
  void slotNewGame();
  void slotJoinNetworkGame();
  void slotOpenGame();
  void slotSaveGame();
  void slotRecycling();
  void slotRecyclingFinished();
  void slotNextPlayer();
  void slotAttack1();
  void slotAttack2();
  void slotAttack3();
  void slotDefense1();
  void slotDefense2();
  void slotDefAuto();
  void slotWindowDef1();
  void slotWindowDef2();
  void slotInvade1();

  void slotInvade5();
  void slotInvade10();
  void slotInvasionFinished();
  void slotRetreat1();
  void slotRetreat5();
  void slotRetreat10();
  void slotMove();
  void slotCancel();
  void slotDumpGameInformations();
  void slotFinishMoves();
  void slotArena(bool);
  //@}

  /**
    * The standard slotShowAboutApplication slot
    */
  void slotShowAboutApplication();

  //@{
  /**
    * Connected to the frame mouse buttons signals, they manages the reaction
    * of the game to user interaction inside its main widget in function of
    * the state of the game.
    *
    * Fundamental methods !!!
    */
  void slotLeftButtonDown(const QPointF&);
  void slotLeftButtonUp(const QPointF&);
  void slotRightButtonDown(const QPointF&);
  void slotRightButtonUp(const QPointF&);
  //@}

  /** 
    * This slot displays a dialog explaining the goal of the current player and 
    * describing her progress in this way.
    */
  void slotShowGoal();
  
  /** 
    * Connected to the chat signals.
    */
  void slotChatMessage();
  void slotChatReduceButton();
  void slotChatFloatButtonPressed();
  void slotChatFloatChanged(bool value = true);

  void slotMovingFightersArrived(AnimSpritesGroup* sprites);
  void slotFiringFinished(AnimSpritesGroup*);
  void slotExplosionFinished(AnimSpritesGroup*);
  void slotMovingArmiesArrived(AnimSpritesGroup*);
  void slotBring(AnimSprite*);
  void slotMovingArmyArrived(AnimSprite*);

  void slotZoomIn();
  void slotZoomOut();

  void slotNewGameOK(unsigned int nbPlayers, const QString& skin, bool networkGame, bool useGoals);
  void slotNewGameKO();
  
private Q_SLOTS:
  void optionsConfigure();

  /**
    * Called every 50ms, it causes the scrolling of the world if necessary the
    * movement of the moving sprites. After it call the paint method to do the
    * animation of the sprites and it asks the pool to update
    */
  void evenementTimer();

  void slotRemoveMessage();

  void slotContextualHelp();

  void slotDisableHelp(const QString &);

  void slotArmiesNumberChanged(int);
  
private:

  QDockWidget * m_rightDock;

  KRightDialog * m_rightDialog;

  QStackedWidget *m_centralWidget;

  GameLogic::GameAutomaton* m_automaton;

  KDialog * m_wSlide;

  int m_nbRArmy;
  int m_nbLArmy;
  int m_currentSlideValue;
  int m_previousSlideValue;

  QLabel * m_nbLArmies;
  QLabel * m_nbRArmies;
  QSlider * m_invadeSlide;

  /**
    * State that say the widget that is currently displayed between the map and the arena.
    */
  widgetType m_currentDisplayedWidget;

  /**
   * The widget initialy docked at bottom where is displayed the events history
   */
  QDockWidget* m_bottomDock;
  
  //@{
  /**
    * NKD = Numberof Killed Defenders
    * NKA = Numberof Killed Attackers
    * These numbers are set up by resolveAttack() and are used to compute
    * in various methods
    * @todo: this solution is ugly. Change that.
    */
  int NKD, NKA;
  //@}
  
  bool ARENA;

  int nbSpriteAttacking;
  int nbSpriteDefending;

  int relativePosInArenaAttack;
  int relativePosInArenaDefense;

  /**
    * the countries, continents, etc.
    */
  GameLogic::ONU* m_theWorld;

  /**
    * The main canvas of the world map
    */
  QGraphicsScene* m_scene_world;

  /**
    * The main canvas of the arena
    */
  QGraphicsScene* m_scene_arena;

  /** 
    * The background sprite of the world map. Gives access to the scene and, furthermore, 
    * the background size, thus giving hints for positioning and annimation.
    */
  BackGnd* m_backGnd_world;

  /** 
    * The background sprite of the arena. Gives access to the scene and, furthermore, 
    * the background size, thus giving hints for positioning and animation.
    */
  BackGnd* m_backGnd_arena;

  /**
    * The fighting fighters (represented by firing cannons)
    */
  AnimSpritesGroup* m_animFighters;

  /**
    * The number of armies to move/moving/moved from one country to another
    */
  int m_nbMovedArmies;

  //@{
  /**
    * The 2 countries involved in a fight or in a armies move
    */
  GameLogic::Country* m_firstCountry;
  GameLogic::Country* m_secondCountry;
  //@}

  /**
    *
    */
//    QDialog *dialog;

  /**
    * The map frame of the game, its visual component ; the main widget
    */
  DecoratedGameFrame* m_frame;

  /**
    * The arena frame of the game, its visual component ; the main widget
    */
  FightArena* m_arena;

  /**
    * The menu frame of the game, its visual component ; the main widget
    */
  mainMenu* m_mainMenu;

  /**
    * a shortcut to the standard dirs object.
    */
  KStandardDirs* m_dirs;

  /**
    * This button is used to display the flag of the currently active player
    * in the status bar.
    */
  KAction* m_goalAction;
  QLabel* m_barFlag;
    
//   KAccel m_accels;
  
  QList<QString> m_temporaryAccelerators;
    
  /** Used during countries distribution to handle network lags on the player member */
  unsigned int m_nbAvailArmies; 
  
  /** 
    * The list of players description whose connection is waited after loading 
    * a saved game. 
    */
  QList<GameLogic::PlayerMatrix> m_waitedPlayers;

  /**
   * The prompter where the game events and chat between network players are 
   * displayed
   */
  KGameChat *m_chatDlg;

  /**
    * Show the beginning of the last message received.
    */
  QLabel* m_titleChatMsg;

  /**
    * Reduced state of the chat widget.
    */
  bool m_chatIsReduced;

  /**
    * Contains all dices of the game.
    */
  QMap< DiceColor, QList<QPixmap> > m_dices;

  /**
    * Audio player object: play all the sounds of the game.
    */
  Phonon::MediaObject* m_audioPlayer;

  /** 
    * This timer is used to scroll the window if the mouse stay unmoving near 
    * the window border. 
    */
  QTimer m_timer;

  QList<AnimSpritesGroup*> m_animSpritesGroups;

  KToolBar* gameActionsToolBar;

  KGamePopupItem * m_message;

  GameLogic::Country* m_mouseLocalisation;

  KDialog * dial;

  // components that will be re-used of the chat
  QPixmap m_upChatReducePix;
  QPixmap m_downChatReducePix;
  QPixmap m_upChatFloatPix;
  QPixmap m_downChatFloatPix;

  // last width of the floating chat not reduced
  int m_lastWidthChat;

  QPushButton* m_reduceChatButton;
  QPushButton* m_floatChatButton;

  // the current saved game file name
  QString m_fileName;

  QAction* m_nextPlayerAction;
  KAction* m_saveGameAction;

  Sprites::ArrowSprite* m_uparrow;
  Sprites::ArrowSprite* m_downarrow;
  Sprites::ArrowSprite* m_leftarrow;
  Sprites::ArrowSprite* m_rightarrow;
  
private: // Private methods
  /*void attack(GameLogic::Country& attacker, GameLogic::Country& defender, unsigned int nb);*/
  void moveArmies(GameLogic::Country& src, GameLogic::Country& dest, unsigned int nb);
  void saveXml(std::ostream& xmlStream);
  void loadDices();
  QPixmap buildDice(DiceColor color, const QString& id);
  void setupPopupMessage();

  bool m_networkGame;
  int m_port;
  uint m_newPlayersNumber;

  NewGameDialogImpl* m_newGameDialog;

  GameLogic::GameAutomaton::GameState m_stateBeforeNewGame;
  int m_stackWidgetBeforeNewGame;
};

} // closing namespace Ksirk

#endif // KGAMEWINDOW_H

