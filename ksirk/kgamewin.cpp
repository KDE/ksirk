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

/*  begin                : mer jui 11 22:27:28 EDT 2001   */

// application specific includes
#include "kgamewin.h"
#include "ksirkConfigDialog.h"
#include "ksirksettings.h"
#include "MessageBubble.h"
#include "Sprites/animspritesgroup.h"
#include "GameLogic/aiplayer.h"
#include "GameLogic/aiColsonPlayer.h"
#include "GameLogic/aiplayerio.h"
#include "GameLogic/country.h"
#include "GameLogic/onu.h"
#include "GameLogic/dice.h"
#include "GameLogic/KMessageParts.h"
#include "GameLogic/goal.h"
#include "GameLogic/gameautomaton.h"
#include "GameLogic/KsirkChatItem.h"
#include "GameLogic/KsirkChatModel.h"
#include "GameLogic/KsirkChatDelegate.h"
#include "SaveLoad/ksirkgamexmlloader.h"
#include "Sprites/backgnd.h"
#include "Dialogs/kplayersetupdialog.h"
#include "Dialogs/kwaitedplayersetupdialog.h"
#include "Dialogs/restartOrExitDialogImpl.h"


// include files for QT
#include <QDockWidget>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardgameaction.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <phonon/mediaobject.h>
#include <KPushButton>
#include <kchatdialog.h>
#include <kgame/kgamechat.h>
#include <kgamepopupitem.h>
#include <kglobal.h>
#include <KStatusBar>
#include <KToolBar>
#include <KAction>
#include <KSvgRenderer>


#include <assert.h>


namespace Ksirk
{
using namespace GameLogic;

// port all occurrences of m_accels
// port all occurrences of setBarPos

KGameWindow::KGameWindow(QWidget* parent) :
  KXmlGuiWindow(parent), m_automaton(new GameAutomaton()),
  NKD(0), NKA(0),
  m_theWorld(0), m_scene(0), m_backGnd(0),
  m_animFighters(new AnimSpritesGroup(this,SLOT(slotMovingFightersArrived(AnimSpritesGroup*)))),
  m_nbMovedArmies(0),
  m_firstCountry(0), m_secondCountry(0),
  m_frame(0),
  m_goalAction(0),
  m_barFlag(new QLabel(this)),
//   m_accels(this),
//   m_chat(0), 
  m_chatDlg(0),
  m_audioPlayer(Phonon::createPlayer( Phonon::NotificationCategory )),
  m_timer(this),
  gameActionsToolBar(0),
  m_message(0),
  m_mouseLocalisation(0)
{
  kDebug() << "KGameWindow constructor begin" << endl;

  statusBar()->addWidget(m_barFlag);

  m_dirs = KGlobal::dirs();
//   m_accels.setEnabled(true);
  
  QString iconFileName = m_dirs-> findResource("appdata", m_automaton->skin() + "/Images/SoldatAGenoux1.png");
  if (iconFileName.isNull())
  {
      QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load icon\nProgram cannot continue"));
      exit(2);
  }
  QPixmap icon(iconFileName);

  m_bottomDock = new QDockWidget();
  KsirkChatModel* chatModel = new KsirkChatModel(this);
  KsirkChatDelegate* chatDelegate = new KsirkChatDelegate(this);
  m_chatDlg = new KGameChat(m_automaton, 10000, this,chatModel,chatDelegate);
  connect(m_chatDlg,
          SIGNAL(signalReturnPressed(const QString&)),
          this,
          SLOT(slotChatMessage()));
  m_bottomDock->setWidget(m_chatDlg);
  m_bottomDock->setAllowedAreas(Qt::BottomDockWidgetArea);
  addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock); // master dockwidget

//    kDebug() << "Before initActions" << endl;
  initActions();

  kDebug() << "Setting up GUI" << endl;
  setupGUI();

  kDebug() <<"Setting up toolbars" << endl;
  kDebug() <<"  creating gameActionsToolBar" << endl;
  gameActionsToolBar = new KToolBar("gameActionsToolBar", this, Qt::BottomToolBarArea);
  gameActionsToolBar->setWindowTitle(i18n("Game Actions Toolbar"));
  gameActionsToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  gameActionsToolBar->setAllowedAreas(Qt::BottomToolBarArea);
  gameActionsToolBar->setIconSize(QSize(32,32));
  gameActionsToolBar->show();

  kDebug() << "Creating automaton" << endl;
  m_automaton->init(this);
  
  kDebug() << "Setting skin" << endl;
  m_automaton->skin(KGlobal::config()->group("skin").readEntry("skin", "skins/default"));

  
//    kDebug() << "Before initStatusBar" << endl;
  initStatusBar();
  
  menuBar()-> show();
  
/*  displayOpenGameButton();*/

//   connect(m_barFlagButton, SIGNAL(clicked()), this, SLOT(slotShowGoal()));
  explain();
  m_automaton->run();
  setMouseTracking(true);

  m_timer.setSingleShot(true);
  connect(&m_timer,SIGNAL(timeout()),this,SLOT(evenementTimer()));
}

KGameWindow::~KGameWindow()
{
  kDebug() << "~GameAutomaton" << endl;
  m_dirs = 0;
  delete m_backGnd; m_backGnd = 0;
  delete m_scene; m_scene = 0;
//   if (m_barFlagButton) {delete m_barFlagButton; m_barFlagButton = 0;}
  delete m_frame; m_frame = 0;
  delete m_audioPlayer;
}

void KGameWindow::initActions()
{
  QAction *action;

  // standard game actions
  action = KStandardGameAction::gameNew(this, SLOT(slotNewGame()), this);
  actionCollection()->addAction(action->objectName(), action);
  action = KStandardGameAction::load(this, SLOT(slotOpenGame()), this);
  actionCollection()->addAction(action->objectName(), action);
  action = KStandardGameAction::save(this, SLOT(slotSaveGame()), this);
  actionCollection()->addAction(action->objectName(), action);
  action = KStandardGameAction::quit(this, SLOT(close()), this);
  actionCollection()->addAction(action->objectName(), action);

  action = KStandardAction::zoomIn(this, SLOT(slotZoomIn()), this);
  actionCollection()->addAction(action->objectName(), action);

  action = KStandardAction::zoomOut(this, SLOT(slotZoomOut()), this);
  actionCollection()->addAction(action->objectName(), action);

  // specific ksirk action
  QString imageFileName = m_dirs-> findResource("appdata", m_automaton->skin() + '/' + CM_NEWNETGAME);
//   kDebug() << "Trying to load button image file: " << imageFileName << endl;
  if (imageFileName.isNull())
  {
    QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
    exit(2);
  }
  QAction* joinAction = new QAction(QIcon(QPixmap(imageFileName)),
        i18n("Join"), this);
  joinAction->setShortcut(Qt::CTRL+Qt::Key_J);
  joinAction->setStatusTip(i18n("Join network game"));
  connect(joinAction,SIGNAL(triggered(bool)),this,SLOT(slotJoinNetworkGame()));
   kDebug() << "Adding action game_join" << endl;
  actionCollection()->addAction("game_join", joinAction);

  m_goalAction = new QAction(QIcon(), i18n("Goal"), this);
  m_goalAction-> setText(i18n("Display the current player goal"));
  m_goalAction-> setIconText("  ");
  m_goalAction->setShortcut(Qt::CTRL+Qt::Key_G);
  m_goalAction->setStatusTip(i18n("Display the current player goal"));
  connect(m_goalAction,SIGNAL(triggered(bool)),this,SLOT(slotShowGoal()));
  kDebug() << "Adding action game_goal" << endl;
  actionCollection()->addAction("game_goal", m_goalAction);
}

void KGameWindow::initStatusBar()
{
  statusBar()-> setSizeGripEnabled(true);
  statusBar()->insertPermanentItem("", ID_STATUS_MSG, 2);
  statusBar()-> setItemAlignment(ID_STATUS_MSG, Qt::AlignLeft | Qt::AlignVCenter);
  statusBar()->insertPermanentItem("", ID_STATUS_MSG2, 3);
  statusBar()-> setItemAlignment(ID_STATUS_MSG2, Qt::AlignLeft | Qt::AlignVCenter);
  statusBar()->addPermanentWidget(m_barFlag);
}

Country* KGameWindow::clickIn(const QPointF &pointf)
{
//   kDebug() << "KGameWindow::clickIn " << pointf << endl;

  return m_theWorld-> countryAt( pointf );
}

Player* KGameWindow::currentPlayer()
{
//   kDebug() << "KGameWindow::currentPlayer" << endl;
  Player* current = m_automaton->currentPlayer();

  return current;
}

void KGameWindow::loadDices()
{
  kDebug() << endl;
  
  m_dices[Blue] = std::vector<QPixmap>(6);
  m_dices[Red] = std::vector<QPixmap>(6);
//   QString dicesDir = m_dirs->findResourceDir("appdata", m_automaton->skin() + "/Images/reddice1.png") + m_automaton->skin() + "/Images/";
  m_dices[Blue][0] = buildDice(Blue, "bluedice1");
  m_dices[Blue][1] = buildDice(Blue, "bluedice2");
  m_dices[Blue][2] = buildDice(Blue, "bluedice3");
  m_dices[Blue][3] = buildDice(Blue, "bluedice4");
  m_dices[Blue][4] = buildDice(Blue, "bluedice5");
  m_dices[Blue][5] = buildDice(Blue, "bluedice6");
  m_dices[Red][0] = buildDice(Red, "reddice1");
  m_dices[Red][1] = buildDice(Red, "reddice2");
  m_dices[Red][2] = buildDice(Red, "reddice3");
  m_dices[Red][3] = buildDice(Red, "reddice4");
  m_dices[Red][4] = buildDice(Red, "reddice5");
  m_dices[Red][5] = buildDice(Red, "reddice6");
}

