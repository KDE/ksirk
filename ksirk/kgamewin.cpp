/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gael de Chalendar <kleag@free.fr>

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

/*  begin                : mer jui 11 22:27:28 EDT 2001   */

// application specific includes
#include "kgamewin.h"
#include "ksirkConfigDialog.h"
#include "ksirksettings.h"
#include "Sprites/animspritesgroup.h"
#include "Sprites/arrowsprite.h"
#include "GameLogic/aiplayer.h"
#include "GameLogic/aiColsonPlayer.h"
#include "GameLogic/aiplayerio.h"
#include "GameLogic/dice.h"
#include "GameLogic/KMessageParts.h"
#include "GameLogic/goal.h"
#include "GameLogic/KsirkChatItem.h"
#include "GameLogic/KsirkChatModel.h"
#include "GameLogic/KsirkChatDelegate.h"
#include "SaveLoad/ksirkgamexmlloader.h"
#include "Sprites/backgnd.h"
#include "Dialogs/kplayersetupwidget.h"
#include "Dialogs/kwaitedplayersetupdialog.h"
#include "Dialogs/restartOrExitDialogImpl.h"
#include "Dialogs/newGameDialogImpl.h"
#include "Dialogs/newGameSummaryWidget.h"
#include "Dialogs/jabbergameui.h"
#include "Dialogs/tcpconnectwidget.h"
#include "im.h"
#include "xmpp_tasks.h"


//include files for QT
#include <QAction>
#include <QDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QHostInfo>
#include <QIcon>
#include <QMenuBar>
#include <QMovie>
#include <QStatusBar>
#include <QString>
#include <QSvgRenderer>
#include <QTreeView>
#include <QUuid>

// include files for KDE
#include <KMessageBox>
#include <KLocalizedString>
#include <KConfig>
#include <KStandardGameAction>
#include <KStandardAction>
#include <KActionCollection>
#include "ksirk_debug.h"
#include <phonon/mediaobject.h>
#include <KGamePopupItem>
#include <KToolBar>
#include <KAboutData>
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kgamechat.h>
#include <sys/utsname.h>

namespace Ksirk
{
using namespace GameLogic;

// port all occurrences of m_accels
// port all occurrences of setBarPos

KGameWindow::KGameWindow(QWidget* parent) :
  KXmlGuiWindow(parent),
  m_rightDock(0),
  m_rightDialog(0),
  m_automaton(new GameAutomaton()),
  m_wSlide(0),
  m_currentDisplayedWidget(MainMenu),
  NKD(0), NKA(0),
  m_useArena(false),
  nbSpriteAttacking(0),
  nbSpriteDefending(0),
  relativePosInArenaAttack(0),
  relativePosInArenaDefense(0),
  m_theWorld(0),
  m_scene_world(0), m_scene_arena(0),
  m_backGnd_world(0), m_backGnd_arena(0),
  m_animFighters(new AnimSpritesGroup(this,SLOT(slotMovingFightersArrived(AnimSpritesGroup*)))),
  m_nbMovedArmies(0),
  m_firstCountry(0), m_secondCountry(0),
  m_frame(0),
  m_arena(0),
  m_mainMenu(0),
  m_goalAction(0),
  m_barFlag(new QLabel(this)),
  m_chatDlg(0),
  m_audioPlayer(Phonon::createPlayer( Phonon::NotificationCategory )),
  m_timer(this),
  m_message(0),
  m_mouseLocalisation(0),
  m_defenseDialog(0),
  m_fileName(),
  m_uparrow(0),
  m_downarrow(0),
  m_leftarrow(0),
  m_rightarrow(0),
  m_reinitializingGame(false),
  m_newGameDialog(0),
  m_newPlayerWidget(0),
  m_stateBeforeNewGame(GameAutomaton::INVALID),
  m_stackWidgetBeforeNewGame(-1),
  m_jabberClient(new JabberClient()),
  m_advertizedHostName(QHostInfo::localHostName()),
  m_jabberGameWidget(0),
  m_presents(),
  m_newGameSetup(new NewGameSetup(m_automaton))
  {
  qCDebug(KSIRK_LOG) << "KGameWindow constructor begin";

  statusBar()->addWidget(m_barFlag);

//   m_accels.setEnabled(true);
  
  QString iconFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Images/soldierKneeling.png");
  if (iconFileName.isNull())
  {
      KMessageBox::error(0, i18n("Cannot load icon<br>Program cannot continue"), i18n("Error!"));
      exit(2);
  }
  QPixmap icon(iconFileName);

  m_bottomDock = new QDockWidget(this);
  m_bottomDock->setObjectName("bottom-dock");
  m_bottomDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  m_bottomDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::TopDockWidgetArea);

  QWidget* titleChatWidget = new QWidget(m_bottomDock);
  QHBoxLayout* titleChatLayout = new QHBoxLayout(titleChatWidget);
  titleChatWidget->setLayout(titleChatLayout);
  titleChatWidget->setFixedHeight(35);

  QWidget* newMessageChat = new QWidget(titleChatWidget);
  QHBoxLayout* newMessageChatLayout = new QHBoxLayout(newMessageChat);
  newMessageChat->setLayout(newMessageChatLayout);

  KsirkChatModel* chatModel = new KsirkChatModel(m_bottomDock,this);
  KsirkChatDelegate* chatDelegate = new KsirkChatDelegate(m_bottomDock);
  // m_bottomDock is the KGameChat parent widget
  m_chatDlg = new KGameChat(m_automaton, 10000, m_bottomDock,chatModel,chatDelegate);
  connect(m_chatDlg,
          SIGNAL(signalReturnPressed(QString)),
          this,
          SLOT(slotChatMessage()));


  m_upChatFloatPix.load(QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Images/2UpArrow.png"));
  m_downChatFloatPix.load(QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Images/2DownArrow.png"));
  m_chatIsReduced = false;

  m_titleChatMsg = new QLabel(i18n("No message..."));
  QPixmap downChatReducePix(QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Images/downArrow.png"));
  m_reduceChatButton = new QPushButton(downChatReducePix,"");
  m_floatChatButton = new QPushButton(m_upChatFloatPix,"");
  m_reduceChatButton->setFixedSize(30,30);
  m_floatChatButton->setFixedSize(30,30);
  connect(m_floatChatButton,
          SIGNAL(clicked()),
          this,
          SLOT(slotChatFloatButtonPressed()));
  connect(m_bottomDock,
          SIGNAL(topLevelChanged(bool)),
          this,
          SLOT(slotChatFloatChanged(bool)));
  connect(m_reduceChatButton,
          SIGNAL(clicked()),
          this,
          SLOT(slotChatReduceButton()));

  newMessageChatLayout->addWidget(m_titleChatMsg);
  m_titleChatMsg->hide();

  titleChatLayout->addWidget(newMessageChat);
  titleChatLayout->addWidget(m_reduceChatButton);
  titleChatLayout->addWidget(m_floatChatButton);

  m_bottomDock->setWidget(m_chatDlg);
  m_bottomDock->setTitleBarWidget(titleChatWidget);
  addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock); // master dockwidget

//    qCDebug(KSIRK_LOG) << "Before initActions";
  initActions();

  qCDebug(KSIRK_LOG) << "Setting up GUI";
  setupGUI();

  qCDebug(KSIRK_LOG) << "Creating automaton";
  m_automaton->init(this);

  // create a central widget if it doesent' exists
  m_centralWidget = new QStackedWidget(this);
  setCentralWidget(m_centralWidget);
  m_mainMenu = new mainMenu(this, m_centralWidget);
  m_mainMenu->init(m_theWorld);
  
  /// @FIXME Hides the "Play KsirK over Jabber Network" button while Jabber
  /// connection does not work
  m_mainMenu->pbJabberGame->hide();
  
  m_newGameDialog = new NewGameWidget(m_newGameSetup, m_centralWidget);
  connect(m_newGameDialog,SIGNAL(newGameOK()), this, SLOT(slotNewGameNext()));
  connect(m_newGameDialog,SIGNAL(newGameKO()), this, SLOT(slotNewGameKO()));
  m_newPlayerWidget = new KPlayerSetupWidget(m_centralWidget);
  connect(m_newPlayerWidget,SIGNAL(next()),this,SLOT(slotNewPlayerNext()));
  connect(m_newPlayerWidget,SIGNAL(previous()),this,SLOT(slotNewPlayerPrevious()));
  connect(m_newPlayerWidget,SIGNAL(cancel()),this,SLOT(slotNewPlayerCancel()));
  connect(m_newPlayerWidget,SIGNAL(previous()),this,SLOT(slotNewPlayerPrevious()));
  connect(m_newPlayerWidget,SIGNAL(cancel()),this,SLOT(slotNewPlayerCancel()));
  qCDebug(KSIRK_LOG) << "create the Jabber widget if it doesn't exist";
  m_jabberGameWidget = new KsirkJabberGameWidget(m_centralWidget);
  m_centralWidget->addWidget(m_mainMenu); // MAINMENU_INDEX 0
  m_centralWidget->addWidget(m_newGameDialog); // NEWGAME_INDEX 1
  m_centralWidget->addWidget(m_jabberGameWidget); // JABBERGAME_INDEX 2
  m_centralWidget->addWidget(m_newPlayerWidget);  // NEWPLAYER_INDEX 3
  m_newGameSummaryWidget = new NewGameSummaryWidget(m_centralWidget);
  connect(m_newGameSummaryWidget->finishButton,SIGNAL(clicked(bool)),this,SLOT(slotStartNewGame()));
  connect(m_newGameSummaryWidget,SIGNAL(previous()),this,SLOT(slotNewPlayerPrevious()));
  connect(m_newGameSummaryWidget,SIGNAL(cancel()),this,SLOT(slotNewPlayerCancel()));
  m_centralWidget->addWidget(m_newGameSummaryWidget);  // NEWGAMESUMMARY_INDEX 4
  m_tcpConnectWidget = new TcpConnectWidget(this);
  m_centralWidget->addWidget(m_tcpConnectWidget);  // TCPCONNECT_INDEX 5
  connect(m_tcpConnectWidget,SIGNAL(next()),this,SLOT(slotConnectToServer()));
  connect(m_tcpConnectWidget,SIGNAL(previous()),this,SLOT(slotTcpConnectPrevious()));
  connect(m_tcpConnectWidget,SIGNAL(cancel()),this,SLOT(slotTcpConnectCancel()));
  m_centralWidget->setCurrentIndex(MAINMENU_INDEX);
  m_currentDisplayedWidget = MainMenu;
  m_bottomDock->hide();
  
  
//    qCDebug(KSIRK_LOG) << "Before initStatusBar";
  initStatusBar();
  
  menuBar()-> show();
  
  explain();
  m_automaton->run();
  setMouseTracking(true);

  m_timer.setSingleShot(true);
  connect(&m_timer,SIGNAL(timeout()),this,SLOT(evenementTimer()));

  m_initialPresence = XMPP::Status ( "", "", 5, true );

  qCDebug(KSIRK_LOG) << "Connecting Jabber signals";
  QObject::connect ( m_jabberClient, SIGNAL (csDisconnected()), this, SLOT (slotCSDisconnected()) );
  QObject::connect ( m_jabberClient, SIGNAL (csError(int)), this, SLOT (slotCSError(int)) );
  QObject::connect ( m_jabberClient, SIGNAL (tlsWarning(QCA::TLS::IdentityResult,QCA::Validity)), this, SLOT (slotHandleTLSWarning(QCA::TLS::IdentityResult,QCA::Validity)) );
  QObject::connect ( m_jabberClient, SIGNAL (connected()), this, SLOT (slotConnected()) );
  QObject::connect ( m_jabberClient, SIGNAL (error(JabberClient::ErrorCode)), this, SLOT (slotClientError(JabberClient::ErrorCode)) );
  
//   QObject::connect ( m_jabberClient, SIGNAL (subscription(XMPP::Jid,QString)), this, SLOT (slotSubscription(XMPP::Jid,QString)) );
  QObject::connect ( m_jabberClient, SIGNAL (rosterRequestFinished(bool)), this, SLOT (slotRosterRequestFinished(bool)) );
//   QObject::connect ( m_jabberClient, SIGNAL (newContact(XMPP::RosterItem)), this, SLOT (slotContactUpdated(XMPP::RosterItem)) );
//   QObject::connect ( m_jabberClient, SIGNAL (contactUpdated(XMPP::RosterItem)), this, SLOT (slotContactUpdated(XMPP::RosterItem)) );
//   QObject::connect ( m_jabberClient, SIGNAL (contactDeleted(XMPP::RosterItem)), this, SLOT (slotContactDeleted(XMPP::RosterItem)) );
//   QObject::connect ( m_jabberClient, SIGNAL (resourceAvailable(XMPP::Jid,XMPP::Resource)), this, SLOT (slotResourceAvailable(XMPP::Jid,XMPP::Resource)) );
//   QObject::connect ( m_jabberClient, SIGNAL (resourceUnavailable(XMPP::Jid,XMPP::Resource)), this, SLOT (slotResourceUnavailable(XMPP::Jid,XMPP::Resource)) );
  QObject::connect ( m_jabberClient, SIGNAL (messageReceived(XMPP::Message)), this, SLOT (slotReceivedMessage(XMPP::Message)) );
//   QObject::connect ( m_jabberClient, SIGNAL (incomingFileTransfer()), this, SLOT (slotIncomingFileTransfer()) );
  QObject::connect ( m_jabberClient, SIGNAL (groupChatJoined(XMPP::Jid)), this, SLOT (slotGroupChatJoined(XMPP::Jid)) );
  QObject::connect ( m_jabberClient, SIGNAL (groupChatLeft(XMPP::Jid)), this, SLOT (slotGroupChatLeft(XMPP::Jid)) );
  QObject::connect ( m_jabberClient, SIGNAL (groupChatPresence(XMPP::Jid,XMPP::Status)), this, SLOT (slotGroupChatPresence(XMPP::Jid,XMPP::Status)) );
  
  QObject::connect ( m_jabberClient, SIGNAL (groupChatError(XMPP::Jid,int,QString)), this, SLOT (slotGroupChatError(XMPP::Jid,int,QString)) );
  QObject::connect ( m_jabberClient, SIGNAL (debugMessage(QString)), this, SLOT (slotClientDebugMessage(QString)) );
  
  m_jabberClient->setUseXMPP09 ( true );
//     m_jabberClient->setUseSSL ( true );
  m_jabberClient->setAllowPlainTextPassword ( true );

  struct utsname utsBuf;
  uname (&utsBuf);
  m_jabberClient->setClientName ("KsirK");
  m_jabberClient->setClientVersion (KAboutData::applicationData().version());
  m_jabberClient->setOSName (QString ("%1 %2").arg (utsBuf.sysname, 1).arg (utsBuf.release, 2));
  
  // Set caps node information
  m_jabberClient->setCapsNode("http://ksirk.kde.org/jabber/caps");
  m_jabberClient->setCapsVersion(KAboutData::applicationData().version());
  
  // Set Disco Identity information
  DiscoItem::Identity identity;
  identity.category = "client";
  identity.type = "pc";
  identity.name = "KsirK";
  m_jabberClient->setDiscoIdentity(identity);

  connect (this, SIGNAL(newJabberGame(QString,int,QString)), m_automaton, SIGNAL(newJabberGame(QString,int,QString)));

  m_automaton->skin("skins/default");
}

KGameWindow::~KGameWindow()
{
  qCDebug(KSIRK_LOG);

  if (m_jabberClient != 0)
  {
    delete m_jabberClient;
    m_jabberClient = 0;
  }
  if (m_automaton != 0)
  {
    m_automaton->setGameStatus( KGame::End );
    delete m_automaton; m_automaton = 0;
  }
  delete m_backGnd_world; m_backGnd_world = 0;
  delete m_scene_world; m_scene_world = 0;
//   if (m_barFlagButton) {delete m_barFlagButton; m_barFlagButton = 0;}
  delete m_frame; m_frame = 0;
  delete m_backGnd_arena; m_backGnd_arena = 0;
  delete m_arena; m_arena = 0;
  delete m_scene_arena; m_scene_arena = 0;
  delete m_mainMenu; m_mainMenu = 0;
  delete m_audioPlayer;
  delete m_rightDialog;
  delete m_defenseDialog;
  delete m_newGameSetup;
}

void KGameWindow::initActions()
{
  QAction *action;

  // standard game actions
  action = KStandardGameAction::gameNew(this, SLOT(slotNewGame()), this);
  actionCollection()->addAction(action->objectName(), action);
  action = KStandardGameAction::load(this, SLOT(slotOpenGame()), this);
  actionCollection()->addAction(action->objectName(), action);
  m_saveGameAction = KStandardGameAction::save(this, SLOT(slotSaveGame()), this);
  m_saveGameAction->setEnabled(false);
  actionCollection()->addAction(m_saveGameAction->objectName(), m_saveGameAction);
  action = KStandardGameAction::quit(this, SLOT(close()), this);
  actionCollection()->addAction(action->objectName(), action);

  m_zoomInAction = KStandardAction::zoomIn(this, SLOT(slotZoomIn()), this);
  m_zoomInAction->setEnabled(false);
  actionCollection()->addAction(m_zoomInAction->objectName(), m_zoomInAction);

  m_zoomOutAction = KStandardAction::zoomOut(this, SLOT(slotZoomOut()), this);
  m_zoomOutAction->setEnabled(false);
  actionCollection()->addAction(m_zoomOutAction->objectName(), m_zoomOutAction);

  KStandardAction::preferences( this, SLOT(optionsConfigure()), actionCollection() );

  // specific ksirk action
  QString imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, "jabber.png");
  //   qCDebug(KSIRK_LOG) << "Trying to load button image file: " << imageFileName;
  if (imageFileName.isNull())
  {
    KMessageBox::error(0, i18n("Cannot load button image %1<br>Program cannot continue",QString("jabber.png")), i18n("Error!"));
    exit(2);
  }
  m_jabberAction = new QAction(QIcon(QPixmap(imageFileName)), i18n("Play over Jabber"), this);
  m_jabberAction-> setText(i18n("Play KsirK over the Jabber Network"));
  m_jabberAction-> setIconText(i18n("Jabber"));
  actionCollection()->setDefaultShortcut(m_jabberAction, Qt::CTRL+Qt::Key_J);
  m_jabberAction->setStatusTip(i18n("Allow to connect to a KsirK Jabber Multi User Gaming Room to create new games or to join present games"));
  connect(m_jabberAction,SIGNAL(triggered(bool)),this,SLOT(slotJabberGame()));
  qCDebug(KSIRK_LOG) << "Adding action game_jabber";
  actionCollection()->addAction("game_jabber", m_jabberAction);
  
  // specific ksirk action
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + '/' + CM_NEWNETGAME);
  //   qCDebug(KSIRK_LOG) << "Trying to load button image file: " << imageFileName;
  if (imageFileName.isNull())
  {
    KMessageBox::error(0, i18n("Cannot load button image %1<br>Program cannot continue",QString(CM_NEWNETGAME)), i18n("Error!"));
    exit(2);
  }
  QAction * newSocketAction = new QAction(QIcon(QPixmap(imageFileName)), i18n("New Standard TCP/IP Network Game"), this);
  newSocketAction->setIconText(i18n("New TCP/IP"));
  actionCollection()->setDefaultShortcut(newSocketAction, Qt::CTRL+Qt::Key_T);
  newSocketAction->setStatusTip(i18n("Create a new standard TCP/IP network game"));
  connect(newSocketAction,SIGNAL(triggered(bool)),this,SLOT(slotNewSocketGame()));
  qCDebug(KSIRK_LOG) << "Adding action game_new_socket";
  actionCollection()->addAction("game_new_socket", newSocketAction);
  
                                     
  // specific ksirk action
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + '/' + CM_NEWNETGAME);
  //   qCDebug(KSIRK_LOG) << "Trying to load button image file: " << imageFileName;
  if (imageFileName.isNull())
  {
    KMessageBox::error(0, i18n("Cannot load button image %1<br>Program cannot continue",QString(CM_NEWNETGAME)), i18n("Error!"));
    exit(2);
  }
  QAction * joinAction = new QAction(QIcon(QPixmap(imageFileName)),
        i18n("Join a Standard TCP/IP Network Game"), this);
  joinAction->setIconText(i18n("Join TCP/IP"));
  actionCollection()->setDefaultShortcut(joinAction, Qt::CTRL+Qt::SHIFT+Qt::Key_J);
  joinAction->setStatusTip(i18n("Join a standard TCP/IP network game"));
  connect(joinAction,SIGNAL(triggered(bool)),this,SLOT(slotJoinNetworkGame()));
   qCDebug(KSIRK_LOG) << "Adding action game_join_socket";
  actionCollection()->addAction("game_join_socket", joinAction);

  m_goalAction = new QAction(i18n("Goal"), this);
  m_goalAction-> setText(i18n("Display the Current Player's Goal"));
  m_goalAction-> setIconText("  ");
  actionCollection()->setDefaultShortcut(m_goalAction, Qt::CTRL+Qt::Key_G);
  m_goalAction->setStatusTip(i18n("Display the current player's goal"));
  connect(m_goalAction,SIGNAL(triggered(bool)),this,SLOT(slotShowGoal()));
  m_goalAction->setVisible(false);
  qCDebug(KSIRK_LOG) << "Adding action game_goal";
  actionCollection()->addAction("game_goal", m_goalAction);
  
  m_contextualHelpAction = new QAction(
        i18n("Contextual Help"), this);
  m_contextualHelpAction->setEnabled(false);
  actionCollection()->setDefaultShortcut(m_contextualHelpAction, Qt::CTRL+Qt::Key_F1);
  connect(m_contextualHelpAction,SIGNAL(triggered(bool)),this,SLOT(slotContextualHelp()));
  actionCollection()->addAction("help_contextual", m_contextualHelpAction);


  QString nextPlayerActionImageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + '/' + CM_NEXTPLAYER);
  m_nextPlayerAction =  new QAction(QIcon(nextPlayerActionImageFileName),
        i18n("Next Player"), this);
  connect(m_nextPlayerAction, SIGNAL(triggered(bool)), this, SLOT(slotNextPlayer()));
  m_contextualHelpAction->setStatusTip(i18n("Lets the next player play"));
  m_nextPlayerAction->setEnabled(false);
  actionCollection()->addAction("game_nextplayer", m_nextPlayerAction);

  QAction * finishMovesAction = new QAction(
        i18n("Finish moves"), this);
  actionCollection()->setDefaultShortcut(finishMovesAction, Qt::Key_Space);
  finishMovesAction->setStatusTip(i18n("Finish moving the current sprites"));
  connect(finishMovesAction,SIGNAL(triggered(bool)),this,SLOT(slotFinishMoves()));
  actionCollection()->addAction("game_finish_moves", finishMovesAction);


}