QPixmap KGameWindow::buildDice(DiceColor color, const QString& id)
{
  kDebug() << endl;
//   QString stop1Color, stop2Color;
//   if (color == Red)
//   {
//     stop1Color = "#f07f7f";
//     stop2Color = "#620000";
//   }
//   else
//   {
//     stop1Color = "#707fff";
//     stop2Color = "#000062";
//   }
//   QDomNode stop1Element = m_theWorld->svgDom()->elementById("stop1LinearGradientDice");
//   m_theWorld->svgDom()->setCurrentNode(stop1Element);
//   m_theWorld->svgDom()->setStyleProperty("style", QString("stop-color:")+stop1Color+";stop-opacity:1");
// 
//   QDomNode stop2Element = m_theWorld->svgDom()->elementById("stop2LinearGradientDice");
//   m_theWorld->svgDom()->setCurrentNode(stop2Element);
//   m_theWorld->svgDom()->setStyleProperty("style", QString("stop-color:")+stop2Color+";stop-opacity:1");
// 
//   kDebug() << "after stops " << stop1Color << ", " << stop2Color << endl;
// 
//   QByteArray svg = m_theWorld->svgDom()->toByteArray();
//   kDebug() << svg << endl;
//   KSvgRenderer renderer;
//   renderer.load(svg);
//   kDebug() << "after loading" << endl;

  QSize size(32,32);
  QImage image(size, QImage::Format_ARGB32_Premultiplied);
  image.fill(0);
  QPainter p(&image);
  m_theWorld->renderer()->render(&p, id);

  return QPixmap::fromImage(image);
}

// void KGameWindow::resizeEvent ( QResizeEvent * event )
// {
//   if (m_frame!=0)
//   {
//     m_frame->fitInView(m_backGnd, Qt::KeepAspectRatio);
//   }
// }

void KGameWindow::newSkin(const QString& onuFileName)
{
  kDebug() << "KGameWindow::newSkin '" << onuFileName << "'" << endl;
  clear();

  if (m_frame != 0)
  {
    m_frame->hide();
//     delete m_frame;
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
    onuDefinitionFileName = m_dirs-> findResource("appdata", m_automaton->skin() + "/Data/onu.desktop");
  }
  if (onuDefinitionFileName.isEmpty())
  {
      QMessageBox::critical(0, i18n("Error !"),
          i18n("UNO definition XML file not found - Verify your installation\nProgram cannot continue"));
      exit(2);
  }
  kDebug() << "Got ONU definition file name: " <<  onuDefinitionFileName << endl;
  m_theWorld = new ONU(m_automaton, onuDefinitionFileName);

  loadDices();

  if (m_frame == 0)
    m_frame = new DecoratedGameFrame(this, m_theWorld->width(), m_theWorld->height());
  m_frame->setMaximumWidth(m_theWorld->width());
  m_frame->setMaximumHeight(m_theWorld->height());
  m_frame->setCacheMode( QGraphicsView::CacheBackground );
  setCentralWidget(m_frame);

  if (m_scene != 0)
  {
    delete m_scene;
  }
    m_scene = new QGraphicsScene(0, 0, m_theWorld->width(), m_theWorld->height(),this);
//   m_scene->setDoubleBuffering(true);
  kDebug() << "Before initView" << endl;
  initView();
  m_backGnd = new BackGnd(m_scene, m_theWorld); //Creation of the background
  kDebug() <<"After m_backGnd new="<< m_backGnd << endl;
  m_frame->setFocus();

  kDebug() <<"End new skin" << endl;
}

void KGameWindow::initView()
{
  QString iconFileName = m_dirs-> findResource("appdata", m_automaton->skin() + "/Images/SoldatAGenoux1.png");
  if (iconFileName.isNull())
  {
      QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load icon\nProgram cannot continue"));
      exit(2);
  }
// to port : still necessary ?
//   m_frame->setIcon(QPixmap(iconFileName));

  disconnectMouse();
  reconnectMouse();
  setCaption("KsirK",false);
  m_scene-> update();
  m_frame->setScene(m_scene);
  m_frame-> show();
  adjustSize();

  m_frame->setFocus();
}

bool KGameWindow::attackEnd()
{
  kDebug() << endl;
  if (m_firstCountry==0 || m_secondCountry == 0)
  {
    return false;
  }
  m_firstCountry->releaseHighlightingLock();
  m_firstCountry->clearHighlighting();
  m_secondCountry->releaseHighlightingLock();
  m_secondCountry->clearHighlighting();
  bool res = false;
  QString mes = "";
  kDebug() << "There is now " << m_secondCountry-> nbArmies() << " armies in " << m_secondCountry->name() << "." << endl;
  if (m_secondCountry-> nbArmies() == 0)
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
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << currentPlayer()->id();
      m_automaton->sendMessage(buffer,CheckGoal);
      
    }
    
    m_secondCountry-> owner(currentPlayer());
    m_secondCountry-> nbArmies(1);
    m_firstCountry-> decrNbArmies();
    m_scene-> update();
    if (m_firstCountry->nbArmies() > 1)
    {
      res = true;
    }
    kDebug() << oldOwner-> name() << " now owns " << newOldOwnerNbCountries << " countries." << endl;
    if (newOldOwnerNbCountries == 0)
    {
      unsigned int oldOwnerId = oldOwner->id();
      KMessageBox::information(this,
                               i18n("%1, you are defeated! Bye, bye...",oldOwner->name()),
                               i18n("KsirK - Game Over !"));
      if (m_automaton->isAdmin())
      {
        kDebug() << "Removing player " << oldOwner-> name() << endl;
        int i = m_automaton->playerList()->indexOf(oldOwner);
        if (i != -1)
          delete m_automaton->playerList()->takeAt(i);
        kDebug() << "There is now " << m_automaton->playerList()->count() << " m_automaton->playerList()->" << endl;
      }
      if ( ( (m_automaton->useGoals())
             && ( (currentPlayer()->goal().type() == GameLogic::Goal::GoalPlayer)
             && ( (*currentPlayer()->goal().players().begin()) == oldOwnerId) ) )
           || (m_automaton->playerList()->count() == 1) )
      {
        m_automaton->state(GameLogic::GameAutomaton::GAME_OVER);
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream << currentPlayer()->id();
        m_automaton->sendMessage(buffer,Winner);
        return res;
      }
    }
  }
  m_firstCountry-> createArmiesSprites(m_backGnd);
  m_secondCountry-> createArmiesSprites(m_backGnd);
  if (m_automaton->isAdmin())
  {
    if (res)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      m_automaton->sendMessage(buffer,DisplayInvasionButtons);
    }
    else
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      m_automaton->sendMessage(buffer,DisplayNormalGameButtons);
      KMessageParts messageParts;
      messageParts << I18N_NOOP("%1 : it is up to you again") << currentPlayer()-> name();
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    }
  }
  return res;
}

void KGameWindow::winner(const Player* player)
{
  QString msg = "%1 won !";
  if (!player->isVirtual())
  {
    msg = "<big><b>%1</b>, you won !</big>";
  }
  if (m_automaton->useGoals())
  {
    msg += QString("<br/>Winner's goal was stated like this:<br/><i>") 
        += player->goal().message()
        += QString("</i><br/><br/>Do you want to play again ?");
  }
  else
  {
    if (!player->isVirtual())
    {
      msg += "<br/>You conquered all the world!";
    }
    else
    {
      msg += "<br/>He conquered all the world!";
    }
  }
  RestartOrExitDialogImpl* restartDia = new RestartOrExitDialogImpl(i18n(msg.toUtf8().data(),player->name()).toUtf8().data());
              
  connect((QObject*)restartDia->newGameButton,
              SIGNAL(clicked()),
              this,
              SLOT(slotNewGame()));

  connect((QObject*)restartDia->exitButton,
              SIGNAL(clicked()),
              this,
              SLOT(close()));

  restartDia->show();
}