void KGameWindow::initStatusBar()
{
  statusBar()-> setSizeGripEnabled(true);
  //QT5 statusBar()->insertPermanentItem("", ID_STATUS_MSG, 2);
  //QT5 statusBar()-> setItemAlignment(ID_STATUS_MSG, Qt::AlignLeft | Qt::AlignVCenter);

  //QT5 statusBar()->insertPermanentItem("", ID_STATUS_MSG2, 3);
  //QT5 statusBar()-> setItemAlignment(ID_STATUS_MSG2, Qt::AlignLeft | Qt::AlignVCenter);
  //QT5 statusBar()->addPermanentWidget(m_barFlag);
}

Country* KGameWindow::clickIn(const QPointF &pointf)
{
//   qCDebug(KSIRK_LOG) << "KGameWindow::clickIn " << pointf;
 /* if(isMyState(GameLogic::GameAutomaton::INIT) || m_theWorld-> countryAt( pointf )==0)
  {
    m_rightDock->hide();
  }*/
  return m_theWorld-> countryAt( pointf );
}

Player* KGameWindow::currentPlayer()
{
//   qCDebug(KSIRK_LOG) << "KGameWindow::currentPlayer";
  Player* current = m_automaton->currentPlayer();

  return current;
}

void KGameWindow::loadDices()
{
  qCDebug(KSIRK_LOG);
  
  m_dices[Blue] = QList<QPixmap>();
  m_dices[Red] = QList<QPixmap>();
  m_dices[Blue].push_back(buildDice("bluedice1"));
  m_dices[Blue].push_back(buildDice("bluedice2"));
  m_dices[Blue].push_back(buildDice("bluedice3"));
  m_dices[Blue].push_back(buildDice("bluedice4"));
  m_dices[Blue].push_back(buildDice("bluedice5"));
  m_dices[Blue].push_back(buildDice("bluedice6"));
  m_dices[Red].push_back(buildDice("reddice1"));
  m_dices[Red].push_back(buildDice("reddice2"));
  m_dices[Red].push_back(buildDice("reddice3"));
  m_dices[Red].push_back(buildDice("reddice4"));
  m_dices[Red].push_back(buildDice("reddice5"));
  m_dices[Red].push_back(buildDice("reddice6"));
}

QPixmap KGameWindow::buildDice(const QString& id)
{
  qCDebug(KSIRK_LOG);

  QSize size(32,32);
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter p(&image);
  if (m_theWorld != 0)
  {
    m_theWorld->renderer()->render(&p, id);
  }

  return QPixmap::fromImage(image);
}

QPixmap KGameWindow::getDice(DiceColor color, int num)
{
  if(num==0 || num==-1)
{return QPixmap();}
  else {return m_dices[color][num-1];}
}

void KGameWindow::newSkin(const QString& onuFileName)
{
  qCDebug(KSIRK_LOG) << onuFileName;
  clear();

  m_animFighters->clear();
  foreach(AnimSpritesGroup* sprites, m_animSpritesGroups)
  {
    sprites->clear();
    delete sprites;
  }
  m_animSpritesGroups.clear();

  if (m_centralWidget != 0)
  {
    m_centralWidget->setCurrentIndex(-1);
  }

// NOTE:I wanted to recreate the automaton here. But it isn't possible as this
// method is called from inside a GameAutomaton method. Furthermore, it isn't
// a good solution because the central KGame object should not be recreated
// but reinitialized as needed
//   m_automaton->init(0);
//   delete m_automaton;
//   m_automaton = new GameAutomaton();
//   m_automaton->init(this);

  m_mouseLocalisation = 0;
  if (m_theWorld != 0)
  {
//     m_theWorld-> reset();
    delete m_theWorld;
    m_theWorld = 0;
  }

  QString onuDefinitionFileName = onuFileName;
  if (onuDefinitionFileName.isEmpty())
  {
    onuDefinitionFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Data/world.desktop");
  }
  else if (!QFile::exists(onuDefinitionFileName))
  {
    onuDefinitionFileName.clear();
  }
  if (onuDefinitionFileName.isEmpty())
  {
      KMessageBox::error(0,
          i18n("World definition file not found - Verify your installation<br>Program cannot continue"), i18n("Error!"));
      exit(2);
  }
  // Bug 308527. Need to remove all goals on new game. Only goals for selected skin will be loaded.
  m_automaton->removeAllGoals();
  qCDebug(KSIRK_LOG) << "Got World definition file name: " <<  onuDefinitionFileName;
  m_theWorld = new ONU(m_automaton, onuDefinitionFileName);
  if (m_theWorld->skin().isEmpty())
  {
    delete m_theWorld;
    m_theWorld = 0;
  }
  loadDices();

  // put the size to the window size if it's the main menu
  int width;
  int height;
  if (m_scene_arena == 0 || m_theWorld == 0)
  {
     width = 2000;
     height = 2000;
  }
  else
  {
     width = m_theWorld->width();
     height = m_theWorld->height();
  }

  //Creation of the arena background
  if (m_backGnd_arena != 0)
  {
    qCDebug(KSIRK_LOG) << "Before m_backGnd_arena delete";
    delete m_backGnd_arena;
  }
  //Creation of the background
  if (m_backGnd_world != 0)
  {
    qCDebug(KSIRK_LOG) << "Before m_backGnd_world delete";
    delete m_backGnd_world;
  }

  // create the arena view
  if (m_scene_arena != 0)
  {
    qCDebug(KSIRK_LOG) << "Before m_scene_arena delete";
    delete m_scene_arena;
  }
  m_scene_arena = new QGraphicsScene(0, 0, width, height,this);

  if (m_frame != 0)
  {
    m_frame->setUpdatesEnabled(false);
    m_uparrow = 0;
    m_downarrow = 0;
    m_leftarrow = 0;
    m_rightarrow = 0;
    m_centralWidget->removeWidget(m_frame);
    delete m_frame;
    m_frame = 0;
  }
  if (m_scene_world != 0)
  {
    qCDebug(KSIRK_LOG) << "Before m_scene_world delete";
    delete m_scene_world;
  }
  m_scene_world = new QGraphicsScene(0, 0, width, height,this);

  qCDebug(KSIRK_LOG) << "create the world map view";
  if (m_theWorld != 0)
  {
    m_frame = new DecoratedGameFrame(m_centralWidget, width, height, m_automaton);
    m_frame->setMaximumWidth(width);
    m_frame->setMaximumHeight(height);
    m_frame->setCacheMode( QGraphicsView::CacheBackground );
    m_frame->setIcon();
  }
  
  if (m_arena != 0)
  {
    m_centralWidget->removeWidget(m_arena);
    delete m_arena;
    m_arena = 0;
  }
  if (m_theWorld != 0)
  {
    m_arena = new FightArena(m_centralWidget, width, height, m_scene_arena, m_theWorld, m_automaton);
    m_arena->setMaximumWidth(width);
    m_arena->setMaximumHeight(height);
    m_arena->setCacheMode( QGraphicsView::CacheBackground );
  }
  
  qCDebug(KSIRK_LOG) << "put the menu, map and arena in the central widget";
  if (m_frame != 0)
  {
    m_centralWidget->addWidget(m_frame); // MAP_INDEX 6
  }
  if (m_arena != 0)
  {
    m_centralWidget->addWidget(m_arena); // ARENA_INDEX 7
  }
  
  if (m_theWorld == 0)
  {
    return;
  }
  m_backGnd_arena = new BackGnd(m_scene_arena, m_theWorld, true);
  m_backGnd_world = new BackGnd(m_scene_world, m_theWorld);

//   m_scene_world->setDoubleBuffering(true);
  qCDebug(KSIRK_LOG) << "Before initView";
  initView();
  qCDebug(KSIRK_LOG) <<"After m_backGnd new="<< m_backGnd_world;
  m_frame->setFocus();

  m_uparrow = new Sprites::ArrowSprite(Qt::UpArrow, m_backGnd_world);
  m_uparrow->setZValue(1000);
  QPointF pos = m_frame->mapToScene(QPoint(m_frame->viewport()->width()/2,0));
  pos = pos + QPointF(-(m_uparrow->boundingRect().width()/2),m_uparrow->boundingRect().height());
  m_uparrow->setPos(pos);
  m_uparrow->setActive(false);
  
  m_downarrow = new Sprites::ArrowSprite(Qt::DownArrow, m_backGnd_world);
  m_downarrow->setZValue(1000);
  pos = m_frame->mapToScene(QPoint(m_frame->viewport()->width()/2,m_frame->viewport()->height()));
  pos = pos - QPointF(m_downarrow->boundingRect().width()/2,m_downarrow->boundingRect().height());
  m_downarrow->setPos(pos);
  m_downarrow->setActive(false);
  pos = m_frame->mapToScene(QPoint(0,m_frame->viewport()->height()/2));
  
  m_leftarrow = new Sprites::ArrowSprite(Qt::LeftArrow, m_backGnd_world);
  m_leftarrow->setZValue(1000);
  pos = pos + QPointF(m_leftarrow->boundingRect().width(),-(m_leftarrow->boundingRect().height()/2));
  m_leftarrow->setPos(pos);
  m_leftarrow->setActive(false);
  
  m_rightarrow = new Sprites::ArrowSprite(Qt::RightArrow, m_backGnd_world);
  m_rightarrow->setZValue(1000);
  pos = m_frame->mapToScene(QPoint(m_frame->viewport()->width(),m_frame->viewport()->height()/2));
  pos = pos - QPointF(m_rightarrow->boundingRect().width(),m_rightarrow->boundingRect().height()/2);
  m_rightarrow->setPos(pos);
  m_rightarrow->setActive(false);
  
  qCDebug(KSIRK_LOG) <<"End new skin";
}