void KGameWindow::resolveAttack()
{
//    kDebug() << "KGameWindow::resolveAttack" << endl;

  int A1 = -1; int A2 = -1; int A3 = -1; int AE = -1;
  int D1 = -1; int D2 = -1; int DE = -1;
  
  NKD = NKA = 0;
  if (currentPlayer() == 0 || m_secondCountry == 0 || m_secondCountry->owner() == 0 || m_firstCountry == 0)
    return;
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
//   kDebug() << "(" << A1<<", "<<A2<<", "<<A3<<") <-> ("<<D1<<", "<<D2<<")"  << endl;
//   kDebug() << "Attacker loses " << NKA<<" armies; Defender loses "<<NKD<<" armies."  << endl;
  
  KMessageParts messageParts;
  if (currentPlayer()-> getNbAttack() == 1)
  {
    messageParts <<m_dices[Red][A1-1]<<"  "<<m_dices[Blue][D1-1];
  }
  else if (currentPlayer()-> getNbAttack() == 2 && m_secondCountry-> owner()-> getNbDefense() == 1) 
  {
    messageParts <<m_dices[Red][A1-1]<<" "<<m_dices[Red][A2-1]
            <<"  "<<m_dices[Blue][D1-1];
  }
  else if (currentPlayer()-> getNbAttack() == 2 && m_secondCountry-> owner()-> getNbDefense() == 2) 
  {
    messageParts <<m_dices[Red][A1-1]<<" "<<m_dices[Red][A2-1]
            <<"  "<<m_dices[Blue][D1-1]<<"  "<<m_dices[Blue][D2-1];
  }
  else if (currentPlayer()-> getNbAttack() == 3 && m_secondCountry-> owner()-> getNbDefense() == 1)
  {
    messageParts <<m_dices[Red][A1-1]<<" "<<m_dices[Red][A2-1]<<" "<<m_dices[Red][A3-1]<<"  "<<m_dices[Blue][D1-1];
  }
  else if (currentPlayer()-> getNbAttack() == 3 && m_secondCountry-> owner()-> getNbDefense() == 2) 
  {
    kDebug() << "3 attacks and 2 defenses: A1 A2 A3 D1 D2 = ["<<A1<<","<<A2<<","<<A3<<","<<D1<<","<<D2<<"]" << endl;
    kDebug() << "m_dices[Red] size= "<< m_dices[Red].size() << endl;
    kDebug() << "m_dices[Blue] size= "<< m_dices[Blue].size() << endl;
    messageParts << m_dices[Red][A1-1] 
            << " " << m_dices[Red][A2-1] 
            << " " << m_dices[Red][A3-1]
            << " " << m_dices[Blue][D1-1] 
            << " " << m_dices[Blue][D2-1];
  }
  broadcastChangeItem(messageParts, ID_NO_STATUS_MSG);
  if (NKA > 0)
  {
    QPixmap pm = currentPlayer()->getFlag()->image(0);
    KMessageParts messagePartsA;
    messagePartsA << I18N_NOOP("The attacker <font color=\"red\">%1</font> (")
                 << m_firstCountry->name() << pm
                 << I18N_NOOP("%1) <font color=\"red\">loses %2 armies</font>.") 
                 << currentPlayer()-> name() << QString::number(NKA);
    broadcastChangeItem(messagePartsA, ID_NO_STATUS_MSG);
  }
  if (NKD > 0)
  {
    QPixmap pm = m_secondCountry->owner()->getFlag()->image(0);
    KMessageParts messagePartsD;
    messagePartsD << I18N_NOOP("The defender <font color=\"blue\">%1</font> (") 
                 << m_secondCountry->name() << pm 
                 << I18N_NOOP("%1) <font color=\"blue\">loses %2 armies</font>.") 
                 << m_secondCountry->owner()-> name() << QString::number(NKD);
    broadcastChangeItem(messagePartsD, ID_NO_STATUS_MSG);
  }
  
  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  
  if ((NKD != 0)&&(NKA != 0)) stream2 << quint32(2);
  else if (NKA != 0) stream2 << quint32(0);
  else if (NKD != 0) stream2 << quint32(1);
  m_automaton->sendMessage(buffer2,AnimExplosion);
}

/**
  * Reimplementation of the inherited function called when a window close event arise
  */
bool KGameWindow::queryClose()
{
    switch ( KMessageBox::warningYesNoCancel( this, i18n("Before you quit, do want to save your game?")) ) {
    case KMessageBox::Yes :
        slotSaveGame();
        return true;
    case KMessageBox::No :
        return true;
    default: // cancel
        return false;
    }
}

bool KGameWindow::actionOpenGame()
{
  kDebug() << "KGameWindow::actionOpenGame" << endl;

  QString fileName = KFileDialog::getOpenFileName(KUrl(), "*.xml", this, i18n("KsirK - Load Game"));
  if (!fileName.isEmpty())
  {
    m_automaton->setGameStatus(KGame::End);
    m_waitedPlayers.clear();
    kDebug() << "KGameWindow::actionOpenGame loader" << endl;
    Ksirk::SaveLoad::GameXmlLoader loader(fileName, *this, m_waitedPlayers);
    kDebug() << "KGameWindow::actionOpenGame loading done" << endl;
    for (unsigned int i = 0; i < m_theWorld->getNbCountries(); i++)
    {
      Country* country = m_theWorld-> getCountries().at(i);
      kDebug() << "Adding sprites to " << country->name() << endl;
      country-> createArmiesSprites(m_backGnd);
      Player *player = country-> owner();
      if (player)
      {
        country-> flag(player->flagFileName(), m_backGnd);
      }
    }
    m_backGnd->hide();
    m_backGnd->show();
    if (m_waitedPlayers.empty())
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      m_automaton->sendMessage(buffer,StartGame);
      m_automaton->sendMessage(buffer,DisplayNormalGameButtons);
      m_frame->setFocus();
      kDebug() << "KGameWindow::actionOpenGame false1" << endl;
      return false;
    }
    else
    {
      KMessageParts messageParts;
      messageParts << I18N_NOOP("Waiting for the connection of %1 network players.") << QString::number(m_waitedPlayers.size());
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
      kDebug() << "KGameWindow::actionOpenGame true" << endl;
      return true;
    }
  }
  kDebug() << "KGameWindow::actionOpenGame false2" << endl;
  return false;
}

void KGameWindow::displayNextPlayerButton()
{
//   kDebug() << "displayNextPlayerButton" << endl;
  clearGameActionsToolbar(false);
  if (currentPlayer() && !currentPlayer()->isVirtual() && currentPlayer()-> isAI())
  {
//     kDebug() << "... NOT adding button: AI" << endl;
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) 
    {
      (static_cast< AIPlayer * >(currentPlayer()))-> start();
    }
  }
  else if (!currentPlayer()->isVirtual())
  {
//     kDebug() << "... adding button" << endl;
    addAButton( CM_NEXTPLAYER, 
                SLOT(slotNextPlayer()), i18n("Next Player"),KShortcut(Qt::Key_Escape),true);
  }
  gameActionsToolBar-> hide();
  gameActionsToolBar-> show();
}

void KGameWindow::displayRecyclingButtons()
{
//   kDebug() << "KGameWindow::displayRecyclingButtons" << endl;
  clearGameActionsToolbar(false);
  if (m_automaton->allLocalPlayersComputer())
  {
//     kDebug() << "There is only computer local players" << endl;
    PlayersArray::iterator it = m_automaton->playerList()->begin();
    PlayersArray::iterator it_end = m_automaton->playerList()->end();
    for (; it != it_end; it++)
    {
//       kDebug() << "Looking at player " << (*it)->name() 
//               << ". is AI  : " << ((Player*)(*it))->isAI() 
//               << ". isRunning: " << ((static_cast<AIPlayer *>(*it))-> isRunning())
//               << ". virtual: " << (*it)->isVirtual() 
//                       << endl;
      if ( ((Player*)(*it))->isAI() 
           && (!(static_cast<AIPlayer *>(*it))-> isRunning())
        && (!(*it)->isVirtual()) ) 
      {
//         kDebug() << "Starting computer player " << (*it)->name() << endl;
        (static_cast<AIPlayer *>(*it))-> start();
        break;
      }
    }
  }
  else
  {
    addAButton(CM_RECYCLING, SLOT(slotRecycling()), i18n("Redistribute"),KShortcut(Qt::Key_R),true);
    addAButton(CM_RECYCLINGFINISHED, SLOT(slotRecyclingFinished()), i18n("End redistribute"), KShortcut(Qt::Key_Tab), true);
  }
  gameActionsToolBar-> hide();
  gameActionsToolBar-> show();
}

void KGameWindow::displayOpenGameButton()
{
  clearGameActionsToolbar();
  addAButton(CM_OPENGAME, SLOT(slotOpenGame()), i18n("Open game"),KShortcut(Qt::CTRL+Qt::Key_O),true);
  gameActionsToolBar-> hide();
  gameActionsToolBar-> show();
}