KRightDialog * KGameWindow::getRightDialog()
{
  return m_rightDialog;
}

void KGameWindow::initView()
{
  qCDebug(KSIRK_LOG);
  QString iconFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Images/soldierKneeling.png");
  if (iconFileName.isNull())
  {
      KMessageBox::error(0, i18n("Cannot load icon<br>Program cannot continue"), i18n("Error!"));
      exit(2);
  }
// to port : still necessary ?
//   m_frame->setIcon(QPixmap(iconFileName));

  disconnectMouse();
  reconnectMouse();
  setCaption("KsirK",false);
  m_scene_world-> update();
  m_frame->setScene(m_scene_world);
  
  m_scene_arena-> update();
  m_arena->setScene(m_scene_arena);

  //ADD a dock widget on the right

  if (m_rightDialog != 0)
  {
    m_rightDialog->hide();
    delete m_rightDialog;
  }

  if (m_rightDock != 0)
  {
    m_rightDock->hide();
    delete m_rightDock;
  }
  m_rightDock = new QDockWidget(this);
  m_rightDock->setObjectName("right-dock");
  m_rightDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
  m_rightDock->setFeatures(QDockWidget::NoDockWidgetFeatures);

  m_rightDialog = new KRightDialog(m_rightDock,theWorld(),this);
  m_rightDock->setWidget(m_rightDialog);
  qCDebug(KSIRK_LOG) << "hiding right dock";
  m_rightDock->hide();
  addDockWidget(Qt::RightDockWidgetArea, m_rightDock);
  m_frame->setFocus();
}

bool KGameWindow::attackEnd()
{
  qCDebug(KSIRK_LOG);
  if (m_firstCountry==0 || m_secondCountry == 0)
  {
    return false;
  }

  m_firstCountry->releaseHighlightingLock();
  m_firstCountry->clearHighlighting();
  m_secondCountry->releaseHighlightingLock();
  m_secondCountry->clearHighlighting();

  // Bug 315491.
  bool playerDeleted = false;
  bool res = false;
  QString mes = "";
  qCDebug(KSIRK_LOG) << "There is now " << m_secondCountry-> nbArmies() << " armies in " << m_secondCountry->name() << ".";
  if (m_secondCountry-> nbArmies() < 1)
  {
    QPixmap pm = currentPlayer()->getFlag()->image(0);

    KMessageParts messageParts;
    messageParts << pm << I18N_NOOP("<font color=\"red\">%1 conquered %2 from %3</font>") << currentPlayer()->name() << m_secondCountry-> name() << m_firstCountry-> name();
    broadcastChangeItem(messageParts, ID_NO_STATUS_MSG);
    
    Player* oldOwner = m_secondCountry-> owner();
    unsigned int newOldOwnerNbCountries = oldOwner-> getNbCountries() - 1;
    
    if (m_automaton->isAdmin())
    {
      currentPlayer()-> incrNbCountries();
      oldOwner-> decrNbCountries();
    }

    m_secondCountry-> owner(currentPlayer());
    m_secondCountry-> nbArmies(currentPlayer()->getNbAttack());
    m_firstCountry-> decrNbArmies(currentPlayer()->getNbAttack());
    m_scene_world-> update();
    if (m_firstCountry->nbArmies() > 1)
    {
      res = true;
    }
    qCDebug(KSIRK_LOG) << oldOwner-> name() << " now owns " << newOldOwnerNbCountries << " countries.";
    if (newOldOwnerNbCountries == 0)
    {
      QString oldOwnerId = oldOwner->name();
      showMessage(i18n("%1, you are defeated! Bye, bye...",oldOwner->name()), 10, ForceShowing);
/*      KMessageBox::information(this,
                               i18n("%1, you are defeated! Bye, bye...",oldOwner->name()),
                               i18n("KsirK - Game Over!"));*/
      if (m_automaton->isAdmin())
      {
        // Bug 315491.
        playerDeleted = true;
        qCDebug(KSIRK_LOG) << "Removing player " << oldOwner-> name();
        int i = m_automaton->playerList()->indexOf(oldOwner);
        if (i != -1)
          delete m_automaton->playerList()->takeAt(i);
        qCDebug(KSIRK_LOG) << "There is now " << m_automaton->playerList()->count() << " players";
        m_automaton->setMinPlayers(m_automaton->playerList()->count());
        m_automaton->setGameStatus(KGame::Run);
      }
      if ( m_automaton->isAdmin()
        && ( ( m_automaton->useGoals()
             && ( currentPlayer()->goal().type() == GameLogic::Goal::GoalPlayer)
             && ( *currentPlayer()->goal().players().begin() == oldOwnerId ) )
           || (m_automaton->playerList()->count() == 1) ) )
      {
        m_automaton->state(GameLogic::GameAutomaton::GAME_OVER);
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream << currentPlayer()->id();
        m_automaton->sendMessage(buffer,Winner);
        res = false;
      }
      else if (m_automaton->isAdmin())
      {
        foreach (KPlayer* player, *m_automaton->playerList())
        {
          m_automaton->checkGoal((Player*)player);
        }
      }
    }
    else if (m_automaton->isAdmin())
    {
      m_automaton->checkGoal();
    }
  }
  // Bug 315491.
  if (!playerDeleted)
  {
    if (backGnd()->bgIsArena())
    {
      m_arena->countryAttack()->createArmiesSprites();
      m_arena->countryDefense()->createArmiesSprites();
    }
    else
    {
      m_firstCountry->createArmiesSprites();
      m_secondCountry->createArmiesSprites();
    }
  }
  if (m_automaton->isAdmin())
  {
    if (res)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      m_automaton->sendMessage(buffer,StartLocalCurrentAI);
    }
    else
    {
      if (m_firstCountry->nbArmies() < 2 || !m_automaton->isAttackAuto())
      {
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        m_automaton->sendMessage(buffer,ClearHighlighting);
        KMessageParts messageParts;
        messageParts << I18N_NOOP("%1: it is up to you again") << currentPlayer()-> name();
        broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
      }
    }
  }
  m_automaton->setGameStatus(KGame::Run);
  return res;
}

void KGameWindow::winner(const Player* player)
{
  qCDebug(KSIRK_LOG) << player->name();
  QString msg = i18n("%1 won!");
  if (!player->isVirtual())
  {
    msg = i18n("<big><b>%1</b>, you won!</big>");
  }
  if (m_automaton->useGoals())
  {
    msg += i18n("<br>Winner's goal was stated like this:<br><i>%1</i><br><br>Do you want to play again?", player->goal().message());
  }
  else
  {
    if (!player->isVirtual())
    {
      msg += i18n("<br>You conquered all the world!");
    }
    else
    {
      msg += i18n("<br>He conquered all the world!");
    }
  }
  RestartOrExitDialogImpl* restartDia = new RestartOrExitDialogImpl(i18n(msg.toUtf8().data(),player->name()));
              
  connect((QObject*)restartDia->newGameButton,
              SIGNAL(clicked()),
              this,
              SLOT(slotNewGame()));

  connect((QObject*)restartDia->exitButton,
              SIGNAL(clicked()),
              this,
              SLOT(slotExit()));

  restartDia->show();
}

void KGameWindow::resolveAttack()
{

//    qCDebug(KSIRK_LOG) << "KGameWindow::resolveAttack";

  int A1 = -1; int A2 = -1; int A3 = -1; int AE = -1;
  int D1 = -1; int D2 = -1; int DE = -1;

  NKD = NKA = 0;
  if (currentPlayer() == 0 || m_secondCountry == 0 || m_secondCountry->owner() == 0 || m_firstCountry == 0)
    return;

  int secondOldNbArmies = m_secondCountry->nbArmies();

  A1 = Dice::roll();
  if (currentPlayer()-> getNbAttack() > 1) A2 = Dice::roll();
  if (currentPlayer()-> getNbAttack() > 2) A3 = Dice::roll();
  if ((A1>=A2)&&(A1>=A3))  // A1 is the greater ; don't move it; look at the two others
    if (A2>=A3) {} // A2 greater than A3 ; don't move
    else {AE = A2;A2 = A3;A3 = AE;} // A2 lesser than A3 ; swap them
  else
  { // A1 is not the greater
    if (A2>=A3) {AE=A1;A1=A2;A2=AE;}  // A2 is greater than A3, so it is the greater; swap it with A1
    else {AE=A1;A1=A3;A3=AE;} // A3 is greater than A2, so it is the greater; swap it with A1
                                            // now the new A1 is the greater, look at the 2 others
    if (A2>=A3) {} // A2 greater than A3, nothing to do
    else {AE=A2;A2=A3;A3=AE;} // A2 lesser than A3; swap them
  }
  D1 = Dice::roll();
  if (m_secondCountry-> owner()-> getNbDefense() > 1)
    D2 = Dice::roll();
  if (D2>D1) {DE=D1;D1=D2;D2=DE;}
  if (A1>D1)
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << m_secondCountry->name() << quint32(1);
    m_automaton->sendMessage(buffer,DecrNbArmies);
    NKD++;
  }
  else
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << m_firstCountry->name() << quint32(1);
    m_automaton->sendMessage(buffer,DecrNbArmies);
    NKA++;
  }
  if ((A2>0)&&(D2>0))
  {
    if (A2>D2)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << m_secondCountry->name() << quint32(1);
      m_automaton->sendMessage(buffer,DecrNbArmies);
      NKD++;
    }
    else
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << m_firstCountry->name() << quint32(1);
      m_automaton->sendMessage(buffer,DecrNbArmies);
      NKA++;
    }
  }
//   qCDebug(KSIRK_LOG) << "(" << A1<<", "<<A2<<", "<<A3<<") <-> ("<<D1<<", "<<D2<<")" ;
//   qCDebug(KSIRK_LOG) << "Attacker loses " << NKA<<" armies; Defender loses "<<NKD<<" armies." ;
  
  QByteArray buffer3;
  QDataStream stream3(&buffer3, QIODevice::WriteOnly);
  stream3 << (quint32)A1 << (quint32)A2 << (quint32)A3 << (quint32)D1 << (quint32)D2 << (quint32)NKA << (quint32)NKD << (quint32)(secondOldNbArmies-NKD < 1);
  qCDebug(KSIRK_LOG) << "sending DisplayFightResult";
  m_automaton->sendMessage(buffer3,DisplayFightResult);

  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  
  if ((NKD != 0)&&(NKA != 0)) stream2 << quint32(2);
  else if (NKA != 0) stream2 << quint32(0);
  else if (NKD != 0) stream2 << quint32(1);
  m_automaton->sendMessage(buffer2,AnimExplosion);

  //qCDebug(KSIRK_LOG)<< "A1:"<< A1<<", A2: " <<A2 <<"A3:" << A3;
  //qCDebug(KSIRK_LOG)<< "D1:"<< D1<<", D2: " <<D2;
  // if arena is displayed, update the arena countries too
  if (currentWidgetType() == Arena) {
    arena()->countryAttack()->decrNbArmies(NKA);
    arena()->countryDefense()->decrNbArmies(NKD);
  }
  m_secondCountry-> owner()-> setNbDefense(0);
}



/**
  * Reimplementation of the inherited function called when a window close event arise
  */
bool KGameWindow::queryClose()
{
  qCDebug(KSIRK_LOG);

  if ((m_automaton->state() == GameAutomaton::INIT) || (m_automaton->state() ==  GameAutomaton::INTERLUDE))
  {
    switch ( KMessageBox::warningYesNo( this, i18n("Do you want to quit the game?")) )
    {
    case KMessageBox::Yes :
        break;
    default:
        return false;
    }
  }
  else
  {
    switch ( KMessageBox::warningYesNoCancel( this, i18n("Before you quit, do you want to save your game?")) )
    {
    case KMessageBox::Yes :
        slotSaveGame();
        break;
    case KMessageBox::No :
        break;
    default: // cancel
        return false;
    }
  }
//   hide();
  disconnect(&m_timer,SIGNAL(timeout()),this,SLOT(evenementTimer()));
  disconnectMouse();
  m_mouseLocalisation = 0;
  m_automaton->setGameStatus(KGame::End);
  
/*  if (m_theWorld != 0)
  {
    delete m_theWorld;
    m_theWorld = 0;
  }
  while (!m_automaton->playerList()->isEmpty())
  {
    delete m_automaton->playerList()->takeFirst();
  }
  delete m_automaton; m_automaton = 0;*/
  KSharedConfig::openConfig()->sync();
  return true;
}

bool KGameWindow::actionOpenGame()
{
  qCDebug(KSIRK_LOG) << "KGameWindow::actionOpenGame";

  QString fileName = QFileDialog::getOpenFileName(this, i18n("KsirK - Load Game"), QString(), "*.xml");
  if (!fileName.isEmpty())
  {
    m_fileName = fileName;
    m_automaton->setGameStatus(KGame::End);
    m_waitedPlayers.clear();
    qCDebug(KSIRK_LOG) << "KGameWindow::actionOpenGame loader";
    Ksirk::SaveLoad::GameXmlLoader loader(fileName, *this, m_waitedPlayers);
    qCDebug(KSIRK_LOG) << "KGameWindow::actionOpenGame loading done";
    for (unsigned int i = 0; i < m_theWorld->getNbCountries(); i++)
    {
      Country* country = m_theWorld-> getCountries().at(i);
      qCDebug(KSIRK_LOG) << "Adding sprites to " << country->name();
      country-> createArmiesSprites();
      Player *player = country-> owner();
      if (player)
      {
        country-> flag(player->flagFileName(), m_backGnd_world);
      }
    }
    m_backGnd_world->hide();
    m_backGnd_world->show();
    if (m_waitedPlayers.empty())
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      m_automaton->sendMessage(buffer,StartGame);
      m_automaton->sendMessage(buffer,ClearHighlighting);
      m_frame->setFocus();
      qCDebug(KSIRK_LOG) << "KGameWindow::actionOpenGame false1";
      m_frame->setArenaOptionEnabled(true);
      reduceChat();

      return false;
    }
    else
    {
      KMessageParts messageParts;
      messageParts << I18N_NOOP("Waiting for the connection of %1 network players.") << QString::number(m_waitedPlayers.size());
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
      qCDebug(KSIRK_LOG) << "KGameWindow::actionOpenGame true";
      unreduceChat();
      m_frame->setArenaOptionEnabled(false);
      return true;
    }
  }
  qCDebug(KSIRK_LOG) << "KGameWindow::actionOpenGame false2";
  return false;
}

void KGameWindow::displayRecyclingButtons()
{
  qCDebug(KSIRK_LOG);
  if (m_automaton->allLocalPlayersComputer())
  {
//     qCDebug(KSIRK_LOG) << "There is only computer local players";
    PlayersArray::iterator it = m_automaton->playerList()->begin();
    PlayersArray::iterator it_end = m_automaton->playerList()->end();
    for (; it != it_end; it++)
    {
//       qCDebug(KSIRK_LOG) << "Looking at player " << (*it)->name() 
//               << ". is AI  : " << ((Player*)(*it))->isAI() 
//               << ". isRunning: " << ((static_cast<AIPlayer *>(*it))-> isRunning())
//               << ". virtual: " << (*it)->isVirtual() 
//                      ;
      if ( ((Player*)(*it))->isAI() 
           && (!(static_cast<AIPlayer *>(*it))-> isRunning())
        && (!(*it)->isVirtual()) ) 
      {
//         qCDebug(KSIRK_LOG) << "Starting computer player " << (*it)->name();
        (static_cast<AIPlayer *>(*it))-> start();
        break;
      }
    }
  }
  else
  {
    m_rightDock->show();
  }
  m_nextPlayerAction->setEnabled(false);
}

void KGameWindow::clearHighlighting()
{
  qCDebug(KSIRK_LOG);
  if (m_firstCountry != 0)
  {
    m_firstCountry->releaseHighlightingLock();
    m_firstCountry->clearHighlighting();
    m_firstCountry = 0;
  }
  if (m_secondCountry != 0)
  {
    m_secondCountry->releaseHighlightingLock();
    m_secondCountry->clearHighlighting();
    m_secondCountry = 0;
  }

  if (currentPlayer() && currentPlayer()-> isAI() && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
    m_nextPlayerAction->setEnabled(false);
  }
  else if (currentPlayer() && !currentPlayer()->isVirtual())
  {
    slotContextualHelp();
    m_nextPlayerAction->setEnabled(true);
  }
  else
  {
    m_nextPlayerAction->setEnabled(false);
  }
}

QString KGameWindow::defenseLabel()
{
  if (firstCountry() && firstCountry()->owner() && secondCountry())
    return i18np("<font color=\"red\">%2</font> attacks you from <font color=\"red\">%3</font> with 1 army!<br> How do you want to defend <font color=\"blue\">%4</font>?",
                        "<font color=\"red\">%2</font> attacks you from <font color=\"red\">%3</font> with %1 armies!<br> How do you want to defend <font color=\"blue\">%4</font>?",
                        QString::number(this->firstCountry()->owner()->getNbAttack()),
                        this->firstCountry()->owner()->name(),
                        this->firstCountry()->name(),
                        this->secondCountry()->name());
  else
    return "";
}

void KGameWindow::createDefenseDialog()
{
  qCDebug(KSIRK_LOG);
  // Create Window Dialog
  m_defenseDialog = new QDialog ();

  QWidget* widget = new QWidget(m_defenseDialog);
  QGridLayout * mainLayout = new QGridLayout(widget);

  // Create the differents layout for buttons and label
  QGridLayout * bottomLayout = new QGridLayout();
  QGridLayout * topLayout = new QGridLayout();

  // Create and add the main Layout
  widget->setLayout(mainLayout);
  mainLayout->addLayout(bottomLayout, 1, 0, Qt::AlignCenter);
  mainLayout->addLayout(topLayout, 0, 0, Qt::AlignCenter);
  
  // Creat buttons and label of defense
  QPushButton * def1 = new QPushButton (i18n("Defend 1"));
  QPushButton * def2 = new QPushButton (i18n("Defend 2"));
  QPushButton * defAuto = new QPushButton (i18n("Defend-Auto"));

  m_labDef = new QLabel ();
  m_labDef->setText(defenseLabel());

  // Add icons on buttons
  QString skin = m_automaton->game()->theWorld()->skin();
  QString imageFileName;

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/defendOne.png");
  def1->setIcon(QIcon(imageFileName));
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/defendTwo.png");
  def2->setIcon(QIcon(imageFileName));
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/attackAuto.png");
  defAuto->setIcon(QIcon(imageFileName));


  // Disable Defend 2 when attack with 1 army or defender have only 1 army
  if (this->firstCountry()->owner()->getNbAttack() == 1)
     def2->setEnabled(false);

  // Add buttons and layout
  bottomLayout->addWidget(def1,0,0);
  bottomLayout->addWidget(def2,0,1);
  bottomLayout->addWidget(defAuto,0,2);
  topLayout->addWidget(m_labDef, 0, 0);

  connect(def1, SIGNAL(clicked()), this, SLOT(slotWindowDef1()));
  connect(def2, SIGNAL(clicked()), this, SLOT(slotWindowDef2()));
  connect(defAuto, SIGNAL(clicked()), this, SLOT(slotDefAuto()));

  QVBoxLayout *dialogLayout = new QVBoxLayout(m_defenseDialog);
  dialogLayout->addWidget(widget);
}

void KGameWindow::displayDefenseWindow()
{
  qCDebug(KSIRK_LOG);
  if (m_defenseDialog == 0)
    createDefenseDialog();
  else
    m_labDef->setText(defenseLabel());
  m_defenseDialog->exec();
}

void KGameWindow::startLocalCurrentAI()
{
  qCDebug(KSIRK_LOG);
  if (currentPlayer() && currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning())
      (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
}

void KGameWindow::setBarFlagButton(const Player* player)
{
//   qCDebug(KSIRK_LOG) << "KGameWindow::setBarFlagButton";

  if (player == 0)
  {
    if (currentPlayer() 
        && currentPlayer()-> getFlag())
    {
      if (!m_goalAction->isVisible())
        m_goalAction->setVisible(true);
      m_goalAction-> setIcon(QIcon(currentPlayer()->getFlag()-> image(0)));
      m_goalAction-> setIconText(i18n("Goal"));
      m_barFlag-> setPixmap(currentPlayer()->getFlag()-> image(0));
      m_barFlag->show();
    }
  }
  else
  {
    if (player-> getFlag())
    {
      if (!m_goalAction->isVisible())
        m_goalAction->setVisible(true);
      m_goalAction-> setIcon(QIcon(player-> getFlag()-> image(0)));
      m_goalAction-> setIconText(i18n("Goal"));
      m_barFlag-> setPixmap(player->getFlag()-> image(0));
      m_barFlag->show();
    }
  }
  m_frame->setFocus();
}

bool KGameWindow::finishSetupPlayers()
{
  qCDebug(KSIRK_LOG);
  if (!(m_automaton->playerList()->isEmpty()))
  {
    m_automaton->playerList()->clear();
    m_automaton->currentPlayer(0);
    qCDebug(KSIRK_LOG) << "  playerList size = " << m_automaton->playerList()->count();
  }
  theWorld()->reset();
  
  QMap< QString, QString > nations = nationsList();
  if (!(m_automaton->playerList()->isEmpty()))
  {
    m_automaton->playerList()->clear();
  }
  qCDebug(KSIRK_LOG) << "newPlayersNumber = " << m_newGameSetup->players().size();
  unsigned int nbAvailArmies = (unsigned int)(m_theWorld->getNbCountries() * 2.5 / m_newGameSetup->players().size());
  qCDebug(KSIRK_LOG) << "nbAvailArmies = " << nbAvailArmies << " ; nb countries = " << m_theWorld->getNbCountries();
  QString nomEntre = "";
  QString password = "";
  QString nationName = "";
  m_newPlayerWidget->init(m_automaton,m_theWorld,(int)1,nomEntre,password,false,nations,nationName, m_newGameSetup);
  m_centralWidget->setCurrentIndex(NEWPLAYER_INDEX);
  // Players names
  QString mes = "";
  if (m_newGameSetup->networkGameType() != GameAutomaton::None)
  {
    m_frame->setArenaOptionEnabled(false);
    unreduceChat();
    qCDebug(KSIRK_LOG) << "In setupPlayers: networkGame";
    KMessageParts messageParts;
    messageParts << I18N_NOOP("Waiting for %1 players to connect")
      << QString::number(m_newGameSetup->nbNetworkPlayers());
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  }
  else
  {
    m_frame->setArenaOptionEnabled(true);
    reduceChat();
  }
  m_frame->setFocus();
  return true;
}

bool KGameWindow::setupOnePlayer()
{
  qCDebug(KSIRK_LOG) << "KGameWindow::setupOnePlayer";
  
  qCDebug(KSIRK_LOG) << "  building the list of available nations";
  QMap< QString, QString > nations = nationsList();
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    Player* player = (Player*)(*it);
    qCDebug(KSIRK_LOG) << "    removing nation of player " << player-> name();
    /// Don't understand why if using Player::getNation below, the method is
    /// not executed (get 0) but if using the same named myNation, it works...
    /// Ideas anybody ????
    Nationality* nation = player-> getNation();
    qCDebug(KSIRK_LOG) << "    got player nation " << nation;
    QString nationName = nation->name();
    QMap<QString,QString>::iterator nationsIt;
    nationsIt = nations.find(nationName);
    if (nationsIt !=  nations.end())
    {
      nations.erase(nationsIt);
    }
  }
  qCDebug(KSIRK_LOG) << "  number of available nations: " << nations.size();
  unsigned int nbAvailArmies = (unsigned int)(m_theWorld->getNbCountries() * 2.5 / (m_automaton->nbPlayers()));
  qCDebug(KSIRK_LOG) << "KGameWindow::setupOnePlayer: nbAvailArmies = " << nbAvailArmies << " ; nb countries = " << m_theWorld->getNbCountries();
  // Players names
  QString mes = "";
  QString nationName;
    
  QString nomEntre = "";
  QString password;
  bool computer=false;

  m_newPlayerWidget->init(m_automaton, m_theWorld, 1, nomEntre, password, computer, nations, nationName, m_newGameSetup);
  m_centralWidget->setCurrentIndex(NEWPLAYER_INDEX);
  return true;
}

bool KGameWindow::setupOneWaitedPlayer()
{
  qCDebug(KSIRK_LOG) << "KGameWindow::setupOneWaitedPlayer";
  
  QString password;
  int result;
  KWaitedPlayerSetupDialog(m_automaton, password, result, this).exec();
  qCDebug(KSIRK_LOG) << "After KWaitedPlayerSetupDialog. number: " << result << ", password: " << password;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << (quint32)result << password;
  m_automaton->sendMessage(buffer,ValidateWaitedPlayerPassword);
  return true;
}

bool KGameWindow::createWaitedPlayer(quint32 waitedPlayerId)
{
  qCDebug(KSIRK_LOG) << "KGameWindow::createWaitedPlayer";
  
  PlayerMatrix& pm = m_waitedPlayers[waitedPlayerId];
  Player* player = addPlayer(pm.name, pm.nbAvailArmies,
                              pm.nbCountries, pm.nation,
                              pm.isAI, pm.password,
                              pm.nbAttack, pm.nbDefense);

  player->goal(pm.goal);
  QList<QString>::iterator it, it_end;
  it = pm.countries.begin(); it_end = pm.countries.end();
  for (; it != it_end; it++)
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << (*it) << pm.name;
    m_automaton->sendMessage(buffer,CountryOwner);
  }
  return true;
}

void KGameWindow::distributeArmies()
{
  qCDebug(KSIRK_LOG) << "KGameWindow::distributeArmies";
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    unsigned int nb = nbNewArmies(dynamic_cast<Player*>(*it));
//     qCDebug(KSIRK_LOG) << "    Giving " << nb << " armies to " << static_cast<Player*>(*it)->name();
    dynamic_cast<Player*>(*it)-> setNbAvailArmies(nb, true);
  }
}

int KGameWindow::nbNewArmies(Player *player)
{
//    qCDebug(KSIRK_LOG) << "KGameWindow::nbNewArmies for "  << player-> name();

  unsigned int res = 0;

  for (unsigned int i = 0; i<m_theWorld->getNbCountries(); i++)
      if (m_theWorld-> getCountries().at(i)-> owner() == player) res++;
  res = (res/3) < 3 ? 3 : res/3 ;

  QList<Continent*>& continents = m_theWorld-> getContinents();
  QList<Continent*>::iterator it = continents.begin();
  for (; it != continents.end(); it++)
  {
    Continent* currCont = *it;
    if (currCont-> owner() == player)
    {
//            qCDebug(KSIRK_LOG) << ">>>>>>>>>>> Adding bonus for "  << currCont-> name();
      res += currCont-> getBonus();
    }
  }

  return res;
}