void KGameWindow::displayNormalGameButtons()
{
  kDebug() << endl;
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
  gameActionsToolBar-> show();

  clearGameActionsToolbar(false);
  if (currentPlayer() && currentPlayer()-> isAI() && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
  else if (currentPlayer() && !currentPlayer()->isVirtual())
  {
/*    addAButton(CM_OPENGAME, SLOT(slotOpenGame()), i18n("Open game"),KShortcut(Qt::CTRL+Qt::Key_O),true);
    addAButton(CM_SAVEGAME, SLOT(slotSaveGame()), i18n("Save game"),KShortcut(Qt::CTRL+Qt::Key_S),true);*/
    addAButton(CM_NEXTPLAYER,  SLOT(slotNextPlayer()), i18n("Next Player"),KShortcut(Qt::Key_Escape),true);
    addAButton(CM_ATTACK1,  SLOT(slotAttack1()), i18n("Attack with one army"),KShortcut(Qt::Key_1),true);
    addAButton(CM_ATTACK2,  SLOT(slotAttack2()), i18n("Attack with two armies"),KShortcut(Qt::Key_2),true);
    addAButton(CM_ATTACK3,  SLOT(slotAttack3()), i18n("Attack with three armies"),KShortcut(Qt::Key_3),true);
    addAButton(CM_SHIFT, SLOT(slotMove()), i18n("Move armies"),KShortcut(Qt::Key_M),true);

    showMessage(i18n("Now, choose an action with the buttons at the bottom.<br/>Note that moving armies is the last action of a turn."), 5);
  }
  gameActionsToolBar-> hide();
  gameActionsToolBar-> show();
}

void KGameWindow::displayDefenseButtons()
{
  clearGameActionsToolbar(false);
  if (currentPlayer() && currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
  if (m_secondCountry && ! (m_secondCountry-> owner()->isAI() ))
  {
    if (m_secondCountry-> owner() && m_secondCountry-> owner()-> getFlag())
    {
      m_goalAction-> setIcon(KIcon(m_secondCountry-> owner()->getFlag()-> image(0)));
      m_goalAction-> setIconText(i18n("Goal"));
      m_barFlag-> setPixmap(m_secondCountry-> owner()->getFlag()-> image(0));
    }
    showMessage(i18n("%1, use the buttons below to choose<br/>"
                    "with how much armies you defend %2.",
                      m_secondCountry-> owner()-> name(),
                      m_secondCountry-> name()), 8);
    addAButton(CM_DEFENSE1, SLOT(slotDefense1()), i18n("Defend with one army"),KShortcut(Qt::Key_1),true);
    addAButton(CM_DEFENSE2, SLOT(slotDefense2()), i18n("Defend with two armies"),KShortcut(Qt::Key_2),true);
  }
  gameActionsToolBar-> hide();
  gameActionsToolBar-> show();
}

void KGameWindow::displayInvasionButtons()
{
  clearGameActionsToolbar(false);
  if (currentPlayer() && currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
  else if (!currentPlayer()->isVirtual())
  {
    addAButton(CM_INVADE1, SLOT(slotInvade1()), i18n("Invade with one army"),KShortcut(Qt::Key_1),true);
    addAButton(CM_INVADE5, SLOT(slotInvade5()), i18n("Invade with five armies"),KShortcut(Qt::Key_5),true);
    addAButton(CM_INVADE10, SLOT(slotInvade10()), i18n("Invade with ten armies"),KShortcut(Qt::Key_0),true);
    addAButton(CM_INVASIONFINISHED, SLOT(slotInvasionFinished()), i18n("End Invasion"),KShortcut(Qt::Key_Return),true);
    addAButton(CM_RETREAT1, SLOT(slotRetreat1()), i18n("Retract one army"),KShortcut(Qt::CTRL+Qt::Key_1),true);
    addAButton(CM_RETREAT5, SLOT(slotRetreat5()), i18n("Retract five armies"),KShortcut(Qt::CTRL+Qt::Key_5),true);
    addAButton(CM_RETREAT10, SLOT(slotRetreat10()), i18n("Retract ten armies"),KShortcut(Qt::CTRL+Qt::Key_0),true);
  }
  gameActionsToolBar-> hide();
  gameActionsToolBar-> show();
}

void KGameWindow::displayCancelButton()
{
  clearGameActionsToolbar();
  if (currentPlayer() && currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
  else addAButton(CM_CANCEL, SLOT(slotCancel()), i18n("Cancel"), KShortcut(Qt::Key_Escape), true);
  gameActionsToolBar-> hide();
  gameActionsToolBar-> show();
}

void KGameWindow::clearGameActionsToolbar(bool send)
{
  kDebug()<< "KGameWindow::clearGameActionsToolbar " << send << endl;
  if (send)
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ClearGameActionsToolbar);
  }
  
  gameActionsToolBar->clear();
  gameActionsToolBar->addSeparator();
  std::set< QString >::const_iterator it, it_end;
  it = m_temporaryAccelerators.begin(); it_end = m_temporaryAccelerators.end();
  for (; it != it_end; it++)
  {
//     m_accels.remove(*it);
  }
  m_temporaryAccelerators.clear();
//   m_accels.updateConnections();
}

void KGameWindow::addAButton(
    const QString& fileName, 
    const char* slot, 
    const QString& txt, 
    const KShortcut& shortcut,
    bool isTemp, 
    const std::string& toolBarName)
{
  kDebug() << "addAButton: " << fileName << endl;
  QString imageFileName = m_dirs-> findResource("appdata", m_automaton->skin() + '/' + fileName);
//   kDebug() << "Trying to load button image file: " << imageFileName << endl;
  if (imageFileName.isNull())
  {
    QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
    exit(2);
  }
  KToolBar* toolBar;
  if (toolBarName == "gameActionsToolBar")
  {
    toolBar = gameActionsToolBar;
  }
  else
  {
    kError() << "Unknown toolbar name" << endl;
    exit(2);
  }
  QAction* action = toolBar->addAction(QPixmap(imageFileName), txt, this,  slot);
//   kDebug() << "Button added " << txt << endl;
  if (shortcut != KShortcut())
  {
    QString str = " ";
    if (!txt.isEmpty())
    {
      str = txt;
    }
    action->setShortcut(shortcut.primary());
    action->setStatusTip(I18N_NOOP(str));
/*    void* accel = m_accels.insert( txt, i18n(str),
                          i18n(str),
                          shortcut, this, slot );*/
//     kDebug() << "Inserted accelerator " << accel << endl;
    
  }
  if (isTemp)
  {
    m_temporaryAccelerators.insert(txt);
  }
}

void KGameWindow::setBarFlagButton(const Player* player)
{
//   kDebug() << "KGameWindow::setBarFlagButton" << endl;

  if (player == 0)
  {
    if (currentPlayer() 
        && currentPlayer()-> getFlag())
    {
      m_goalAction-> setIcon(KIcon(currentPlayer()->getFlag()-> image(0)));
      m_goalAction-> setIconText(i18n("Goal"));
      m_barFlag-> setPixmap(currentPlayer()->getFlag()-> image(0));
      m_barFlag->show();
    }
  }
  else
  {
    if (player-> getFlag())
    {
      m_goalAction-> setIcon(KIcon(player-> getFlag()-> image(0)));
      m_goalAction-> setIconText(i18n("Goal"));
      m_barFlag-> setPixmap(player->getFlag()-> image(0));
      m_barFlag->show();
    }
  }
  m_frame->setFocus();
}

bool KGameWindow::setupPlayers()
{
  kDebug() << "KGameWindow::setupPlayers" << endl;
  
  // Number of players
  bool networkGame = false;
  int port;
  uint newPlayersNumber = 0;
  if (!m_automaton->setupPlayersNumberAndSkin(networkGame, port, newPlayersNumber))
  {
    return false;
  }
  
  if (!(m_automaton->playerList()->isEmpty()))
  {
    m_automaton->playerList()->clear();
    m_automaton->currentPlayer(0);
    kDebug() << "  playerList size = " << m_automaton->playerList()->count() << endl;
  }
  theWorld()->reset();
  clearGameActionsToolbar();
  
  std::map< QString, QString > nations = nationsList();
  if (!(m_automaton->playerList()->isEmpty()))
  {
    m_automaton->playerList()->clear();
  }
  kDebug() << "KGameWindow::setupPlayers: before switch; newPlayersNumber = " << newPlayersNumber << endl;
  unsigned int nbAvailArmies = (unsigned int)(m_theWorld->getNbCountries() * 2.5 / newPlayersNumber);
  kDebug() << "KGameWindow::setupPlayers: nbAvailArmies = " << nbAvailArmies << " ; nb countries = " << m_theWorld->getNbCountries() << endl;
  // Players names
  QString mes = "";
  QString nationName;
  for (unsigned int i = 0; 
       i < newPlayersNumber - m_automaton->networkPlayersNumber();
       i++)
  {
    QString nomEntre = "";
    bool computer;
    bool network = false;
    QString password;

    // After closing KPlayerSetupDialog, it is guaranteed, that nomEntre is a 
    // valid username (not empty, unique)
    KPlayerSetupDialog(m_automaton, m_theWorld, i+1, nomEntre, network,
                        password, computer, nations, nationName, this).exec();
    kDebug() << "Creating player " << nomEntre << "(computer: "
             << computer << "): " << nationName << endl;
    addPlayer(nomEntre, nbAvailArmies, 0, nationName, computer);
    nations.erase(nationName);
  }
  if (networkGame)
  {
    kDebug() << "In setupPlayers: networkGame" << endl;
    m_automaton->offerConnections(port);
    KMessageParts messageParts;
    messageParts << I18N_NOOP("Waiting for %1 players to connect")
      << QString::number(m_automaton->networkPlayersNumber());
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  }
  m_frame->setFocus();
  return true;
}

bool KGameWindow::setupOnePlayer()
{
  kDebug() << "KGameWindow::setupOnePlayer" << endl;
  
  kDebug() << "  building the list of available nations" << endl;
  std::map< QString, QString > nations = nationsList();
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    Player* player = (Player*)(*it);
    kDebug() << "    removing nation of player " << player-> name() << endl;
    /// Don't understand why if using Player::getNation below, the method is
    /// not executed (get 0) but if using the same named myNation, it works...
    /// Ideas anybody ????
    Nationality* nation = player-> getNation();
    std::cerr << "    got player nation " << nation << std::endl;
    QString nationName = nation->name();
    std::map<QString,QString>::iterator nationsIt;
    nationsIt = nations.find(nationName);
    if (nationsIt !=  nations.end())
    {
      nations.erase(nationsIt);
    }
  }
  kDebug() << "  number of available nations: " << nations.size() << endl;
  unsigned int nbAvailArmies = (unsigned int)(m_theWorld->getNbCountries() * 2.5 / (m_automaton->nbPlayers()));
  kDebug() << "KGameWindow::setupOnePlayer: nbAvailArmies = " << nbAvailArmies << " ; nb countries = " << m_theWorld->getNbCountries() << endl;
  // Players names
  QString mes = "";
  QString nationName;
    
  QString nomEntre = "";
  bool computer;
    
  bool found = true;
  while(found)
  {
    QString password;
    bool emptyName = true;
    while (emptyName)
    {
      mes = i18n("Player number %1, what's your name ?", 1);
      bool network = true;
      KPlayerSetupDialog(m_automaton, m_theWorld, 1, nomEntre, network, password, computer, nations, nationName, this).exec();
      kDebug() << "After KPlayerSetupDialog. name: " << nomEntre << endl;
      if (nomEntre.isEmpty())
      {
        mes = i18n("Error - Player %1, you have to choose a name.", 1);
        QMessageBox::warning(this, i18n("Error"), mes);
        nomEntre = i18n("Player%1", 1);
      }
      else 
      {
        emptyName = false;
      }
    }
    found = false;
    PlayersArray::iterator it = m_automaton->playerList()->begin();
    PlayersArray::iterator it_end = m_automaton->playerList()->end();
    for (; it != it_end; it++)
    {
      if ( (*it)-> name() ==  nomEntre)
      {
        found = true;
        it = it_end;
      }
    }
    if (!found)
    {
      kDebug() << "Creating player " << nomEntre 
        << "(computer: " << computer << "): " << nationName 
        << " password: " << password << endl;
      addPlayer(nomEntre, nbAvailArmies, 0, nationName, computer, password);
      nations.erase(nationName);
    }
  }
  return true;
}

bool KGameWindow::setupOneWaitedPlayer()
{
  kDebug() << "KGameWindow::setupOneWaitedPlayer" << endl;
  
  QString password;
  int result;
  KWaitedPlayerSetupDialog(m_automaton, password, result, this).exec();
  kDebug() << "After KWaitedPlayerSetupDialog. number: " << result << ", password: " << password << endl;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << (quint32)result << password;
  m_automaton->sendMessage(buffer,ValidateWaitedPlayerPassword);
  return true;
}

bool KGameWindow::createWaitedPlayer(quint32 waitedPlayerId)
{
  kDebug() << "KGameWindow::createWaitedPlayer" << endl;
  
  PlayerMatrix& pm = m_waitedPlayers[waitedPlayerId];
  Player* player = addPlayer(pm.name, pm.nbAvailArmies,
                              pm.nbCountries, pm.nation,
                              pm.isAI, pm.password,
                              pm.nbAttack, pm.nbDefense);

  player->goal(pm.goal);
  std::set<QString>::iterator it, it_end;
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
//   kDebug() << "KGameWindow::distributeArmies" << endl;
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    unsigned int nb = nbNewArmies(static_cast<Player*>(*it));
//     kDebug() << "    Giving " << nb << " armies to " << static_cast<Player*>(*it)->name() << endl;
    static_cast<Player*>(*it)-> setNbAvailArmies(nb);
  }
}

int KGameWindow::nbNewArmies(Player *player)
{
//    kDebug() << "KGameWindow::nbNewArmies for "  << player-> name() << endl;

  unsigned int res = 0;

  for (unsigned int i = 0; i<m_theWorld->getNbCountries(); i++)
      if (m_theWorld-> getCountries().at(i)-> owner() == player) res++;
  res = (res/3) < 3 ? 3 : res/3 ;

  std::vector<Continent*>& continents = m_theWorld-> getContinents();
  std::vector<Continent*>::iterator it = continents.begin();
  for (; it != continents.end(); it++)
  {
    Continent* currCont = *it;
    if (currCont-> owner() == player)
    {
//            kDebug() << ">>>>>>>>>>> Adding bonus for "  << currCont-> name() << endl;
      res += currCont-> getBonus();
    }
  }

  return res;
}

void KGameWindow::changeItem( const QString& text, int id, bool log )
{
  if (id != ID_NO_STATUS_MSG)
  {
      statusBar()-> changeItem(text, id);
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
//  kDebug() << "KGameWindow::changeItem(KMessageParts,int, log)" << log << endl;
  if (strings.begin() == strings.end())
  {
//     kDebug() << "  nothing " << strings.size() << endl;
    return;
  }
  bool arguing = false;
  KLocalizedString argument;
  KMessageParts::iterator it, it_end;
  it = strings.begin(); it_end = strings.end();
  if (it.curIsStr())
  {
    if (!it.curStr().isEmpty())
      argument = ki18n(it.curStr().toUtf8().data());
    else
      argument = KLocalizedString();
    arguing = true;
  }
  KsirkChatItem* item = new KsirkChatItem();
  if (it.curIsPix())
  {
    (*item) << it.curPix();
  }
  it++;
  for (; it != it_end; it++)
  {
    if (it.curIsStr())
    {
//       kDebug() << "argument current string: '" << it.curStr() << "'" << endl;
      if (arguing)
      {
//         kDebug() << "  substituting" << endl;
        argument=argument.subs(it.curStr());
      }
      else
      {
//         kDebug() << "  assigning new argument" << endl;
        if (!it.curStr().isEmpty())
          argument = ki18n(it.curStr().toUtf8().data());
        else
          argument = KLocalizedString();
        arguing = true;
      }
    }
    else
    {
      if (arguing)
      {
//         kDebug() << "  storing" << endl;
        if (argument.isEmpty())
        {
          (*item) << QString("");
        }
        else
        {
          (*item) << argument.toString();
        }
      }
      (*item) << it.curPix();
      arguing = false;
    }
  }
  if (arguing)
  {
    if (argument.isEmpty())
    {
      (*item) << QString("");
    }
    else
    {
      (*item) << argument.toString();
    }
  }
  if (log)
  {
    ((KsirkChatModel*)(m_chatDlg->model()))->addMessage(*item);
  }
  if (id != ID_NO_STATUS_MSG)
  {
    if (argument.toString() == "(I18N_EMPTY_MESSAGE)")
    {
      kError() << "received a (I18N_EMPTY_MESSAGE)" << endl;
    }
//     kDebug() << "  argument: " << argument.toString() << endl;
    statusBar()-> changeItem(argument.toString(), id);
  }
}

void KGameWindow::broadcastChangeItem ( KMessageParts& strings, int id, bool log )
{
  if (strings.empty())
  {
    return;
  }
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
//   kDebug() << "Broadcasting change item, size=" << strings.size() << endl;
  stream << (quint32)id << (quint32)log << (quint32)strings.size();
  
  /// @TODO finish the port to new KMessageParts
//   QString s;
//   strings >> s;
//   stream << m_automaton->idForMsg(s);
//   while (!strings.empty())
//   {
//     strings >> s;
//     kDebug() << "Pushing string '" << s << "'" << endl;
//     stream << s;
//   }

  
  KMessageParts::iterator it, it_end;
  it = strings.begin(); it_end = strings.end();
  if (it != it_end)
  {
    stream << m_automaton->idForMsg(it.curStr());
    it++;
    for (; it != it_end; it++)
    {
      if (it.curIsStr())
      {
//         kDebug() << "Pushing string '" << it.curStr() << "'" << endl;
        stream << KMessageParts::Text << it.curStr();
      }
      else if (it.curIsPix())
      {
        stream << KMessageParts::Pixmap << it.curPix();
      }
      else
      {
        kError() << "Unsuported KMessageParts elem type " << endl;
      }
    }
  }
  m_automaton->sendMessage(buffer,ChangeItem);
  changeItem( strings, id, log );
}

void KGameWindow::enterEvent(QEvent* /*ev*/)
{
//   kDebug() << "KGameWindow::enterEvent()" << endl;
  // Restart the AIs threads
  if ( currentPlayer() )
    if ( (currentPlayer()-> isAI()) && (!currentPlayer()->isVirtual()) && ( !((static_cast<AIPlayer*>(currentPlayer()))-> isRunning())) )
      (static_cast<AIPlayer*>(currentPlayer()))-> start();
          
}

void KGameWindow::leaveEvent(QEvent* /*ev*/)
{
//    kDebug() << "KGameWindow::leaveEvent()" << endl;
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

void KGameWindow::attack(Country& attacker, Country& defender, unsigned int nb)
{
  clearGameActionsToolbar();
  currentPlayer()-> setNbAttack( nb );
  KMessageParts messageParts;
  messageParts << I18N_NOOP("%1 attacks %2 from %3 with <font color=\"red\">%4 armies</font>.") 
    << currentPlayer()-> name() 
    << defender.name() 
    << attacker.name() 
    << QString::number(nb);
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
  

  messageParts.clear();
  if (! (defender.owner()->isAI()))
  {
    if ((m_secondCountry-> nbArmies() > 1)
        && (nb >= 2))
    {
      QPixmap pm = m_secondCountry->owner()->getFlag()->image(0);

      messageParts <<pm<< I18N_NOOP("%1, you are attacked by <font color=\"red\">%2 armies</font> ; with how many armies do you <font color=\"blue\">defend %3</font> ?") 
        << m_secondCountry->owner()->name() 
        << QString::number(nb)
        << m_secondCountry-> name();
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << m_secondCountry->owner()->name();
      m_automaton->sendMessage(buffer,DisplayDefenseButtons);
    }
    else
    {
    QPixmap pmA = m_firstCountry-> owner()->getFlag()->image(0);
    QPixmap pmD = m_secondCountry-> owner()->getFlag()->image(0);

    messageParts 
      << I18N_NOOP("Battle between <font color=\"red\">%1</font> (")
      << m_firstCountry-> name()
      << pmA
      << I18N_NOOP("%1) <font color=\"red\">with %2 armies</font> and <font color=\"blue\">%3</font> (")
      << m_firstCountry->owner()->name()
      << QString::number(nb)
      << m_secondCountry-> name()
      << pmD
      << I18N_NOOP("%1) <font color=\"blue\">with %2 armies</font>.") 
      << m_secondCountry->owner()->name()
      << QString::number(1);

      initCombatMovement(m_firstCountry, m_secondCountry);
    }
    broadcastChangeItem(messageParts, ID_NO_STATUS_MSG);
  }
  else
  {
  }
}

void KGameWindow::moveArmies(Country& src, Country& dest, unsigned int nb)
{
//    kDebug() << "KGameWindow::moveArmies()" << endl;
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
//    kDebug() << "OUT KGameWindow::moveArmies()" << endl;
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
    messageParts << I18N_NOOP("You are not the owner of the first country: %1 !")
            << m_firstCountry->name();
  }
  else if ( secondCountry->owner() != currentPlayer() )
  {
  messageParts << I18N_NOOP("You are not the owner of the second country: %1 !")
            << secondCountry->name();
  }
  else if (m_firstCountry == secondCountry)
  {
    messageParts << I18N_NOOP("You are trying to move armies from %1 to itself !")
                << m_firstCountry->name();
  }
  else if (!m_firstCountry->communicateWith(secondCountry))
  {
    messageParts 
      << I18N_NOOP("%1 is not a neighbour of %2 !") 
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

int KGameWindow::setCurrentPlayerToFirst()
{
//   kDebug() << "KGameWindow::setCurrentPlayerToFirst()" << endl;
  if (    ( currentPlayer() ) &&
            ( currentPlayer()-> isAI()) && ( static_cast<AIPlayer *>(currentPlayer())-> isRunning() ) )
        static_cast<AIPlayer *>(currentPlayer())-> stop();

  m_automaton->currentPlayer((Player*)(*m_automaton->playerList()->begin()));

  if ( currentPlayer() && currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()) )
      if ( ! ( static_cast<AIPlayer *>(currentPlayer())-> isRunning()) )
          static_cast<AIPlayer *>(currentPlayer())-> start();
//    kDebug() << "OUT KGameWindow::setCurrentPlayerToFirst()" << endl;
  m_frame->setFocus();
  return 0;
}


int KGameWindow::setCurrentPlayerToNext(bool restartRunningAIs)
{
  int looped(0);
//   kDebug() << "KGameWindow::setCurrentPlayerToNext()" << endl;
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

  if ( restartRunningAIs && currentPlayer() && currentPlayer()-> isAI() && (!currentPlayer()->isVirtual()) )
  {
    if ( ! (static_cast< AIPlayer* >(currentPlayer())-> isRunning()) )
    {
      static_cast< AIPlayer* >(currentPlayer())-> start();
    }
  }

//   kDebug() << "New current player is " << currentPlayer()->name() << " ; return value is " << looped << endl;
  return looped;
}

bool KGameWindow::terminateAttackSequence()
{
  m_firstCountry->clearHighlighting();
  m_secondCountry->clearHighlighting();
  m_animFighters->hideAndRemoveAll();
  gameActionsToolBar-> show();
  return attackEnd();
}

bool KGameWindow::attacker(const QPointF& point)
{
  kDebug() << endl;
  Country* clickedCountry = clickIn(point);
  KMessageParts messageParts;
  if (clickedCountry == 0)
  {
    messageParts << I18N_NOOP("<font color=\"orange\">No country here !</font>");
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    displayNormalGameButtons();
    return false;
  }

  if (clickedCountry-> owner() != currentPlayer())
  {
    messageParts << I18N_NOOP("<font color=\"orange\">You are not the owner of %1 !</font>")  << clickedCountry-> name();
    displayNormalGameButtons();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return false;
  }
  else if (clickedCountry-> nbArmies() <= currentPlayer()-> getNbAttack())
  {
    messageParts << I18N_NOOP("<font color=\"orange\">There is only %1 armies in %2 !</font>") 
      << QString::number(clickedCountry-> nbArmies())
      << clickedCountry-> name();
    displayNormalGameButtons();
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
    messageParts
      << pm
      << I18N_NOOP("%1 attacks from %2 with <font color=\"red\">%3 armies</font>") 
      << currentPlayer()-> name()
      << clickedCountry-> name()
      << QString::number(currentPlayer()-> getNbAttack());
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    return true;
  }
}

unsigned int KGameWindow::attacked(const QPointF& point)
{
  kDebug() << endl;
//  if (currentPlayer()-> isAI()) return 3;
        
  unsigned int res = 0;
  Country* secondCountry = clickIn(point);
  m_secondCountry = secondCountry;
  KMessageParts messageParts;

  kDebug() << "2nd country is now set" << endl;
  if ( (m_firstCountry == NULL) || (secondCountry == NULL)
          || (m_firstCountry-> owner() != currentPlayer()) )
  {
    messageParts << I18N_NOOP("Nothing to attack !");
    displayNormalGameButtons();
  }
  else if (!secondCountry-> owner()) 
  {
    messageParts << I18N_NOOP("Invalid attacked country.");
    displayNormalGameButtons();
  }
  else if (m_firstCountry == secondCountry)
  {
    messageParts << I18N_NOOP("You are trying to attack %1 from itself !") << m_firstCountry-> name();
    displayNormalGameButtons();
  }
  else if (!m_firstCountry-> communicateWith(secondCountry))
  {
    messageParts << I18N_NOOP("%1 is not a neighbour of %2 !") << secondCountry-> name() << m_firstCountry-> name();
    displayNormalGameButtons();
  }
  else if (m_firstCountry-> owner() == secondCountry-> owner())
  {
    messageParts << I18N_NOOP("%1! You cannot attack %2! It is yours!") << currentPlayer()-> name()
            << secondCountry-> name();
    displayNormalGameButtons();
  }
  else if (m_firstCountry-> owner() != currentPlayer()) 
  {
    messageParts << I18N_NOOP("%1 ! You are not the owner of %2!") << currentPlayer()-> name() << m_firstCountry-> name();
    displayNormalGameButtons();
  }
  else if ((secondCountry-> nbArmies() > 1)
      && (currentPlayer()-> getNbAttack() >= 2))
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (secondCountry != 0)
    {
      stream << secondCountry->name();
    }
    else
    {
      stream << QString("");
    }
    m_automaton->sendMessage(buffer,SecondCountry);

    messageParts
      << I18N_NOOP("%1, with how many armies do you defend %2 ?") 
      << secondCountry->owner()-> name()
      << secondCountry-> name();
    QByteArray buffer2;
    QDataStream stream2(&buffer2, QIODevice::WriteOnly);
    stream2 << secondCountry->owner()->name();
    m_automaton->sendMessage(buffer2,DisplayDefenseButtons);
    res = 1;
  }
  else
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    if (secondCountry != 0)
    {
      stream << secondCountry->name();
    }
    else
    {
      stream << QString("");
    }
    m_automaton->sendMessage(buffer,SecondCountry);
    messageParts
      << I18N_NOOP("%1, you defend %2 with its unique army.") 
      << secondCountry->owner()-> name()
      << secondCountry-> name();
    res = 2;
  }
  kDebug() << "will change item" << endl;
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  kDebug() << "change item broadcasted; returning " << res << endl;
  return res;
}