void KGameWindow::changeItem( const QString& text, int id, bool log )
{
  if (id != ID_NO_STATUS_MSG)
  {
      //QT5 statusBar()-> changeItem(text, id);
  }
  if (log)
  {
    KsirkChatItem item;
    item << text;
//     m_chatDlg->addMessage("",text);
// to port
//     m_chatDlg->addSystemMessage   ( "", text  ) ;
  }
}

void KGameWindow::changeItem( KMessageParts& strings, int id, bool log )
{
//  qCDebug(KSIRK_LOG) << "KGameWindow::changeItem(KMessageParts,int, log)" << strings.size() << id << log;
  if (strings.begin() == strings.end())
  {
//     qCDebug(KSIRK_LOG) << "  nothing " << strings.size();
    return;
  }
  bool arguing = false;
  KLocalizedString argument;
  KsirkChatItem item;
  KMessageParts::iterator it, it_end;
  it = strings.begin(); it_end = strings.end();
  if (it.curIsStr())
  {
    if (!it.curStr().isEmpty())
    {
//       qCDebug(KSIRK_LOG) << "setting argument to: '" << it.curStr() << "'";
      argument = ki18n(it.curStr().toUtf8().data());
    }
    else
    {
//       qCDebug(KSIRK_LOG) << "setting argument to empty";
      argument = KLocalizedString();
    }
    arguing = true;
  }
  else if (it.curIsPix())
  {
//     qCDebug(KSIRK_LOG) << "first item is pixmap";
    item << it.curPix();
  }
  
  it++;
  for (; it != it_end; it++)
  {
//     qCDebug(KSIRK_LOG) << "next item";
    if (it.curIsStr())
    {
//       qCDebug(KSIRK_LOG) << "item is: '" << it.curStr() << "'";
      if (arguing)
      {
//         qCDebug(KSIRK_LOG) << "  substituting";
        argument=argument.subs(it.curStr());
      }
      else
      {
//         qCDebug(KSIRK_LOG) << "  assigning new argument";
        if (!it.curStr().isEmpty())
        {
//           qCDebug(KSIRK_LOG) << "setting argument to: '" << it.curStr() << "'";
          argument = ki18n(it.curStr().toUtf8().data());
        }
        else
        {
//           qCDebug(KSIRK_LOG) << "setting argument to empty";
          argument = KLocalizedString();
        }
        arguing = true;
      }
    }
    else
    {
      if (arguing)
      {
//         qCDebug(KSIRK_LOG) << "  storing";
        if (argument.isEmpty())
        {
          item << QString("");
        }
        else
        {
          item << argument.toString();
        }
      }
      item << it.curPix();
      arguing = false;
    }
  }
//   qCDebug(KSIRK_LOG) << "no more items";
  if (arguing)
  {
    if (argument.isEmpty())
    {
      item << QString("");
    }
    else
    {
      item << argument.toString();
    }
  }
  if (log)
  {
    ((KsirkChatModel*)(m_chatDlg->model()))->addMessage(item);
  }
  if (id != ID_NO_STATUS_MSG)
  {
    if (argument.toString() == "(I18N_EMPTY_MESSAGE)")
    {
      qCCritical(KSIRK_LOG) << "received a (I18N_EMPTY_MESSAGE)";
    }
//     qCDebug(KSIRK_LOG) << "  argument: " << argument.toString();
    //QT5 statusBar()-> changeItem(argument.toString(), id);
  }
}

void KGameWindow::broadcastChangeItem(KMessageParts& strings, int id, bool log)
{
  if (strings.empty())
  {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
//   qCDebug(KSIRK_LOG) << "Broadcasting change item, size=" << strings.size();
  stream << (quint32)id << (quint32)log << (quint32)strings.size();
  
  KMessageParts::iterator it, it_end;
  it = strings.begin(); it_end = strings.end();
  if (it != it_end)
  {
/*    // if first element is string, convert it to id 
    if (it.curIsStr())
    {
      qCDebug(KSIRK_LOG) << "Pushing first element: id "<<m_automaton->idForMsg(it.curStr())<<" for '" << it.curStr() << "'";
      stream << (quint32)KMessageParts::StringId << m_automaton->idForMsg(it.curStr());
    }
    else if (it.curIsPix())
    {
      qCDebug(KSIRK_LOG) << "Pushing first element pix";
      stream << (quint32)KMessageParts::Pixmap << it.curPix();
    }
    else
    {
      qCCritical(KSIRK_LOG) << "Unsupported KMessageParts elem type ";
    }
    it++;*/
    for (; it != it_end; it++)
    {
      if (it.curIsStr())
      {
//         qCDebug(KSIRK_LOG) << "Pushing string '" << it.curStr() << "'";
        stream << (quint32)KMessageParts::Text << it.curStr();
      }
      else if (it.curIsPix())
      {
//         qCDebug(KSIRK_LOG) << "Pushing pix";
        stream << (quint32)KMessageParts::Pixmap << it.curPix();
      }
      else
      {
        qCCritical(KSIRK_LOG) << "Unsupported KMessageParts elem type ";
      }
    }
  }
  m_automaton->sendMessage(buffer,ChangeItem);
  changeItem(strings, id, log );
}

void KGameWindow::enterEvent(QEvent* /*ev*/)
{
//   qCDebug(KSIRK_LOG) << "KGameWindow::enterEvent()";
  // Restart the AIs threads
  if ( currentPlayer() )
    if ( (currentPlayer()-> isAI()) && (!currentPlayer()->isVirtual()) && ( !((static_cast<AIPlayer*>(currentPlayer()))-> isRunning())) )
      (static_cast<AIPlayer*>(currentPlayer()))-> start();
          
}

void KGameWindow::leaveEvent(QEvent* /*ev*/)
{
//    qCDebug(KSIRK_LOG) << "KGameWindow::leaveEvent()";
    // Stops the AIs threads
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    if (static_cast<Player*>(*it)-> isAI())
    {
        (static_cast<AIPlayer*>(*it))-> stop();
    }
  }
}


/** Return true if the state of the game is the argument; false otherwise */
bool KGameWindow::isMyState(GameLogic::GameAutomaton::GameState state) const
{
  return (m_automaton->state() == state);
}

/**
  * returns the current state of the game
  */
GameLogic::GameAutomaton::GameState KGameWindow::getState() const
{
  return m_automaton->state();
}

void KGameWindow::moveArmies(Country& src, Country& dest, unsigned int nb)
{
//    qCDebug(KSIRK_LOG) << "KGameWindow::moveArmies()";
  if ((src.owner() == currentPlayer())
      && (dest.owner() == currentPlayer())
      && (src.communicateWith(&dest))
      && (src.nbArmies() > nb) )
  {
    QByteArray bufferSrc;
    QDataStream streamSrc(&bufferSrc, QIODevice::WriteOnly);
    streamSrc << src.name();
    m_automaton->sendMessage(bufferSrc,FirstCountry);
    
    QByteArray bufferDest;
    QDataStream streamDest(&bufferDest, QIODevice::WriteOnly);
    streamDest << src.name();
    m_automaton->sendMessage(bufferDest,SecondCountry);
    
    int toMove = nb;
    while ( toMove >= 10 )
    {
      toMove -= 10;
      slotInvade10();
    }
    while ( toMove >= 5 )
    {
      toMove -= 5;
      slotInvade5();
    }
    while ( toMove >= 1 )
    {
      toMove -= 1;
      slotInvade1();
    }
  }
//    qCDebug(KSIRK_LOG) << "OUT KGameWindow::moveArmies()";
}

bool KGameWindow::isMoveValid(const QPointF& point)
{
  bool res = false;
  KMessageParts messageParts;
  Country* secondCountry = clickIn(point); 
  if  ( ( m_firstCountry == 0 ) || ( secondCountry == 0 ) )
  {
    messageParts << I18N_NOOP("There is no country here!");
  }
  else if  ( m_firstCountry->owner() != currentPlayer() )
  {
    messageParts << I18N_NOOP("You are not the owner of the first country: %1!") << m_firstCountry->name();
  }
  else if ( secondCountry->owner() != currentPlayer() )
  {
    messageParts << I18N_NOOP("You are not the owner of the second country: %1!") << secondCountry->name();
  }
  else if (m_firstCountry == secondCountry)
  {
    messageParts << I18N_NOOP("You are trying to move armies from %1 to itself!") << m_firstCountry->name();
  }
  else if (!m_firstCountry->communicateWith(secondCountry))
  {
    messageParts
      << I18N_NOOP("%1 is not a neighbour of %2!") 
      << secondCountry-> name() 
      << m_firstCountry-> name();
  }
  else 
  {
    messageParts << I18N_NOOP("Moving armies from %1 to %2.") 
            << m_firstCountry->name() 
            << secondCountry->name();
    res = true;
  }
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  return res;    
}

bool KGameWindow::isFightValid(const QPointF& point)
{
  bool res = false;
  KMessageParts messageParts;
  Country* secondCountry = clickIn(point); 
  if  ( ( m_firstCountry == 0 ) || ( secondCountry == 0 ) )
  {
    qCDebug(KSIRK_LOG) << "There is no country here!";
    messageParts << I18N_NOOP("There is no country here!");
  }
  else if  ( m_firstCountry->owner() != currentPlayer() )
  {
    qCDebug(KSIRK_LOG) << "You are not the owner of the first country: "<< m_firstCountry->name();
    messageParts << I18N_NOOP("You are not the owner of the first country: %1!")
            << m_firstCountry->name();
  }
  else if ( secondCountry->owner() == currentPlayer() )
  {
    qCDebug(KSIRK_LOG) << "You are the owner of the second country: " << secondCountry->name();
    messageParts << I18N_NOOP("You are the owner of the second country: %1!") << secondCountry->name();
  }
  else if (m_firstCountry == secondCountry)
  {
    qCDebug(KSIRK_LOG) <<"You are trying to move armies from "<<m_firstCountry->name()<<" to itself ";
    messageParts << I18N_NOOP("You are trying to move armies from %1 to itself!") << m_firstCountry->name();
  }
  else if (!m_firstCountry->communicateWith(secondCountry))
  {
    qCDebug(KSIRK_LOG) << secondCountry-> name() << "is not a neighbour of " << secondCountry-> name();
    messageParts 
      << I18N_NOOP("%1 is not a neighbour of %2!") 
      << secondCountry-> name() 
      << m_firstCountry-> name();
  }
  else 
  {
    qCDebug(KSIRK_LOG) << "Ready to fight !";
    messageParts << I18N_NOOP("Ready to fight!");
    res = true;
  }
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  return res;    
}

int KGameWindow::setCurrentPlayerToFirst()
{
  if (currentPlayer() && currentPlayer()->isAI() 
                      && (static_cast<AIPlayer *>(currentPlayer())->isRunning()))
  {
        static_cast<AIPlayer *>(currentPlayer())->stop();
  }
  m_automaton->currentPlayer((Player*)(*m_automaton->playerList()->begin()));

/*  if (currentPlayer() && currentPlayer()->isAI() && !currentPlayer()->isVirtual()
      && !(static_cast<AIPlayer *>(currentPlayer())->isRunning()))
  {
          static_cast<AIPlayer *>(currentPlayer())->start();
          qCDebug(KSIRK_LOG) <<"setCurrentPlayerToFirst : step 3";
  }*/
  m_frame->setFocus();
  return 0;
}

int KGameWindow::setCurrentPlayerToNext(bool restartRunningAIs)
{
  qCDebug(KSIRK_LOG) << restartRunningAIs;
  m_rightDock->hide();
  int looped(0);
//   qCDebug(KSIRK_LOG) << "KGameWindow::setCurrentPlayerToNext()";
  if ( currentPlayer() && ( currentPlayer()-> isAI())  && ( static_cast<AIPlayer *>(currentPlayer())-> isRunning() ) )
      static_cast<AIPlayer *>(currentPlayer())-> stop();
  
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (;it != it_end; it++)
  {
    if (*it == currentPlayer())
    {
      it++;
      break;
    }
  }
  if (it == it_end)
  {
      setCurrentPlayerToFirst();
      looped = 1;
  }
  else
  {
    m_automaton->currentPlayer((Player*)(*it));
  }

  if ( restartRunningAIs && currentPlayer() && currentPlayer()-> isAI() && (!currentPlayer()->isVirtual()) && !looped)
  {
    if ( ! (static_cast< AIPlayer* >(currentPlayer())-> isRunning()))
    {
      static_cast< AIPlayer* >(currentPlayer())-> start();
    }
  }

  if ( currentPlayer()->isAI() || currentPlayer()->isVirtual() )
  {
    m_nextPlayerAction->setEnabled(false);
  }
  else
  {
    m_nextPlayerAction->setEnabled(true);
  }
  
  qCDebug(KSIRK_LOG) << "New current player is " << currentPlayer()->name() << " ; return value is " << looped;
  return looped;
}

bool KGameWindow::terminateAttackSequence()
{
  if (m_firstCountry != 0)
    m_firstCountry->clearHighlighting();
  if (m_secondCountry != 0)
    m_secondCountry->clearHighlighting();
  if (m_animFighters != 0)
    m_animFighters->hideAndRemoveAll();
  return attackEnd();
}

bool KGameWindow::attacker(const QPointF& point)
{
  qCDebug(KSIRK_LOG);
  Country* clickedCountry = clickIn(point);
  KMessageParts messageParts;
  if (clickedCountry == 0)
  {
    messageParts << I18N_NOOP("<font color=\"orange\">No country here!</font>");
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    clearHighlighting();
    return false;
  }

  if (clickedCountry-> owner() != currentPlayer())
  {
    messageParts << I18N_NOOP("<font color=\"orange\">You are not the owner of %1!</font>")  << clickedCountry-> name();
    clearHighlighting();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return false;
  }
  else if (clickedCountry-> nbArmies() <= currentPlayer()-> getNbAttack())
  {
    messageParts << I18N_NOOP("<font color=\"orange\">There is only %1 armies in %2!</font>") 
      << QString::number(clickedCountry-> nbArmies())
      << clickedCountry-> name();
    clearHighlighting();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return false;
  }
  else
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << clickedCountry->name();
    m_automaton->sendMessage(buffer,FirstCountry);
    QByteArray buffer2;
    QDataStream stream2(&buffer2, QIODevice::WriteOnly);
    stream2 << "";
    m_automaton->sendMessage(buffer2,SecondCountry);

    QPixmap pm = currentPlayer()->getFlag()->image(0);
    /*messageParts
      << pm
      << I18N_NOOP("%1 attacks from %2 with <font color=\"red\">%3 armies</font>") 
      << currentPlayer()-> name()
      << clickedCountry-> name()
      << QString::number(currentPlayer()-> getNbAttack());
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);*/
    return true;
  }
}