void KGameWindow::firstCountryAt(const QPointF& point)
{
  if (clickIn(point))
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << clickIn(point)->name();
    m_automaton->sendMessage(buffer,FirstCountry);
  }
}

void KGameWindow::secondCountryAt(const QPointF& point)
{
  if (clickIn(point))
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << clickIn(point)->name();
    m_automaton->sendMessage(buffer,SecondCountry);
  }
}

bool KGameWindow::playerPutsArmy(const QPointF& point, bool removable)
{
  kDebug() << "KGameWindow::playerPutsArmy" << endl;
  Country* clickedCountry = clickIn(point);

  if ( (clickedCountry) )
  {
    kDebug() << "clickedCountry name=" << clickedCountry->name()  << endl;
    kDebug() << "clickedCountry owner=" << clickedCountry-> owner()->name() << endl;
    kDebug() << "currentPlayer=" << currentPlayer()->name() << endl;
    kDebug() << "nbAvailArmies=" << currentPlayer()->getNbAvailArmies() << endl;
    unsigned int nbAvailArmies = currentPlayer()->getNbAvailArmies();
    if (
          (clickedCountry-> owner() == currentPlayer()) &&
          (nbAvailArmies > 0))
    {
      nbAvailArmies--;
      currentPlayer()-> setNbAvailArmies(nbAvailArmies);
      kDebug() << "owner new available armies=" << nbAvailArmies << endl;
      if (removable) clickedCountry-> incrNbAddedArmies();
      clickedCountry-> incrNbArmies();
      clickedCountry-> incrNbAddedArmies();
      clickedCountry-> createArmiesSprites(m_backGnd);
      QPixmap pm = currentPlayer()->getFlag()->image(0);
      KMessageParts messageParts;
      messageParts 
        << pm 
        << I18N_NOOP("%1 : %2 armies to place") 
        << currentPlayer()-> name()
        << QString::number(nbAvailArmies);
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);

      if (m_automaton->isAdmin())
      {
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream << currentPlayer()->id();
        m_automaton->sendMessage(buffer,CheckGoal);
        
      }
    }
  }
  return false;
}