unsigned int KGameWindow::attacked(const QPointF& point)
{
  qCDebug(KSIRK_LOG) << point << (void*)m_firstCountry << (void*)m_secondCountry;
  //if (currentPlayer()-> isAI()) return 3;
  // executed on the admin side only
  if (!m_automaton->isAdmin()) return 3;

  unsigned int res = 0;
  //Country* secondCountry = clickIn(point);
  //m_secondCountry = secondCountry;
  KMessageParts messageParts;

//   qCDebug(KSIRK_LOG) << "2nd country is now set";
  if ( (m_firstCountry == NULL) || (m_secondCountry == NULL)
          || (m_firstCountry-> owner() != currentPlayer()) )
  {
    qCDebug(KSIRK_LOG) << ("Nothing to attack !");
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
  else if (!m_secondCountry-> owner())
  {
    qCDebug(KSIRK_LOG) << ("Invalid attacked country.");
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
/*  else if (!m_secondCountry-> owner()->isVirtual())
  {
    // messageParts << I18N_NOOP("Invalid attacked country.");
    return 3;
  }*/
  else if (m_firstCountry == m_secondCountry)
  {
   qCDebug(KSIRK_LOG) << ("You are trying to attack %1 from itself !") << m_firstCountry-> name();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
  else if (!m_firstCountry-> communicateWith(m_secondCountry))
  {
    qCDebug(KSIRK_LOG) << ("%1 is not a neighbour of %2 !") << m_secondCountry-> name() << m_firstCountry-> name();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
  else if (m_firstCountry-> owner() == m_secondCountry-> owner())
  {
    qCDebug(KSIRK_LOG) << ("%1! You cannot attack %2! It is yours!") << currentPlayer()-> name()
           << m_secondCountry-> name();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
  else if (m_firstCountry-> owner() != currentPlayer()) 
  {
    qCDebug(KSIRK_LOG) << ("%1 ! You are not the owner of %2!") << currentPlayer()-> name() << m_firstCountry-> name();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
  else if (m_firstCountry->nbArmies() - currentPlayer()->getNbAttack() < 1)
  {
   qCDebug(KSIRK_LOG)
      << ("%1, you have to keep one army to defend %2.")
      << m_firstCountry->owner()-> name()
      << m_firstCountry-> name();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
  else if (m_secondCountry-> nbArmies() > 1)
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (m_secondCountry != 0)
    {
      stream << m_secondCountry->name();
    }
    else
    {
      stream << QString("");
    }
    m_automaton->sendMessage(buffer,SecondCountry);

    qCDebug(KSIRK_LOG)
        << ("%1, with how many armies do you defend %2 ?")
        << m_secondCountry->owner()-> name()
        << m_secondCountry-> name();
    QByteArray buffer2;
    QDataStream stream2(&buffer2, QIODevice::WriteOnly);
    stream2 << m_secondCountry->owner()->name();
    m_automaton->sendMessage(buffer2,DisplayDefenseButtons);
    res = 1;
  }
  else
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (m_secondCountry != 0)
    {
      stream << m_secondCountry->name();
    }
    else
    {
      stream << QString("");
    }
    m_automaton->sendMessage(buffer,SecondCountry);
    messageParts
      << I18N_NOOP("%1, you defend with the only army you have in %2.") 
      << m_secondCountry->owner()-> name()
      << m_secondCountry-> name();
    res = 2;
  }
  qCDebug(KSIRK_LOG) << "will change item";
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  qCDebug(KSIRK_LOG) << "change item broadcasted; returning " << res;
  return res;
}

bool KGameWindow::firstCountryAt(const QPointF& point)
{
  Country* c = clickIn(point);
  if (c)
  {
    if (c-> owner() == currentPlayer())
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << c->name();
      m_automaton->sendMessage(buffer,FirstCountry);
      return true;
    }
  }
  return false;
}

bool KGameWindow::secondCountryAt(const QPointF& point)
{
  if (clickIn(point))
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << clickIn(point)->name();
    m_automaton->sendMessage(buffer,SecondCountry);
    return true;
  }
  return false;
}

bool KGameWindow::playerPutsArmy(const QPointF& point, bool removable)
{
  qCDebug(KSIRK_LOG) << removable;
  Country* clickedCountry = clickIn(point);

  if (clickedCountry)
  {
    qCDebug(KSIRK_LOG) << "clickedCountry name=" << clickedCountry->name() ;
    qCDebug(KSIRK_LOG) << "clickedCountry owner=" << clickedCountry-> owner()->name();
    qCDebug(KSIRK_LOG) << "currentPlayer=" << currentPlayer()->name();
    unsigned int nbAvailArmies = currentPlayer()->getNbAvailArmies();
    qCDebug(KSIRK_LOG) << "nbAvailArmies=" << nbAvailArmies;
    if (clickedCountry->owner() == currentPlayer() &&  nbAvailArmies > 0)
    {
      nbAvailArmies--;
      qCDebug(KSIRK_LOG) << "owner new available armies=" << nbAvailArmies;
      currentPlayer()->putArmiesInto(1, theWorld()->indexOfCountry(clickedCountry));
      clickedCountry-> incrNbArmies();
      clickedCountry-> createArmiesSprites();
      QPixmap pm = currentPlayer()->getFlag()->image(0);
      KMessageParts messageParts;
      messageParts 
        << pm 
        << I18N_NOOP("%1: %2 armies to place") 
        << currentPlayer()-> name()
        << QString::number(nbAvailArmies);
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);

      getRightDialog()->updateRecycleDetails(clickedCountry,false,nbAvailArmies);

      if (m_automaton->isAdmin())
      {
        m_automaton->checkGoal();
      }
    }
  }
  return false;
}

bool KGameWindow::playerPutsInitialArmy(const QPointF& point)
{
  {
    qCDebug(KSIRK_LOG) << point;
    Country* clickedCountry = clickIn(point);
    
    if ( (clickedCountry) )
    {
      qCDebug(KSIRK_LOG) << "clickedCountry name=" << clickedCountry->name() ;
      qCDebug(KSIRK_LOG) << "clickedCountry owner=" << clickedCountry-> owner()->name();
      qCDebug(KSIRK_LOG) << "clickedCountry had armies=" << clickedCountry-> nbArmies();
      qCDebug(KSIRK_LOG) << "currentPlayer=" << currentPlayer()->name();
      qCDebug(KSIRK_LOG) << "nbAvailArmies=" << currentPlayer()->getNbAvailArmies();
      
      if (
           (clickedCountry-> owner() == currentPlayer()) &&
           (((GameLogic::Player*)currentPlayer())-> getNbAvailArmies() > 0))
      {
        unsigned int currentAvailArmiesNumber = ((GameLogic::Player*)currentPlayer())-> getNbAvailArmies() - 1;
        bool last = (currentAvailArmiesNumber == 0);
        currentPlayer()->putArmiesInto(1, theWorld()->indexOfCountry(clickedCountry));
        qCDebug(KSIRK_LOG) << "owner new available armies=" << currentAvailArmiesNumber;
        clickedCountry-> incrNbArmies();
        clickedCountry-> createArmiesSprites();

        if ( last )
        {
          if (m_automaton->isAdmin())
          {
            PlayersArray::iterator it = m_automaton->playerList()->begin();
            PlayersArray::iterator it_end = m_automaton->playerList()->end();
            for (;it != it_end; it++)
            {
              if (*it == currentPlayer())
              {
                it++;
                break;
              }
            }
            if (it != it_end)
            {
              QPixmap pm= ((Player*)(*it))->getFlag()->image(0);

             /* KMessageParts messageParts;
              messageParts
                << pm
                << I18N_NOOP("%1: %2 armies to place") << ((Player*)(*it))-> name()
                << QString::number(((Player*)(*it))-> getNbAvailArmies());
              broadcastChangeItem(messageParts, ID_STATUS_MSG2);*/
/*            m_nbAvailArmies = ((Player*)(*it))-> getNbAvailArmies();
              QByteArray buffer;
              QDataStream stream(&buffer, QIODevice::WriteOnly);
              stream << (quint32)m_nbAvailArmies;
              m_automaton->sendMessage(buffer,KGameWinAvailArmies);*/
              getRightDialog()->close();

              QByteArray buffer2;
              QDataStream stream2(&buffer2, QIODevice::WriteOnly);
              stream2 << ((GameLogic::Player*)(*it))->name();
              stream2 << (quint32) ((GameLogic::Player*)(*it))->getNbAvailArmies();
              qCDebug(KSIRK_LOG) << "sending DisplayRecycleDetails "
                << ((Player*)(*it))->name() << (quint32) ((GameLogic::Player*)(*it))->getNbAvailArmies()
                << " at " << __FILE__ << ", line " << __LINE__;
              m_automaton->sendMessage(buffer2,DisplayRecycleDetails);
            }
            int ret = setCurrentPlayerToNext();
            setContextualHelpActionEnabled(getState(), currentPlayer() && currentPlayer()->isAI());
            return ret;
          }
          else
          {
            return false;
          }
        }
        else
        {
          getRightDialog()->updateRecycleDetails(clickedCountry,false,(quint32) ((GameLogic::Player*)(currentPlayer()))->getNbAvailArmies());
          QPixmap pm = currentPlayer()->getFlag()->image(0);
          KMessageParts messageParts;
          messageParts << pm << I18N_NOOP("%1: %2 armies to place") 
            << currentPlayer()-> name()
            << QString::number((quint32) ((GameLogic::Player*)(currentPlayer()))->getNbAvailArmies());
          changeItem(messageParts, ID_STATUS_MSG2, false);
        }
      }
    }
    return false;
  }
}

bool KGameWindow::playerRemovesArmy(const QPointF& point)
{
  qCDebug(KSIRK_LOG) << point;
  
  Country *clickedCountry = clickIn(point);
  qCDebug(KSIRK_LOG) << "  currentPlayer=" << currentPlayer()->name();
  if (clickedCountry == 0)
  {
    return false;
  }
  qCDebug(KSIRK_LOG) << "  owner=" << clickedCountry-> owner()->name();
  qCDebug(KSIRK_LOG) << "  nbArmies=" << clickedCountry->nbArmies();
  qCDebug(KSIRK_LOG) << "  canRemoveArmiesFrom=" << clickedCountry-> owner()->canRemoveArmiesFrom(1, theWorld()->indexOfCountry(clickedCountry) );
  if ( clickedCountry
      && ( clickedCountry-> owner() == currentPlayer() )
      && ( clickedCountry-> nbArmies() > 1)
      && ( clickedCountry-> owner()->canRemoveArmiesFrom(1, theWorld()->indexOfCountry(clickedCountry) ) )
  )
  {
    clickedCountry-> owner()->removeArmiesFrom(1, theWorld()->indexOfCountry(clickedCountry) );
    unsigned int newNbAvailArmies = currentPlayer()-> getNbAvailArmies() /*+ 1*/;

    if ( m_automaton->isAdmin() )
    {
      QPixmap pm = currentPlayer()->getFlag()->image(0);
      KMessageParts messageParts;
      messageParts <<pm<< I18N_NOOP("%1: %2 armies to place") << currentPlayer()-> name() 
        << QString::number(newNbAvailArmies);
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    }
    clickedCountry-> decrNbArmies();
    clickedCountry-> createArmiesSprites();

    getRightDialog()->updateRecycleDetails(clickedCountry,false,newNbAvailArmies);
    return true;
  }
  return false;
}

/**
  * @brief setups window for recycling
  */
void KGameWindow::initRecycling()
{
  qCDebug(KSIRK_LOG) << "Initiating recycling";
  m_nextPlayerAction->setEnabled(false);

  setCurrentPlayerToFirst();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << quint32(0);
  m_automaton->sendMessage(buffer,DisplayRecyclingButtons);
  
  KMessageParts messageParts;
  messageParts << I18N_NOOP("Exchange armies again or continue?");
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
}

void KGameWindow::clear()
{
  m_nbMovedArmies = 0;
  m_firstCountry = m_secondCountry = 0;
  NKD = NKA = 0;
  if (m_message !=0)
  {
    delete m_message;
  }
  m_message = 0;
}

bool KGameWindow::nextPlayerRecycling()
{
  qCDebug(KSIRK_LOG);
  m_nextPlayerAction->setEnabled(false);
  if ( currentPlayer() && currentPlayer()-> getNbAvailArmies() > 0)
  {
    qCDebug(KSIRK_LOG) << "You must distribute all your armies";
    if (!currentPlayer()->isVirtual() && !currentPlayer()->isAI())
    {
      KMessageBox::sorry(0, i18n("You must distribute\nall your armies"), i18n("KsirK"));
    }
    return false;
  }
  else
  {
    m_rightDock->hide();
    if (currentPlayer() && currentPlayer()-> isAI() && (!currentPlayer()->isVirtual()))
    {
      if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
      m_nextPlayerAction->setEnabled(false);
    }
    else if (currentPlayer() && !currentPlayer()->isVirtual())
    {
      m_nextPlayerAction->setEnabled(true);
    }
    else
    {
      m_nextPlayerAction->setEnabled(false);
    }
    return true;
  }
}

/** 
  * @return if true next state will be NEWARMIES else it will be WAIT
  */
bool KGameWindow::nextPlayerNormal()
{
  qCDebug(KSIRK_LOG) << " (current is" << currentPlayer()->name()<<")";
  setCurrentPlayerToNext();
  distributeArmies();

  QByteArray buffer;
  m_automaton->sendMessage(buffer,ShowArmiesToPlace);

  clear();
  QByteArray buffer2;
  m_automaton->sendMessage(buffer2,StartLocalCurrentAI);
  getRightDialog()->close();

  QByteArray buffer3;
  QDataStream stream3(&buffer3, QIODevice::WriteOnly);
  stream3 << currentPlayer()-> name();
  stream3 << (quint32)nbNewArmies(currentPlayer());
  qCDebug(KSIRK_LOG) << "sending DisplayRecycleDetails "
      << currentPlayer()->name() << nbNewArmies(currentPlayer())
      << " at " << __FILE__ << ", line " << __LINE__;
  m_automaton->sendMessage(buffer3,DisplayRecycleDetails);
  return true;
}

void KGameWindow::centerOnFight()
{
  qCDebug(KSIRK_LOG);

  qreal aj=m_rightDialog->width();    //get the width of the right widget
  qreal ay=m_chatDlg->height();      //get the height of the bottom widget

//Larg
  if (m_firstCountry==0 || m_secondCountry==0)
  {
    qCCritical(KSIRK_LOG) << "countries should not be null ("<<(void*)m_firstCountry<<","<<(void*)m_secondCountry<<") at "<<__FILE__<<", line "<<__LINE__;
    return;
  }
  qreal larg=((m_secondCountry->centralPoint().x())-(m_firstCountry->centralPoint().x()));
  if (larg<0)
  {
   larg=-larg;    //si negatif alors on remet en positif
  }
//Long
  qreal longu=((m_secondCountry->centralPoint().y())-(m_firstCountry->centralPoint().y()));
  if (longu<0)
  {
   longu=-longu;    //si negatif alors on remet en positif
  }
//Point NordOuest
  qreal minx=m_secondCountry->centralPoint().x();  
  qreal miny=m_secondCountry->centralPoint().y();
  if (minx>m_firstCountry->centralPoint().x())
  {
  minx=m_firstCountry->centralPoint().x();
  }
  if (miny>m_firstCountry->centralPoint().y())
  {
  miny=m_firstCountry->centralPoint().y();
  }

  QSizeF size (larg,longu);   //creation de la size  (2x la largeur entre les deux pays)
  QPointF NO (minx,miny);  //creation du point Nord Ouest
  QRectF rect(NO,size);    //creation du rect
  m_frame->ensureVisible(rect);

// centering on the middle point
  qreal xx=((m_secondCountry->centralPoint().x())+(m_firstCountry->centralPoint().x()))/2;
  if (xx<0)
  {
   xx=-xx;    //si negatif alors on remet en positif
  }
  qreal yy=((m_secondCountry->centralPoint().y())+(m_firstCountry->centralPoint().y()))/2;
  if (yy<0)
  {
   yy=-yy;    //si negatif alors on remet en positif
  }

  QPointF mid (xx,yy);
  m_frame->centerOn(mid);      //center on the point

  m_frame->translate(-aj/2,-ay/2);  //translate to center perfectly
//end Benjamin M.


}
void KGameWindow::attack(unsigned int nb)
{  
  centerOnFight();        //center the view on the fight Benj
  
  currentPlayer()-> setNbAttack(nb);
  /*KMessageParts messageParts;
  messageParts << I18N_NOOP("Attack with %1 armies: designate the belligerants") 
    << QString::number(nb);
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);*/
  /*
  showMessage(i18n("To attack, press the mouse button in the attacking country<br>and then <b>drag and drop</b> on its neighbour your want to attack."), 5);
  */
}

void KGameWindow::defense(unsigned int nb)
{
  qCDebug(KSIRK_LOG);

  if (!m_firstCountry) // anything left to do?
     return;

  m_secondCountry-> owner()-> setNbDefense(nb);
  
  QPixmap pmA = m_firstCountry-> owner()->getFlag()->image(0);
  QPixmap pmD = m_secondCountry-> owner()->getFlag()->image(0);

  KMessageParts messageParts;
  messageParts << I18N_NOOP("Battle ongoing.");
//     << I18N_NOOP("Battle between <font color=\"red\">%1</font> (")
//     << m_firstCountry-> name()
//     << pmA
//     << I18N_NOOP("%1) <font color=\"red\">with %2 armies</font> and <font color=\"blue\">%3</font> (")
//     << m_firstCountry->owner()->name()
//     << QString::number(currentPlayer()-> getNbAttack())
//     << m_secondCountry-> name()
//     << pmD
//     << I18N_NOOP("%1) <font color=\"blue\">with %2 armies</font>.") 
//     << m_secondCountry->owner()->name()
//     << QString::number(nb);

  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);

  if (m_firstCountry-> owner() && m_firstCountry-> owner()-> getFlag())
  {
    if (!m_goalAction->isVisible())
      m_goalAction->setVisible(true);
    m_goalAction-> setIcon(QIcon(m_firstCountry-> owner()->getFlag()-> image(0)));
    m_goalAction-> setIconText(i18n("Goal"));
    m_barFlag-> setPixmap(m_firstCountry-> owner()->getFlag()-> image(0));
  }
  if (m_automaton->isAdmin())
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,InitCombatMovement);
    m_automaton->state(GameLogic::GameAutomaton::FIGHT_BRING);
  }
}