bool KGameWindow::playerPutsInitialArmy(const QPointF& point)
{
  {
    kDebug() << "KGameWindow::playerPutsInitialArmy " << point << endl;
    Country* clickedCountry = clickIn(point);
    
    if ( (clickedCountry) )
    {
      kDebug() << "clickedCountry name=" << clickedCountry->name()  << endl;
      kDebug() << "clickedCountry owner=" << clickedCountry-> owner()->name() << endl;
      kDebug() << "currentPlayer=" << currentPlayer()->name() << endl;
      kDebug() << "m_nbAvailArmies=" << m_nbAvailArmies << endl;
      
      if (
           (clickedCountry-> owner() == currentPlayer()) &&
           (m_nbAvailArmies > 0))
      {
        m_nbAvailArmies--;
        bool last = (m_nbAvailArmies == 0);
        unsigned int currentAvailArmiesNumber = m_nbAvailArmies;
        currentPlayer()-> setNbAvailArmies(currentAvailArmiesNumber);
        kDebug() << "owner new available armies=" << m_nbAvailArmies << endl;
        clickedCountry-> incrNbArmies();
        clickedCountry-> incrNbAddedArmies();
        clickedCountry-> createArmiesSprites(m_backGnd);
        if ( last )
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

            KMessageParts messageParts;
            messageParts 
              << pm 
              << I18N_NOOP("%1 : %2 armies to place") << ((Player*)(*it))-> name()
              << QString::number(((Player*)(*it))-> getNbAvailArmies());
            broadcastChangeItem(messageParts, ID_STATUS_MSG2);
            m_nbAvailArmies = ((Player*)(*it))-> getNbAvailArmies();
            QByteArray buffer;
            QDataStream stream(&buffer, QIODevice::WriteOnly);
            stream << (quint32)m_nbAvailArmies;
            m_automaton->sendMessage(buffer,KGameWinAvailArmies);
          }
          if (m_automaton->isAdmin())
          {
            return setCurrentPlayerToNext();
          }
          else
          {
            return false;
          }
        }
        else
        {
          QPixmap pm = currentPlayer()->getFlag()->image(0);
          KMessageParts messageParts;
          messageParts << pm << I18N_NOOP("%1 : %2 armies to place") 
            << currentPlayer()-> name()
            << QString::number(m_nbAvailArmies);
          changeItem(messageParts, ID_STATUS_MSG2, false);
        }
      }
    }
    return false;
  }
}

bool KGameWindow::playerRemovesArmy(const QPointF& point)
{
  kDebug() << "KGameWindow::playerRemovesArmy" << endl;
  
  Country *clickedCountry = clickIn(point);
  kDebug() << "  currentPlayer=" << currentPlayer()->name() << endl;
  if (clickedCountry == 0)
  {
    return false;
  }
  kDebug() << "  owner=" << clickedCountry-> owner()->name() << endl;
  kDebug() << "  nbArmies=" << clickedCountry->nbArmies() << endl;
  kDebug() << "  nbAddedArmies=" << clickedCountry->nbAddedArmies() << endl;
  if ( clickedCountry
      && ( clickedCountry-> owner() == currentPlayer() )
      && ( clickedCountry-> nbArmies() > 1)
      && ( clickedCountry-> nbAddedArmies() >0 ) )
  {
    unsigned int newNbAvailArmies = currentPlayer()-> getNbAvailArmies() + 1;
    if ( m_automaton->isAdmin() )
    {
      currentPlayer()-> incrNbAvailArmies();
      QPixmap pm = currentPlayer()->getFlag()->image(0);
      KMessageParts messageParts;
      messageParts <<pm<< I18N_NOOP("%1 : %2 armies to place") << currentPlayer()-> name() 
        << QString::number(newNbAvailArmies);
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    }
    clickedCountry-> decrNbAddedArmies();
    clickedCountry-> decrNbArmies();
    clickedCountry-> createArmiesSprites(m_backGnd);
  }
  return true;
}

/**
  * @brief setups window for recycling
  */
void KGameWindow::initRecycling()
{
  kDebug() << "Initiating recycling" << endl;
  setCurrentPlayerToFirst();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  m_automaton->sendMessage(buffer,DisplayRecyclingButtons);
  
  KMessageParts messageParts;
  messageParts << I18N_NOOP("Exchange armies again or continue ?");
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
}

void KGameWindow::clear()
{
  m_nbMovedArmies = 0;
  m_firstCountry = m_secondCountry = 0;
  NKD = NKA = 0;
}

bool KGameWindow::nextPlayerRecycling()
{
  kDebug() << "nextPlayerRecycling" << endl;
  if ( currentPlayer() && currentPlayer()-> getNbAvailArmies() > 0 )
  {
    QMessageBox::information(0, "KsirK", i18n("You must distribute\nall your armies"));
    return false;
  }
  else
  {
    if (setCurrentPlayerToNext())
    {
      initRecycling();
      return true;
    }
    else
    {
      KMessageParts messageParts;
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      m_automaton->sendMessage(buffer,DisplayNextPlayerButton);
      QPixmap pm = currentPlayer()->getFlag()->image(0);
      messageParts <<pm<< I18N_NOOP("%1 : %2 armies to place") << currentPlayer()-> name() 
        << QString::number(currentPlayer()-> getNbAvailArmies());
      broadcastChangeItem(messageParts, ID_STATUS_MSG2);
      return false;
    }
  }
}

/** 
  * @return if true next state will be NEWARMIES else it will be WAIT
  */
bool KGameWindow::nextPlayerNormal()
{
  kDebug() << endl;
  if (setCurrentPlayerToNext())
  {
    distributeArmies();
  
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,ShowArmiesToPlace);
    
    clear();
    QByteArray buffer2;
    QDataStream stream2(&buffer2, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer2,DisplayNextPlayerButton);
    return true;
  }
  else
  {
    QPixmap pm = currentPlayer()->getFlag()->image(0);
    KMessageParts messageParts;
    messageParts 
      << pm
      << I18N_NOOP("%1, it is up to you.") << currentPlayer()->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    clear();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,DisplayNormalGameButtons);
    return false;
  }
}

void KGameWindow::attack(unsigned int nb)
{
  displayCancelButton();
  currentPlayer()-> setNbAttack(nb);
  KMessageParts messageParts;
  messageParts << I18N_NOOP("Attack with %1 armies : Designate the belligerants") 
    << QString::number(nb);
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  showMessage(i18n("To attack, press the mouse button in the attacking country<br/>and then <b>drag and drop</b> on its neighbour your want to attack."), 5);
}

void KGameWindow::defense(unsigned int nb)
{
  kDebug() << endl;
  clearGameActionsToolbar();

  if (!m_firstCountry) // anything left to do?
     return;

  m_secondCountry-> owner()-> setNbDefense(nb);
  KMessageParts messageParts;
  
  QPixmap pmA = m_firstCountry-> owner()->getFlag()->image(0);
  QPixmap pmD = m_secondCountry-> owner()->getFlag()->image(0);

  messageParts 
    << I18N_NOOP("Battle between <font color=\"red\">%1</font> (")
    << m_firstCountry-> name()
    << pmA
    << I18N_NOOP("%1) <font color=\"red\">with %2 armies</font> and <font color=\"blue\">%3</font> (")
    << m_firstCountry->owner()->name()
    << QString::number(currentPlayer()-> getNbAttack())
    << m_secondCountry-> name()
    << pmD
    << I18N_NOOP("%1) <font color=\"blue\">with %2 armies</font>.") 
    << m_secondCountry->owner()->name()
    << QString::number(nb);

  broadcastChangeItem(messageParts, ID_NO_STATUS_MSG);

  if (m_firstCountry-> owner() && m_firstCountry-> owner()-> getFlag())
  {
    m_goalAction-> setIcon(KIcon(m_firstCountry-> owner()->getFlag()-> image(0)));
    m_goalAction-> setIconText(i18n("Goal"));
    m_barFlag-> setPixmap(m_firstCountry-> owner()->getFlag()-> image(0));
  }
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  m_automaton->sendMessage(buffer,InitCombatMovement);
  m_automaton->state(GameLogic::GameAutomaton::FIGHT_BRING);
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
  bool res = initArmiesMovement(nb, m_firstCountry, m_secondCountry);
  kDebug() << "invade("<<nb<<") returns " << res << endl;
  return res;
}

bool KGameWindow::retreat(unsigned int nb)
{
    if (m_nbMovedArmies >= int(nb))
      return initArmiesMovement(nb, m_secondCountry, m_firstCountry);
    else
    {
      KMessageParts messageParts;
      messageParts << I18N_NOOP("<font color=\"orange\">Cannot remove %1 armies from %2</font>: %3 maximum.") 
        << QString::number(nb)
        << m_secondCountry->name() 
        << QString::number(m_nbMovedArmies);
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
      return false;
    }
}

void KGameWindow::invasionFinished()
{
  displayNormalGameButtons();
  KMessageParts messageParts;

  QPixmap pm = currentPlayer()->getFlag()->image(0);
  messageParts 
    << pm 
    << I18N_NOOP("%1, it is up to you.") << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
}
        
void KGameWindow::shiftFinished()
{
  displayNormalGameButtons();
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
  kDebug() << "KGameWindow::cancelAction" << endl;
  displayNormalGameButtons();
  KMessageParts messageParts;
  QPixmap pm = currentPlayer()->getFlag()->image(0);
  messageParts 
    << pm 
    << I18N_NOOP("%1, it is up to you.") << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
}

void KGameWindow::cancelShiftSource()
{
//        clearGameActionsToolbar();
  if (m_nbMovedArmies < 0)
  {
    m_firstCountry-> decrNbArmies(m_nbMovedArmies);
    m_secondCountry-> incrNbArmies(m_nbMovedArmies);
    m_firstCountry-> createArmiesSprites(m_backGnd);
    m_secondCountry-> createArmiesSprites(m_backGnd);
    m_nbMovedArmies = 0;
  }
  if (m_nbMovedArmies > 0)
  {
    m_firstCountry-> incrNbArmies(m_nbMovedArmies);
    m_secondCountry-> decrNbArmies(m_nbMovedArmies);
    m_firstCountry-> createArmiesSprites(m_backGnd);
    m_secondCountry-> createArmiesSprites(m_backGnd);
    m_nbMovedArmies = 0;
  }
}