int KGameWindow::nbMovedArmies()
{
  return m_nbMovedArmies;
}

void KGameWindow::incrNbMovedArmies(unsigned int nb)
{
  m_nbMovedArmies += nb;
}

void KGameWindow::decrNbMovedArmies(unsigned int nb)
{
  m_nbMovedArmies -= nb;
}

bool KGameWindow::invade(unsigned int nb )
{
  if (m_firstCountry==0 || m_secondCountry==0)
  {
    qCDebug(KSIRK_LOG) << "invade("<<nb<<") returns " << false;
    return false;
  }
  bool res = initArmiesMovement(nb, m_firstCountry, m_secondCountry);
  qCDebug(KSIRK_LOG) << "invade("<<nb<<") returns " << res;
  return res;
}

AnimSprite* KGameWindow::simultaneousAttack(int nb, FightType state)
{
  qCDebug(KSIRK_LOG) << nb << state << relativePosInArenaAttack << relativePosInArenaDefense;
  AnimSprite* res;

  if (state == Attack)
  {
    QPointF pointAttaquant(0,0);
    QPointF pointDefenseur(0,0);
    determinePointArriveeForArena(relativePosInArenaAttack, pointAttaquant,pointDefenseur);

    qCDebug(KSIRK_LOG) << "****point att****" << pointAttaquant;

    qCDebug(KSIRK_LOG) << "****SIMULTANEOUS ATTACK****" << pointAttaquant;
    res = initArmiesMultipleCombat(nb, firstCountry(), secondCountry(), pointAttaquant);

    relativePosInArenaAttack++;
  }
  else // Defense
  {
    QPointF pointAttaquant(0,0);
    QPointF pointDefenseur(0,0);
    determinePointArriveeForArena(/*secondCountry(), firstCountry(),*/
      relativePosInArenaDefense, pointDefenseur, pointAttaquant);

    qCDebug(KSIRK_LOG) << "****point def****" << pointAttaquant;

    qCDebug(KSIRK_LOG) << "****SIMULTANEOUS DEFENSE****" << pointDefenseur;
    res = initArmiesMultipleCombat(nb, secondCountry(), secondCountry(), pointDefenseur);

    relativePosInArenaDefense++;
  }
  qCDebug(KSIRK_LOG) << (void*)res;
  return res;
}


bool KGameWindow::retreat(unsigned int nb)
{
  bool res;
  if (m_nbMovedArmies >= int(nb))
  {
    res = initArmiesMovement(nb, m_secondCountry, m_firstCountry);
  }
  else
  {
    res = false;
  }
  qCDebug(KSIRK_LOG) << "retreat("<<nb<<") returns " << res;
  return res;
}

void KGameWindow::invasionFinished()
{
  qCDebug(KSIRK_LOG);
  clearHighlighting();
 //KMessageParts messageParts;

  QPixmap pm = currentPlayer()->getFlag()->image(0);
  KMessageParts messageParts;
  messageParts
    << pm 
    << I18N_NOOP("%1, it is up to you.") << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
}
        
void KGameWindow::shiftFinished()
{
  clearHighlighting();
  QPixmap pm = currentPlayer()->getFlag()->image(0);
  KMessageParts messageParts;
  messageParts 
    << pm 
    << I18N_NOOP("%1, it is up to you.") << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
  slotNextPlayer();
}

void KGameWindow::cancelAction()
{
  qCDebug(KSIRK_LOG) << "KGameWindow::cancelAction";
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << "";
  m_automaton->sendMessage(buffer,FirstCountry);
  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  stream2 << "";
  m_automaton->sendMessage(buffer2,SecondCountry);

  clearHighlighting();

  KMessageParts messageParts;
  QPixmap pm = currentPlayer()->getFlag()->image(0);
  messageParts
    << pm 
    << I18N_NOOP("%1, it is up to you.") << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
}

void KGameWindow::cancelShiftSource()
{
  if (m_nbMovedArmies < 0)
  {
    m_firstCountry-> decrNbArmies(m_nbMovedArmies);
    m_secondCountry-> incrNbArmies(m_nbMovedArmies);
    m_firstCountry-> createArmiesSprites();
    m_secondCountry-> createArmiesSprites();
    m_nbMovedArmies = 0;
  }
  if (m_nbMovedArmies > 0)
  {
    m_firstCountry-> incrNbArmies(m_nbMovedArmies);
    m_secondCountry-> decrNbArmies(m_nbMovedArmies);
    m_firstCountry-> createArmiesSprites();
    m_secondCountry-> createArmiesSprites();
    m_nbMovedArmies = 0;
  }
}

bool KGameWindow::actionNewGame(GameAutomaton::NetworkGameType socket)
{
  qCDebug(KSIRK_LOG);
  if  ( ( m_automaton->playerList()->count() == 0 ) ||
  ( isMyState(GameLogic::GameAutomaton::GAME_OVER)  ) ||
        (KMessageBox::warningContinueCancel(this,i18n("Do you really want to end your current game and start a new one?"),i18n("New game confirmation"),KStandardGuiItem::yes()) == KMessageBox::Continue ) )

  {
    qCDebug(KSIRK_LOG) << "valid";
    m_automaton->setGameStatus(KGame::End);
    m_reinitializingGame = true;
    m_automaton->removeAllPlayers();
    // Bug 308527. Need to remove all goals on new game.
    m_automaton->removeAllGoals();
    m_automaton->state(GameLogic::GameAutomaton::INIT);
    m_automaton->savedState(GameLogic::GameAutomaton::INVALID);
    QObject::disconnect((QObject*)m_automaton->messageServer(),SIGNAL(connectionLost(KMessageIO*)),
                        (QObject*)m_automaton,SLOT(slotConnectionToClientBroken(KMessageIO*)));

    m_automaton->disconnect();

    m_newGameSetup->clear();
    m_automaton->setupPlayersNumberAndSkin(socket);
  }
  return false;
}

void KGameWindow::saveXml(QTextStream& xmlStream)
{
  xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  xmlStream << "<ksirkSavedGame formatVersion=\"" << SAVE_GAME_FILE_FORMAT_VERSION << "\" >";
  xmlStream << "<game skin=\"" << m_automaton->skin() << "\" state=\"" << m_automaton->state() << "\" >";
  m_theWorld->saveXml(xmlStream);
  xmlStream << "<players nb=\""<<m_automaton->playerList()->count()<<"\">";
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    static_cast<Player*>(*it)->saveXml(xmlStream);
  }
  xmlStream << "</players>";
  Player* player = m_automaton->currentPlayer();
  if (player)
  {
    QString name = player->name();
    name = name.replace('&',"&amp;");
    name = name.replace('<',"&lt;");
    name = name.replace('>',"&gt;");
    xmlStream << "<currentPlayer name=\"" << name << "\" />";
  }
  else
    xmlStream << "<currentPlayer name=\"\" />";
  xmlStream << "<goals>\n";
  it = m_automaton->playerList()->begin();
  it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    static_cast<Player*>(*it)->goal().saveXml(xmlStream);
  }
  xmlStream << "</goals>\n";
  xmlStream << "</game>";
  xmlStream << "</ksirkSavedGame>";
}

/**
  * @brief Accessor to the world
  * @return A pointer to the world
  */
ONU* KGameWindow::theWorld()
{
  return m_theWorld;
}

/**
  * @brief Adds a player
  */
Player* KGameWindow::addPlayer(const QString& playerName,
      unsigned int nbAvailArmies, 
      unsigned int nbCountries, 
      const QString& nationName, 
      bool isAI,
      const QString& password,
      unsigned int nbAttack,
      unsigned int nbDefense)
{
  qCDebug(KSIRK_LOG) << "Adding player (AI: " << isAI << ")";
  Player* p = dynamic_cast<Player*>(m_automaton->createPlayer(isAI?2:1,0,false)); 
  if (p)
  {
    if (!isAI)
    {
      m_chatDlg->setFromPlayer(p);
    }
    p->setName(playerName);
    p->setNation(nationName);
    p->setNbCountries(nbCountries);
    p->setNbAvailArmies(nbAvailArmies, true);
    p->setNbAttack(nbAttack);
    p->setNbDefense(nbDefense);
    p->setPassword(password);
    if (!m_automaton->addPlayer(p))
    {
      qCDebug(KSIRK_LOG) << p->name() << "NOT added!";
      p = 0; // freed - weired API
    }
  }
  return p;
}

QMap< QString, QString > KGameWindow::nationsList()
{
  QMap< QString, QString >  res;
  if (m_theWorld == 0)
  {
    return res;
  }
  QList<Nationality*>& nationsList = m_theWorld->getNationalities();
  qCDebug(KSIRK_LOG) << "There is " << nationsList.size() << " nations";
  QList<Nationality*>::iterator nationsIt = nationsList.begin();
  for (; nationsIt != nationsList.end(); nationsIt++ ) 
  {
    Nationality* nation = *nationsIt;
    qCDebug(KSIRK_LOG) << "Nation '" << nation->name() << "' = " << nation;
    res.insert(nation->name(),nation->flagFileName());
  } 
  return res;
}

/** @return true if the given player is the last one ; false otherwise */
bool KGameWindow::isLastPlayer(const Player& player)
{
  if (m_automaton->playerList()->begin() == m_automaton->playerList()->end())
  {
    qCCritical(KSIRK_LOG) << "No player ; should not be able to call isLastPlayer !";
    exit(1);
  }
  PlayersArray::iterator it = m_automaton->playerList()->end();
  //  it--;
  Player* lastPlayer = static_cast<Player*>(*it);
  return (player == (*lastPlayer));
}

void KGameWindow::actionRecycling()
{
  qCDebug(KSIRK_LOG) << "KGameWindow::actionRecycling";
  setCurrentPlayerToFirst();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  m_automaton->sendMessage(buffer,StartLocalCurrentAI);
  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  stream2 << currentPlayer()->name();
  stream2 << (quint32)0;
  qCDebug(KSIRK_LOG) << "sending DisplayRecycleDetails "
    << currentPlayer()->name() << 0
    << " at " << __FILE__ << ", line " << __LINE__;
  m_automaton->sendMessage(buffer2,DisplayRecycleDetails);

  QPixmap pm = currentPlayer()->getFlag()->image(0);
}

void KGameWindow::actionRecyclingFinished()
{
  qCDebug(KSIRK_LOG);
  getRightDialog()->close();
  if (m_automaton->isAdmin())
  {
    setCurrentPlayerToFirst();
    distributeArmies();

    QByteArray buffer;
    m_automaton->sendMessage(buffer,ShowArmiesToPlace);

    clear();
    QByteArray buffer2;
    m_automaton->sendMessage(buffer2,StartLocalCurrentAI);

    QByteArray buffer3;
    QDataStream stream3(&buffer3, QIODevice::WriteOnly);
    stream3 << currentPlayer()-> name();
    stream3 << (quint32)nbNewArmies(currentPlayer());
    qCDebug(KSIRK_LOG) << "sending DisplayRecycleDetails "
        << currentPlayer()->name() << nbNewArmies(currentPlayer())
        << " at " << __FILE__ << ", line " << __LINE__;
    m_automaton->sendMessage(buffer3,DisplayRecycleDetails);
    m_automaton->state(GameLogic::GameAutomaton::NEWARMIES);
  }
}

void KGameWindow::finishMoves()
{
  qCDebug(KSIRK_LOG);
  m_animFighters->moveAllToDestinationNow();
}

void KGameWindow::displayButtonsForState(GameAutomaton::GameState state)
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  switch (state)
  {
  case GameLogic::GameAutomaton::WAIT:;
    m_automaton->sendMessage(buffer,ClearHighlighting);
    break;
  case GameLogic::GameAutomaton::WAIT_RECYCLING:;
    m_automaton->sendMessage(buffer,StartLocalCurrentAI);
    break;
  case GameLogic::GameAutomaton::NEWARMIES:;
    m_automaton->sendMessage(buffer,StartLocalCurrentAI);
    break;
  case GameLogic::GameAutomaton::INIT:;
    break;
  case GameLogic::GameAutomaton::INTERLUDE:;
    break;
  case GameLogic::GameAutomaton::ATTACK:;
    break;
  case GameLogic::GameAutomaton::ATTACK2:;
    break;
  case GameLogic::GameAutomaton::INVADE:;
    break;
  case GameLogic::GameAutomaton::SHIFT1:;
    break;
  case GameLogic::GameAutomaton::SHIFT2:;
    break;
  case GameLogic::GameAutomaton::FIGHT_BRING:;
    break;
  case GameLogic::GameAutomaton::FIGHT_ANIMATE:;
    break;
  case GameLogic::GameAutomaton::FIGHT_BRINGBACK:;
    break;
  case GameLogic::GameAutomaton::WAITDEFENSE:;
    break;
  case GameLogic::GameAutomaton::EXPLOSION_ANIMATE:;
    break;
    
  default: 
    m_automaton->sendMessage(buffer,ClearHighlighting);
  }
}

void KGameWindow::optionsConfigure()
{
  //An instance of your dialog could be already created and could be cached, 
  //in which case you want to display the cached dialog instead of creating 
  //another one 
  if ( KsirkConfigurationDialog::showDialog( "settings" ) ) 
    return; 
 
  //KConfigDialog didn't find an instance of this dialog, so lets create it : 
  KsirkConfigurationDialog* dialog = new KsirkConfigurationDialog( this, "settings", 
                                             KsirkSettings::self() ); 

  connect(dialog,SIGNAL(armiesNumberShowingChanged(int)),
      this,SLOT(slotArmiesNumberChanged(int)));

  dialog->show();
}

void KGameWindow::explain()
{
  KMessageParts message0Parts;
  message0Parts << I18N_NOOP("<b>KsirK quick Introduction</b>");
  broadcastChangeItem(message0Parts, ID_NO_STATUS_MSG);

  KMessageParts message1Parts;
  message1Parts << I18N_NOOP("Attacks and moves are issued through drag & drop between neighbour countries.");
  broadcastChangeItem(message1Parts, ID_NO_STATUS_MSG);

  KMessageParts message2Parts;
  message2Parts << I18N_NOOP("Start a new game or join a network game with the menu or the toolbar...");
  broadcastChangeItem(message2Parts, ID_NO_STATUS_MSG);

  KMessageParts message5Parts;
  message5Parts << I18N_NOOP("and then let the system guide you through messages and tooltips appearing on buttons when hovering above them. You can disable bubble help in the options window.");
  broadcastChangeItem(message5Parts, ID_NO_STATUS_MSG);
}

void KGameWindow::showMessage(const QString& message, quint32 delay, MessageShowingType forcing)
{
  qCDebug(KSIRK_LOG);
  QString lmessage = message + "<br><a href=\"dontshowagain\">"+i18n("Don't show messages anymore") + "</a>";
  if(KsirkSettings::helpEnabled() || forcing == ForceShowing)
  {
    if (m_message == 0)
    {
      qCDebug(KSIRK_LOG) << "Creating KGamePopupItem";
      m_message  = new KGamePopupItem();
      connect(m_message,SIGNAL(linkActivated(QString)),this,SLOT(slotDisableHelp(QString)));
      m_scene_world->addItem(m_message);
      m_message->setSharpness(KGamePopupItem::Soft);
      m_message->setZValue(1000);
    }
    m_message->setMessageTimeout(delay*1000);
    m_message->showMessage(lmessage, KGamePopupItem::TopLeft, KGamePopupItem::ReplacePrevious);

//   m_message->setPos(m_frame-> mapToScene(QPoint(30,30)));
  }
}


void KGameWindow::firstCountry(GameLogic::Country* country)
{
  qCDebug(KSIRK_LOG) << (void*)country;
  if (m_firstCountry != 0)
  {
    m_firstCountry->releaseHighlightingLock();
    m_firstCountry->clearHighlighting();
  }
  m_firstCountry = country;
  if (country == 0)
  {
    return;
  }
  qCDebug(KSIRK_LOG) << country->name();
  country->highlightAsAttacker();
}

void KGameWindow::secondCountry(GameLogic::Country* country)
{
  if (m_secondCountry != 0)
  {
    m_secondCountry->clearHighlighting();
  }
  m_secondCountry = country;
  if (country == 0)
  {
    return;
  }
  qCDebug(KSIRK_LOG) << country->name();
  country->highlightAsDefender();
}


GameLogic::Country* KGameWindow::firstCountry()
{
  if (m_currentDisplayedWidget == Arena) {
     return m_arena->countryAttack();
  }
  return m_firstCountry;
}


GameLogic::Country* KGameWindow::secondCountry()
{
  if (m_currentDisplayedWidget == Arena) {
    return m_arena->countryDefense();
  }
  return m_secondCountry;
}


void KGameWindow::showArena()
{
  qCDebug(KSIRK_LOG);
  if (m_currentDisplayedWidget != Arena)
  {
    // synchronize the arena countries
    m_currentDisplayedWidget = Arena;
    m_arena->initFightArena(m_firstCountry,m_secondCountry,m_backGnd_arena);
  }
  qCDebug(KSIRK_LOG) << "before setCurrentIndex";
  m_centralWidget->setCurrentIndex(ARENA_INDEX);
}


void KGameWindow::showMap()
{
  qCDebug(KSIRK_LOG);
  m_centralWidget->setCurrentIndex(MAP_INDEX);
  m_currentDisplayedWidget = Map;
  statusBar()->show();
  m_zoomInAction->setEnabled(true);
  m_zoomOutAction->setEnabled(true);
}

void KGameWindow::showMainMenu()
{
  qCDebug(KSIRK_LOG);
  m_centralWidget->setCurrentIndex(MAINMENU_INDEX);
  m_currentDisplayedWidget = MainMenu;
}


KGameWindow::WidgetType KGameWindow::currentWidgetType()
{
  return m_currentDisplayedWidget;
}


QGraphicsView* KGameWindow::currentWidget()
{
  switch (currentWidgetType())
  {
    case Arena:
      return dynamic_cast <QGraphicsView*>(arena());
      break;
    case MainMenu:
      return 0;
      break;
    case Map:
      return dynamic_cast <QGraphicsView*>(frame());
      break;
    default:
      return 0;
  }
  return 0;
}


BackGnd* KGameWindow::backGnd() {
  if (currentWidgetType() == Arena) {
    return backGndArena();
  } else {
    return backGndWorld();
  }
}

void KGameWindow::slideInvade(GameLogic::Country * attack, GameLogic::Country * defender, InvasionSlider::InvasionType invasionType)
{
  if (m_wSlide != 0)
  {
    m_wSlide->hide();
    delete m_wSlide;
  }
  m_wSlide = new InvasionSlider(this,attack,defender,invasionType);
  m_wSlide->show();
}

bool KGameWindow::isArena()
{
  return m_useArena;
}

void KGameWindow::reduceChat()
{
  qCDebug(KSIRK_LOG);
  m_chatIsReduced = true;

  m_lastWidthChat = m_bottomDock->width();

  // reduce the chat
  QPixmap upChatReducePix(QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Images/upArrow.png"));
  m_reduceChatButton->setIcon(upChatReducePix);
  m_chatDlg->hide();
  m_titleChatMsg->show();
}

void KGameWindow::unreduceChat()
{
  qCDebug(KSIRK_LOG);
  m_chatIsReduced = false;

  // restore the chat
  QPixmap downChatReducePix(QStandardPaths::locate(QStandardPaths::AppDataLocation, m_automaton->skin() + "/Images/downArrow.png"));
  m_reduceChatButton->setIcon(downChatReducePix);
  m_chatDlg->show();
  m_titleChatMsg->hide();
  m_bottomDock->setMaximumSize(16777215,16777215);
  m_bottomDock->resize(m_lastWidthChat,38+m_chatDlg->height());
}

void KGameWindow::setNextPlayerActionEnabled(bool value)
{
  qCDebug(KSIRK_LOG) << value;
  m_nextPlayerAction->setEnabled(value);
}

void KGameWindow::setSaveGameActionEnabled(bool value)
{
  qCDebug(KSIRK_LOG) << value;
  m_saveGameAction->setEnabled(value);
}

void KGameWindow::setContextualHelpActionEnabled(GameLogic::GameAutomaton::GameState gameState, bool isPlayerAI)
{
  qCDebug(KSIRK_LOG) << isPlayerAI << gameState;
  bool enabled = (gameState == GameLogic::GameAutomaton::WAIT || gameState == GameLogic::GameAutomaton::NEWARMIES || gameState == GameLogic::GameAutomaton::INTERLUDE) &&
                 !isPlayerAI &&
                 KsirkSettings::helpEnabled();
  m_contextualHelpAction->setEnabled(enabled);
}

void KGameWindow::setupPopupMessage()
{
  if (m_message == 0)
  {
    qCDebug(KSIRK_LOG);
    m_message  = new KGamePopupItem();
    connect(m_message,SIGNAL(linkActivated(QString)),this,SLOT(slotDisableHelp(QString)));
    m_scene_world->addItem(m_message);
    m_message->setSharpness(KGamePopupItem::Soft);
    QColor color = QColor(102,102,255);
    m_message->setBackgroundBrush(color);
    m_message->setZValue(1000);
  }
}

bool KGameWindow::newGameDialog(const QString& skin, GameAutomaton::NetworkGameType netGameType)
{
  qCDebug(KSIRK_LOG) << "state is" << m_automaton->stateName();
  m_automaton->setGameStatus( KGame::Pause );
  m_stateBeforeNewGame = m_automaton->state();
  m_automaton->state(GameAutomaton::STARTING_GAME);
  m_rightDock->hide();
  statusBar()->hide();
  m_zoomInAction->setEnabled(false);
  m_zoomOutAction->setEnabled(false);
  m_nextPlayerAction->setEnabled(false);
  m_goalAction->setVisible(false);;
  m_newGameDialog->init(skin, netGameType);
  m_stackWidgetBeforeNewGame = m_centralWidget->currentIndex();
  m_centralWidget->setCurrentIndex(NEWGAME_INDEX);
  return false;
}

/* Set presence (usually called by dialog widget). */
void KGameWindow::setPresence ( const XMPP::Status &status )
{
  qCDebug(KSIRK_LOG) << "Status: " << status.show () << ", Reason: " << status.status ();
  
  // fetch input status
  XMPP::Status newStatus = status;
  
  // TODO: Check if Caps is enabled
  // Send entity capabilities
  if( m_jabberClient )
  {
    newStatus.setCapsNode( m_jabberClient->capsNode() );
    newStatus.setCapsVersion( m_jabberClient->capsVersion() );
    newStatus.setCapsExt( m_jabberClient->capsExt() );
  }
  
  // make sure the status gets the correct priority
  newStatus.setPriority ( 5 );
  
/*  XMPP::Jid jid ( this->contactId() );
  XMPP::Resource newResource ( resource (), newStatus );
  
  // update our resource in the resource pool
  resourcePool()->addResource ( jid, newResource );
  
  // make sure that we only consider our own resource locally
  resourcePool()->lockToResource ( jid, newResource );*/
  
  /*
  * Unless we are in the connecting status, send a presence packet to the server
  */
  if(status.show () != QString("connecting") )
  {
    /*
    * Make sure we are actually connected before sending out a packet.
    */
    if (true/*isConnected()*/)
    {
      qCDebug(KSIRK_LOG) << "Sending new presence to the server.";
      
      XMPP::JT_Presence * task = new XMPP::JT_Presence ( m_jabberClient->rootTask ());
      
      task->pres ( newStatus );
      task->go ( true );
    }
    else
    {
      qCDebug(KSIRK_LOG) << "We were not connected, presence update aborted.";
    }
  }
  
}

void KGameWindow::askForJabberGames()
{
  if (m_jabberClient && m_jabberClient->isConnected())
  {
    XMPP::Message message(QString(m_groupchatRoom+'@'+m_groupchatHost));
    message.setType("groupchat");
    message.setId(QUuid::createUuid().toString().remove('{').remove('}').remove('-'));
    QString body("Who propose online KsirK games here?");
    message.setBody(body);
    qCDebug(KSIRK_LOG) << "Sending message: <<" << body << ">> to" << (m_groupchatRoom+'@'+m_groupchatHost);
    m_jabberClient->sendMessage(message);
  }
}

void KGameWindow::sendGameInfoToJabber()
{
  qCDebug(KSIRK_LOG);
  if (m_jabberClient
    && m_jabberClient->isConnected()
    && m_automaton->startingGame())
  {
    qCDebug(KSIRK_LOG) << "Sending 'I'm starting a game with ...'";
    XMPP::Message message(QString(m_groupchatRoom+'@'+m_groupchatHost));
    message.setType("groupchat");
    message.setId(QUuid::createUuid().toString().remove('{').remove('}').remove('-'));
    QString body;
    QTextStream qts(&body);
    qts <<"I'm starting a game with skin '" << m_automaton->skin() << "' and '" << m_automaton->nbPlayers() << "' players";
    message.setBody(body);
    qCDebug(KSIRK_LOG) << "Sending message: <<" << body << ">> to" << (m_groupchatRoom+'@'+m_groupchatHost);
    m_jabberClient->sendMessage(message);
  }
}

void KGameWindow::joinNetworkGame()
{
  qCDebug(KSIRK_LOG);
  /// @TODO show the network connect screen
  m_centralWidget->setCurrentIndex(TCPCONNECT_INDEX);
}

void KGameWindow::updateNewGameSummary()
{
  qCDebug(KSIRK_LOG);
  m_newGameSummaryWidget->show(this);
}

void KGameWindow::showNewGameSummary()
{
  qCDebug(KSIRK_LOG);
  m_centralWidget->setCurrentIndex(NEWGAMESUMMARY_INDEX);
}

} // closing namespace Ksirk