bool KGameWindow::actionNewGame()
{
//   kDebug() << "KGameWindow::actionNewGame()" << endl;
  if  ( ( m_automaton->playerList()->count() == 0 ) ||
  ( isMyState(GameLogic::GameAutomaton::GAME_OVER)  ) ||
        (KMessageBox::warningContinueCancel(this,i18n("Do you really want to end your current game and start a new one ?"),i18n("New game confirmation"),KStandardGuiItem::yes()) == KMessageBox::Continue ) )

  {
    // @todo if new game is canceled, removed buttons should be displayed again
//    clearGameActionsToolbar();
/*    if (!(m_automaton->playerList()->isEmpty()))
    {
      m_automaton->playerList()->clear();
      m_automaton->currentPlayer(0);
      kDebug() << "  playerList size = " << m_automaton->playerList()->count() << endl;
    }
    theWorld()->reset();*/
    m_automaton->setGameStatus(KGame::End);
    m_automaton->state(GameLogic::GameAutomaton::INIT);
    m_automaton->savedState(GameLogic::GameAutomaton::INVALID);
    return (setupPlayers());
  }
  return false;
}

void KGameWindow::saveXml(std::ostream& xmlStream)
{
  xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  xmlStream << "<ksirkSavedGame formatVersion=\"" << SAVE_GAME_FILE_FORMAT_VERSION << "\" >" << std::endl;
  xmlStream 
    << "<game skin=\"" 
    << m_automaton->skin().toUtf8().data()
    << "\" state=\"" 
    << m_automaton->state()
    << "\" >" 
    << std::endl;
  m_theWorld->saveXml(xmlStream);
  xmlStream << "<players nb=\""<<m_automaton->playerList()->count()<<"\">" << std::endl;
  PlayersArray::iterator it = m_automaton->playerList()->begin();
  PlayersArray::iterator it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    static_cast<Player*>(*it)->saveXml(xmlStream);
  }
  xmlStream << "</players>" << std::endl;
  Player* player = m_automaton->currentPlayer();
  if (player)
  {
    QString name = player->name().toUtf8();
    name = name.replace("&","&amp;");
    name = name.replace("<","&lt;");
    name = name.replace(">","&gt;");
    xmlStream << "<currentPlayer name=\"" << name.toUtf8().data() << "\" />" << std::endl;
  }
  else
    xmlStream << "<currentPlayer name=\"\" />" << std::endl;
  xmlStream << "<goals>\n";
  it = m_automaton->playerList()->begin();
  it_end = m_automaton->playerList()->end();
  for (; it != it_end; it++)
  {
    static_cast<Player*>(*it)->goal().saveXml(xmlStream);
  }
  xmlStream << "</goals>\n";
  xmlStream << "</game>" << std::endl;
  xmlStream << "</ksirkSavedGame>" << std::endl;
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
  kDebug() << "Adding player (AI: " << isAI << ")" << endl;
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
    p->setNbAvailArmies(nbAvailArmies);
    p->setNbAttack(nbAttack);
    p->setNbDefense(nbDefense);
    p->setPassword(password);
    if (!m_automaton->addPlayer(p))
        p = 0; // freed - weired API
  }
  return p;
}

std::map< QString, QString > KGameWindow::nationsList()
{
  std::map< QString, QString >  res;
  
  std::vector<Nationality*>& nationsList = m_theWorld->getNationalities();
  kDebug() << "There is " << nationsList.size() << " nations" << endl;
  std::vector<Nationality*>::iterator nationsIt = nationsList.begin();
  for (; nationsIt != nationsList.end(); nationsIt++ ) 
  {
    Nationality* nation = *nationsIt;
    kDebug() << "Nation '" << nation->name() << "' = " << nation << endl;
    res.insert(std::make_pair(nation->name(),nation->flagFileName()));
  } 
  return res;
}

/** @return true if the given player is the last one ; false otherwise */
bool KGameWindow::isLastPlayer(const Player& player)
{
  if (m_automaton->playerList()->begin() == m_automaton->playerList()->end())
  {
    kError() << "No player ; should not be able to call isLastPlayer !" << endl;
    exit(1);
  }
  PlayersArray::iterator it = m_automaton->playerList()->end();
  //  it--;
  Player* lastPlayer = static_cast<Player*>(*it);
  return (player == (*lastPlayer));
}

void KGameWindow::actionRecycling()
{
//   kDebug() << "actionRecycling" << endl;
  setCurrentPlayerToFirst();
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  m_automaton->sendMessage(buffer,DisplayNextPlayerButton);
  KMessageParts messageParts;
  QPixmap pm = currentPlayer()->getFlag()->image(0);
  messageParts 
    << pm 
    << I18N_NOOP("%1, please change your distributions.") 
    << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
}

void KGameWindow::actionRecyclingFinished()
{
//   kDebug() << "actionRecyclingFinished" << endl;
  if (m_automaton->isAdmin())
  {
    for (unsigned int i = 0; i < m_theWorld->getNbCountries(); i++) 
    {
      m_theWorld-> getCountries().at(i)-> nbAddedArmies(0);
    }
    QPixmap pm = currentPlayer()->getFlag()->image(0);
    KMessageParts messageParts;
    messageParts 
      << pm
      << I18N_NOOP("%1, it is up to you.") << currentPlayer()->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    m_automaton->sendMessage(buffer,DisplayNormalGameButtons);
    m_automaton->state(GameLogic::GameAutomaton::WAIT);
  }
}

void KGameWindow::finishMoves()
{
  m_animFighters->moveAllToDestinationNow();
}

void KGameWindow::displayButtonsForState(GameAutomaton::GameState state)
{
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  switch (state)
  {
  case GameLogic::GameAutomaton::WAIT:;
    m_automaton->sendMessage(buffer,DisplayNormalGameButtons);
    break;
  case GameLogic::GameAutomaton::WAIT_RECYCLING:;
    m_automaton->sendMessage(buffer,DisplayNextPlayerButton);
    break;
  case GameLogic::GameAutomaton::NEWARMIES:;
    m_automaton->sendMessage(buffer,DisplayNextPlayerButton);
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
    m_automaton->sendMessage(buffer,DisplayNormalGameButtons);
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
  

  dialog->show();
}

void KGameWindow::explain()
{
  KMessageParts message0Parts;
  message0Parts << "<b>KsirK quick Introduction</b>";
  broadcastChangeItem(message0Parts, ID_NO_STATUS_MSG);

  KMessageParts message1Parts;
  message1Parts << I18N_NOOP("All commands are issued through changing buttons and drag & drop.");
  broadcastChangeItem(message1Parts, ID_NO_STATUS_MSG);

  KMessageParts message2Parts;
  message2Parts << I18N_NOOP("Start a new game or join a network game with the menu or the toolbar...");
  broadcastChangeItem(message2Parts, ID_NO_STATUS_MSG);

//   QString newGameImageFileName = m_dirs-> findResource("appdata", m_automaton->skin() + '/' + CM_NEWGAME);
//   if (newGameImageFileName.isNull())
//   {
//     QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
//     exit(2);
//   }
//   QPixmap newGameButtonPix(newGameImageFileName); 
//   KMessageParts message2Parts;
//   message2Parts << "\t" << newGameButtonPix << I18N_NOOP(" to start a new game;");
//   broadcastChangeItem(message2Parts, ID_NO_STATUS_MSG);
// 
//   QString joinGameImageFileName = m_dirs-> findResource("appdata", m_automaton->skin() + '/' + CM_NEWNETGAME);
//   if (joinGameImageFileName.isNull())
//   {
//     QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
//     exit(2);
//   }
//   QPixmap joinGameButtonPix(joinGameImageFileName); 
//   KMessageParts message3Parts;
//   message3Parts << "\t" << joinGameButtonPix << I18N_NOOP(" to join a network game (starting or reloading); and");
//   broadcastChangeItem(message3Parts, ID_NO_STATUS_MSG);
// 
//   QString openGameImageFileName = m_dirs-> findResource("appdata", m_automaton->skin() + '/' + CM_OPENGAME);
//   if (openGameImageFileName.isNull())
//   {
//     QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
//     exit(2);
//   }
//   QPixmap openGameButtonPix(openGameImageFileName); 
//   KMessageParts message4Parts;
//   message4Parts << "\t" << openGameButtonPix << I18N_NOOP(" to open a saved game.");
//   broadcastChangeItem(message4Parts, ID_NO_STATUS_MSG);

  KMessageParts message5Parts;
  message5Parts << I18N_NOOP("and then let the system guide you through messages and tooltips appearing on buttons when hovering above them.");
  broadcastChangeItem(message5Parts, ID_NO_STATUS_MSG);
}

void KGameWindow::showMessage(const QString& message, quint32 delay)
{
  kDebug() << endl;
  if (m_message == 0)
  {
    m_message  = new KGamePopupItem();
    m_scene->addItem(m_message);
    m_message->setSharpness(KGamePopupItem::Soft);
    m_message->setBackgroundBrush(Qt::blue);
    m_message->setZValue(1000);
  }
  m_message->setMessageTimeout(delay*1000);
  m_message->showMessage(message, KGamePopupItem::TopLeft, KGamePopupItem::ReplacePrevious);

//   m_message->setPos(m_frame-> mapToScene(QPoint(30,30)));
}


void KGameWindow::firstCountry(GameLogic::Country* country)
{
  kDebug() << endl;
  if (m_firstCountry != 0)
  {
    m_firstCountry->clearHighlighting();
  }
  kDebug() << endl;
  m_firstCountry = country;
  if (country == 0)
  {
    return;
  }
  kDebug() << country->name() << endl;
  country->highlightAsAttacker();
}

void KGameWindow::secondCountry(GameLogic::Country* country)
{
  kDebug() << endl;
  if (m_secondCountry != 0)
  {
    m_secondCountry->clearHighlighting();
  }
  kDebug() << endl;
  m_secondCountry = country;
  if (country == 0)
  {
    return;
  }
  kDebug() << country->name() << endl;
  country->highlightAsDefender();
}


} // closing namespace Ksirk

#include "kgamewin.moc"
