/* This file is part of KsirK.
   Copyright (C) 2001-2007 Gaël de Chalendar <kleag@free.fr>

   KsirK is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*  begin                : mer jui 11 22:27:28 EDT 2001   */

// application specific includes
#include "kgamewin.h"
#include "ksirkConfigDialog.h"
#include "ksirksettings.h"
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


// STL include files
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>

// include files for QT
#include <qmetaobject.h>
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qregexp.h>
#include <qcursor.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <QPixmap>
#include <QDockWidget>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardgameaction.h>
#include <kstandarddirs.h>
#include <kmenubar.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <phonon/audioplayer.h>
#include <KPushButton>
#include <kchatdialog.h>
#include <kgame/kgamechat.h>
#include <kglobal.h>
#include <KStatusBar>
#include <KToolBar>

#include <assert.h>

namespace Ksirk
{
using namespace GameLogic;

// port all occurrences of m_accels
// port all occurrences of setBarPos

KGameWindow::KGameWindow(QWidget* parent) :
  KMainWindow(parent), NKD(0), NKA(0), 
  m_theWorld(0), m_scene(0), m_backGnd(0),
  m_animFighters(new AnimSpritesGroup(this,SLOT(slotMovingFightersArrived(AnimSpritesGroup*)))),
  m_nbMovedArmies(0),
  m_firstCountry(0), m_secondCountry(0),
  m_frame(0),
  m_barFlagButton(new KPushButton(KIcon(), "", 0)),
//   m_accels(this), 
//   m_chat(0), 
  m_chatDlg(0),
  m_audioPlayer(0),//new Phonon::AudioPlayer( Phonon::NotificationCategory )),
  m_timer(this)
{
  kDebug() << "KGameWindow constructor begin" << endl;
  statusBar()->addWidget(m_barFlagButton);

  m_config=KGlobal::config();
  m_dirs = KGlobal::dirs();
//   m_accels.setEnabled(true);
  
  kDebug() << "Setting up std game actions" << endl;
  KStandardGameAction::gameNew( this, SLOT( slotNewGame() ), this );
  KStandardGameAction::save( this, SLOT( slotSaveGame() ), this );
  KStandardGameAction::load( this, SLOT( slotOpenGame() ), this );
  KStandardGameAction::quit( this, SLOT( close() ), this );
  
  QString iconFileName = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Images/SoldatAGenoux1.png");
  if (iconFileName.isNull())
  {
      QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load icon\nProgram cannot continue"));
      exit(2);
  }
  QPixmap icon(iconFileName);

  m_bottomDock = new QDockWidget();
  KsirkChatModel* chatModel = new KsirkChatModel(this);
  KsirkChatDelegate* chatDelegate = new KsirkChatDelegate(this);
  m_chatDlg = new KGameChat(&GameAutomaton::changeable(), 10000, this,chatModel,chatDelegate);
  connect(m_chatDlg,
          SIGNAL(signalReturnPressed(const QString&)),
          this,
          SLOT(slotChatMessage()));
  m_bottomDock->setWidget(m_chatDlg);
  m_bottomDock->setAllowedAreas(Qt::BottomDockWidgetArea);
  addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock); // master dockwidget

  kDebug() << "Creating automaton" << endl;
  GameAutomaton::changeable().init(this);
  
  kDebug() << "Setting skin" << endl;
  GameLogic::GameAutomaton::changeable().skin(m_config->readEntry("skin", "skins/default"));
//    kDebug() << "Before initActions" << endl;
//   initActions();
//    kDebug() << "Before initStatusBar" << endl;
  initStatusBar();
  
  menuBar()-> hide();
  
  displayOpenGameButton();
  
  connect(m_barFlagButton, SIGNAL(clicked()), this, SLOT(slotShowGoal()));
  explain();
  GameLogic::GameAutomaton::changeable().run();
  setMouseTracking(true);

  m_timer.setSingleShot(true);
  connect(&m_timer,SIGNAL(timeout()),this,SLOT(evenementTimer()));
}

KGameWindow::~KGameWindow()
{
  m_dirs = 0;
  m_config = 0;
  delete m_backGnd; m_backGnd = 0;
  delete m_scene; m_scene = 0;
  if (m_barFlagButton) {delete m_barFlagButton; m_barFlagButton = 0;}
  delete m_frame; m_frame = 0;
  delete m_audioPlayer;
}

void KGameWindow::initActions()
{
  kDebug() << "Adding exit toolBar button" << endl;
  addAButton(CM_EXITGAME, SLOT(close()), i18n("Exit"), KShortcut(Qt::ALT+Qt::Key_F4), false, "mainToolBar");
  kDebug() << "Adding new game toolBar button" << endl;
  addAButton(CM_NEWGAME, SLOT(slotNewGame()), i18n("New game"), KShortcut(Qt::CTRL+Qt::Key_N), false, "mainToolBar");
  kDebug() << "Adding new net game toolBar button" << endl;
  addAButton(CM_NEWNETGAME, SLOT(slotJoinNetworkGame()), i18n("Join network game"), KShortcut(Qt::CTRL+Qt::Key_J), false, "mainToolBar");
  kDebug() << "Adding info toolBar button" << endl;
  addAButton(CM_PREFERENCES, SLOT(optionsConfigure()), i18n("Preferences"), KShortcut(), false, "mainToolBar");
  addAButton(CM_INFO, SLOT(slotShowAboutApplication()), i18n("About"), KShortcut(), false, "mainToolBar");
}

void KGameWindow::initStatusBar()
{
  statusBar()-> setSizeGripEnabled(true);
  statusBar()->insertItem("", ID_STATUS_MSG, 2);
  statusBar()-> setItemAlignment(ID_STATUS_MSG, Qt::AlignLeft | Qt::AlignVCenter);
  statusBar()->insertItem("", ID_STATUS_MSG2, 3);
  statusBar()-> setItemAlignment(ID_STATUS_MSG2, Qt::AlignLeft | Qt::AlignVCenter);
  statusBar()->addWidget(m_barFlagButton);
}

Country* KGameWindow::clickIn(const QPointF &pointf)
{
//   kDebug() << "KGameWindow::clickIn " << pointf << endl;

  return m_theWorld-> countryAt( pointf );
}

Player* KGameWindow::currentPlayer()
{
//   kDebug() << "KGameWindow::currentPlayer" << endl;
  Player* current = GameLogic::GameAutomaton::changeable().currentPlayer();

  return current;
}

void KGameWindow::loadDices()
{
//   kDebug() << "KGameWindow::loadDices" << endl;
  
  m_dices[Blue] = std::vector<QPixmap>(6);
  m_dices[Red] = std::vector<QPixmap>(6);
  QString dicesDir = m_dirs->findResourceDir("appdata", GameAutomaton::changeable().skin() + "/Images/reddice1.png") + GameAutomaton::changeable().skin() + "/Images/";
  
  m_dices[Blue][0] = QPixmap(dicesDir+"/bluedice1.png");
  m_dices[Blue][1] = QPixmap(dicesDir+"/bluedice2.png");
  m_dices[Blue][2] = QPixmap(dicesDir+"/bluedice3.png");
  m_dices[Blue][3] = QPixmap(dicesDir+"/bluedice4.png");
  m_dices[Blue][4] = QPixmap(dicesDir+"/bluedice5.png");
  m_dices[Blue][5] = QPixmap(dicesDir+"/bluedice6.png");
  m_dices[Red][0] = QPixmap(dicesDir+"/reddice1.png");
  m_dices[Red][1] = QPixmap(dicesDir+"/reddice2.png");
  m_dices[Red][2] = QPixmap(dicesDir+"/reddice3.png");
  m_dices[Red][3] = QPixmap(dicesDir+"/reddice4.png");
  m_dices[Red][4] = QPixmap(dicesDir+"/reddice5.png");
  m_dices[Red][5] = QPixmap(dicesDir+"/reddice6.png");
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
//   clear();
  if (m_theWorld != 0)
  {
    m_theWorld-> reset();
    delete m_theWorld;
    m_theWorld = 0;
  }

  QString onuDefinitionFileName = onuFileName;
  if (onuDefinitionFileName.isEmpty())
  {
    onuDefinitionFileName = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Data/onu.xml");
  }
  if (onuDefinitionFileName.isEmpty())
  {
      QMessageBox::critical(0, i18n("Error !"),
          i18n("UNO definition XML file not found - Verify your installation\nProgram cannot continue"));
      exit(2);
  }
  kDebug() << "Got ONU definition file name: " <<  onuDefinitionFileName << endl;
  m_theWorld = new ONU(onuDefinitionFileName);
  if (m_frame != 0)
  {
//     haltTimer();
    m_frame->hide();
    delete m_frame;
  }
  loadDices();

  m_frame = new DecoratedGameFrame(this, m_theWorld->width(), m_theWorld->height());
  setCentralWidget(m_frame);
  m_frame->setMaximumWidth(m_theWorld->width());
  m_frame->setMaximumHeight(m_theWorld->height());
  m_frame->setCacheMode( QGraphicsView::CacheBackground );

  kDebug() << "ONU backgnd file name: " <<  m_theWorld->mapFileName() << endl;
  
  if (m_scene != 0)
  {
    delete m_scene;
  }

  m_scene = new QGraphicsScene(0, 0, m_theWorld->width(), m_theWorld->height(),this);
//   m_scene->setDoubleBuffering(true);
  kDebug() << "Before initView" << endl;
  initView();
  kDebug() <<"Before m_backGnd" << endl;
  m_backGnd = new BackGnd(m_scene, m_theWorld); //Creation of the background
//   m_frame->fitInView(m_backGnd, Qt::KeepAspectRatio);
  kDebug() <<"Before paint" << endl;
//   paint();
  
  kDebug() << "Setting up GUI" << endl;
  setupGUI();

//   initTimer();
  kDebug() <<"End new skin" << endl;

  initActions();
  toolBar("mainToolBar")-> hide();
  toolBar("gameActionsToolBar")-> hide();
  toolBar("mainToolBar")-> show();
  toolBar("gameActionsToolBar")-> show();
//   toolBar("mainToolBar")-> setBarPos(KToolBar::Bottom);
//   toolBar("gameActionsToolBar")-> setBarPos(KToolBar::Bottom);
  m_frame->setFocus();
}

void KGameWindow::initView()
{
  QString iconFileName = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Images/SoldatAGenoux1.png");
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
  kDebug() << "KGameWindow::attackEnd" << endl;
  if (m_firstCountry==0 || m_secondCountry == 0)
  {
    return false;
  }
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
    
    if (GameLogic::GameAutomaton::changeable().isAdmin())
    {
      currentPlayer()-> incrNbCountries();
      oldOwner-> decrNbCountries();
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << currentPlayer()->id();
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,CheckGoal);
      
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
      if (GameLogic::GameAutomaton::changeable().isAdmin())
      {
        kDebug() << "Removing player " << oldOwner-> name() << endl;
        int i = GameLogic::GameAutomaton::changeable().playerList()->indexOf(oldOwner);
        if (i != -1)
          delete GameLogic::GameAutomaton::changeable().playerList()->takeAt(i);
        kDebug() << "There is now " << GameLogic::GameAutomaton::changeable().playerList()->count() << " GameLogic::GameAutomaton::changeable().playerList()->" << endl;
      }
      if ( ( (GameLogic::GameAutomaton::changeable().useGoals()) 
             && ( (currentPlayer()->goal()->type() == GameLogic::Goal::GoalPlayer) 
             && ( (*currentPlayer()->goal()->players().begin()) == oldOwnerId) ) ) 
           || (GameLogic::GameAutomaton::changeable().playerList()->count() == 1) )
      {
//         haltTimer();
        GameLogic::GameAutomaton::changeable().state(GameLogic::GameAutomaton::GAME_OVER);
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream << currentPlayer()->id();
        GameLogic::GameAutomaton::changeable().sendMessage(buffer,Winner);
        return res;
      }
    }
  }
  m_firstCountry-> createArmiesSprites(m_backGnd);
  m_secondCountry-> createArmiesSprites(m_backGnd);
  if (GameLogic::GameAutomaton::changeable().isAdmin())
  {
    if (res)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayInvasionButtons);
    }
    else
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNormalGameButtons);
      KMessageParts messageParts;
      messageParts << I18N_NOOP("%1 : it's up to you again") << currentPlayer()-> name();
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
  if (GameLogic::GameAutomaton::changeable().useGoals())
  {
    msg += QString("<br/>Winner's goal was stated like this:<br/><i>") 
        += player->goal()->message()
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
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DecrNbArmies);
    NKD++;
  }
  else
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << m_firstCountry->name() << quint32(1);
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DecrNbArmies);
    NKA++;
  }
  if ((A2>0)&&(D2>0))
  {
    if (A2>D2)
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << m_secondCountry->name() << quint32(1);
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,DecrNbArmies);
      NKD++;
    }
    else
    {
      QByteArray buffer;
      QDataStream stream(&buffer, QIODevice::WriteOnly);
      stream << m_firstCountry->name() << quint32(1);
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,DecrNbArmies);
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
  GameLogic::GameAutomaton::changeable().sendMessage(buffer2,AnimExplosion);
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
//   kDebug() << "KGameWindow::actionOpenGame" << endl;
//   haltTimer();
  QString fileName = KFileDialog::getOpenFileName(KUrl(), "*.xml", this, i18n("KsirK - Load Game")); 
  if (fileName != "")
  {
    m_waitedPlayers.clear();
    Ksirk::SaveLoad::GameXmlLoader loader(fileName, *this, m_waitedPlayers);
    for (unsigned int i = 0; i < m_theWorld->getNbCountries(); i++)
    {
      Country* country = m_theWorld-> getCountries().at(i);
//       kDebug() << "Adding sprites to " << country->name() << endl;    
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
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNormalGameButtons);
      GameLogic::GameAutomaton::changeable().setGameStatus(KGame::Run);
//       kDebug() << "slotOpenGame before initTimer" << endl;
//       initTimer();
//       kDebug() << "slotOpenGame after initTimer" << endl;
      m_frame->setFocus();
      return false;
    }
    else
    {
      KMessageParts messageParts;
      messageParts << I18N_NOOP("Waiting for the connection of %1 network players.") << QString::number(m_waitedPlayers.size());
      broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
      return true;
    }
  }
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
}

void KGameWindow::displayRecyclingButtons()
{
//   kDebug() << "KGameWindow::displayRecyclingButtons" << endl;
  clearGameActionsToolbar(false);
  if (GameLogic::GameAutomaton::changeable().allLocalPlayersComputer())
  {
//     kDebug() << "There is only computer local players" << endl;
    PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
    PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
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
}

void KGameWindow::displayOpenGameButton()
{
  clearGameActionsToolbar();
  addAButton(CM_OPENGAME, SLOT(slotOpenGame()), i18n("Open game"),KShortcut(Qt::CTRL+Qt::Key_O),true);
}

void KGameWindow::displayNormalGameButtons()
{
//   kDebug() << "KGameWindow::displayNormalGameButtons" << endl;
  clearGameActionsToolbar(false);
  if (currentPlayer() && currentPlayer()-> isAI() && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
  else if (!currentPlayer()->isVirtual())
  {
    addAButton(CM_OPENGAME, SLOT(slotOpenGame()), i18n("Open game"),KShortcut(Qt::CTRL+Qt::Key_O),true);
    addAButton(CM_SAVEGAME, SLOT(slotSaveGame()), i18n("Save game"),KShortcut(Qt::CTRL+Qt::Key_S),true);
    addAButton(CM_NEXTPLAYER,  SLOT(slotNextPlayer()), i18n("Next Player"),KShortcut(Qt::Key_Escape),true);
    addAButton(CM_ATTACK1,  SLOT(slotAttack1()), i18n("Attack with one army"),KShortcut(Qt::Key_1),true);
    addAButton(CM_ATTACK2,  SLOT(slotAttack2()), i18n("Attack with two armies"),KShortcut(Qt::Key_2),true);
    addAButton(CM_ATTACK3,  SLOT(slotAttack3()), i18n("Attack with three armies"),KShortcut(Qt::Key_3),true);
    addAButton(CM_SHIFT, SLOT(slotMove()), i18n("Move armies"),KShortcut(Qt::Key_M),true);
  }
}

void KGameWindow::displayDefenseButtons()
{
  clearGameActionsToolbar(false);
  if (currentPlayer() && currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
  if (! (m_secondCountry-> owner()->isAI() ))
  {
    if (m_secondCountry && m_secondCountry-> owner() && m_secondCountry-> owner()-> getFlag())
        m_barFlagButton-> setIcon(KIcon(m_secondCountry-> owner()->getFlag()-> image(0)));
    addAButton(CM_DEFENSE1, SLOT(slotDefense1()), i18n("Defend with one army"),KShortcut(Qt::Key_1),true);
    addAButton(CM_DEFENSE2, SLOT(slotDefense2()), i18n("Defend with two armies"),KShortcut(Qt::Key_2),true);
  }
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
}

void KGameWindow::displayCancelButton()
{
  clearGameActionsToolbar();
  if (currentPlayer() && currentPlayer()-> isAI()  && (!currentPlayer()->isVirtual()))
  {
    if (!(static_cast<AIPlayer *>(currentPlayer()))-> isRunning()) (static_cast<AIPlayer *>(currentPlayer()))-> start();
  }
  else addAButton(CM_CANCEL, SLOT(slotCancel()), i18n("Cancel"), KShortcut(Qt::Key_Escape), true);
}

void KGameWindow::clearGameActionsToolbar(bool send)
{
//   kDebug()<< "KGameWindow::clearGameActionsToolbar " << send << endl;
  if (send)
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,ClearGameActionsToolbar);
  }
  
  (*toolBar("gameActionsToolBar")).clear();
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
//   kDebug() << "addAButton: " << fileName << endl;
  QString imageFileName = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/" + fileName);
//   kDebug() << "Trying to load button image file: " << imageFileName << endl;
  if (imageFileName.isNull())
  {
    QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
    exit(2);
  }
  (*toolBar(toolBarName.c_str())).addAction(QPixmap(imageFileName), 
      txt, this,  slot);
//   kDebug() << "Button added " << txt << endl;
  if (shortcut != KShortcut())
  {
    QString str = " ";
    if (txt != "")
    {
      str = txt;
    }
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
      m_barFlagButton-> setIcon(KIcon(currentPlayer()->getFlag()-> image(0)));
      m_barFlagButton->show();
    }
  }
  else
  {
    if (player-> getFlag())
    {
      m_barFlagButton-> setIcon(KIcon(player-> getFlag()-> image(0)));
      m_barFlagButton->show();
    }
  }
  m_frame->setFocus();
}

bool KGameWindow::setupPlayers()
{
  kDebug() << "KGameWindow::setupPlayers" << endl;
//   haltTimer();
  
  // Number of players
  bool networkGame = false;
  int port;
  uint newPlayersNumber = 0;
  if (!GameAutomaton::changeable().setupPlayersNumberAndSkin(networkGame, port, newPlayersNumber))
  {
    return false;
  }
  
  if (!(GameLogic::GameAutomaton::changeable().playerList()->isEmpty()))
  {
    GameLogic::GameAutomaton::changeable().playerList()->clear();
    GameLogic::GameAutomaton::changeable().currentPlayer(0);
    kDebug() << "  playerList size = " << GameLogic::GameAutomaton::changeable().playerList()->count() << endl;
  }
  theWorld()->reset();
  clearGameActionsToolbar();
  
  std::map< QString, QString > nations = nationsList();
  if (!(GameLogic::GameAutomaton::changeable().playerList()->isEmpty()))
  {
    GameLogic::GameAutomaton::changeable().playerList()->clear();
  }
  kDebug() << "KGameWindow::setupPlayers: before switch; newPlayersNumber = " << newPlayersNumber << endl;
  unsigned int nbAvailArmies = (unsigned int)(m_theWorld->getNbCountries() * 2.5 / newPlayersNumber);
  kDebug() << "KGameWindow::setupPlayers: nbAvailArmies = " << nbAvailArmies << " ; nb countries = " << m_theWorld->getNbCountries() << endl;
  // Players names
  QString mes = "";
  QString nationName;
  for (unsigned int i = 0; 
       i < newPlayersNumber - GameLogic::GameAutomaton::changeable().networkPlayersNumber(); 
       i++)
  {
    QString nomEntre = "";
    bool computer;
    bool network = false;
    QString password;

    // After closing KPlayerSetupDialog, it is guaranteed, that nomEntre is a valid
    // username (not empty, unique)
    KPlayerSetupDialog(m_theWorld, i+1, nomEntre, network, password, computer, nations, nationName,
                       this, "KDialogSetupPlayer").exec();

    kDebug() << "Creating player " << nomEntre << "(computer: " << computer << "): " << nationName << endl;
    addPlayer(nomEntre, nbAvailArmies, 0, nationName, computer);
    nations.erase(nationName);
  }
  if (networkGame)
  {
    kDebug() << "In setupPlayers: networkGame" << endl;
    GameLogic::GameAutomaton::changeable().offerConnections(port);
    KMessageParts messageParts;
    messageParts << I18N_NOOP("Waiting for %1 players to connect")
      << QString::number(GameLogic::GameAutomaton::changeable().networkPlayersNumber());
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  }
  m_frame->setFocus();
  return true;
}

bool KGameWindow::setupOnePlayer()
{
  kDebug() << "KGameWindow::setupOnePlayer" << endl;
//   haltTimer();
  
  kDebug() << "  building the list of available nations" << endl;
  std::map< QString, QString > nations = nationsList();
  PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
  PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
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
  unsigned int nbAvailArmies = (unsigned int)(m_theWorld->getNbCountries() * 2.5 / (GameAutomaton::changeable().nbPlayers()));
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
      mes = i18n("Player number %d, what's your name ?", 1);
      bool network = true;
      KPlayerSetupDialog(m_theWorld, 1, nomEntre, network, password, computer, nations, nationName, this, "KDialogSetupPlayer").exec();
      kDebug() << "After KPlayerSetupDialog. name: " << nomEntre << endl;
      if (nomEntre == "")
      {
        mes = i18n("Error - Player %d, you have to choose a name.", 1);
        QMessageBox::warning(this, i18n("Error"), mes);
        nomEntre = i18n("Player%d", 1);
      }
      else 
      {
        emptyName = false;
      }
    }
    found = false;
    PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
    PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
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
//   haltTimer();
  
  QString password;
  int result;
  KWaitedPlayerSetupDialog(password, result, this).exec();
  kDebug() << "After KWaitedPlayerSetupDialog. number: " << result << ", password: " << password << endl;
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << (quint32)result << password;
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,ValidateWaitedPlayerPassword);
//   initTimer();
  return true;
}

bool KGameWindow::createWaitedPlayer(quint32 waitedPlayerId)
{
  kDebug() << "KGameWindow::createWaitedPlayer" << endl;
//   haltTimer();
  
  PlayerMatrix& pm = m_waitedPlayers[waitedPlayerId];
  Player* player = 0;
  if (pm.isAI)
  {
    player = new AIColsonPlayer(pm.name, pm.nbAvailArmies,
                                      m_theWorld-> nationNamed(pm.nation), *GameLogic::GameAutomaton::changeable().playerList(), m_theWorld, &GameLogic::GameAutomaton::changeable());
    GameAutomaton::changeable().addPlayer(player);
    kDebug() << "Created Waited AI player " << player->name() << endl;
    ((AIPlayer*)(player))->stop();
    kDebug() << "Calling AI player createIO" << endl;
    GameAutomaton::changeable().createIO(player, KGameIO::IOMode(AIPLAYERIO));
  }
  else
  {
    player = new Player(pm.name, pm.nbAvailArmies, m_theWorld-> nationNamed(pm.nation));
    GameAutomaton::changeable().addPlayer(player);
    kDebug() << "Created player " << player->name() << endl;
    kDebug() << "Calling player createIO" << endl;
    GameAutomaton::changeable().createIO(player,KGameIO::IOMode(KGameIO::MouseIO));
    m_chatDlg->setFromPlayer(player);
  }
  player->setNbCountries(pm.nbCountries);
  player->setNbAvailArmies(pm.nbAvailArmies);
  player->setNbAttack(pm.nbAttack);
  player->setNbDefense(pm.nbDefense);
  player->setPassword(pm.password);
  player->goal(pm.goal);
  std::set<QString>::iterator it, it_end;
  it = pm.countries.begin(); it_end = pm.countries.end();
  for (; it != it_end; it++)
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << (*it) << pm.name;
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,CountryOwner);
  }
  return true;
}




void KGameWindow::distributeArmies()
{
//   kDebug() << "KGameWindow::distributeArmies" << endl;
  PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
  PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
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
//   stream << GameLogic::GameAutomaton::changeable().idForMsg(s);
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
    stream << GameLogic::GameAutomaton::changeable().idForMsg(it.curStr());
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
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,ChangeItem);
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
  PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
  PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
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
    return (GameLogic::GameAutomaton::single().state() == state);
}

/**
  * returns the current state of the game
  */
GameLogic::GameAutomaton::GameState KGameWindow::getState() const
{
    return GameLogic::GameAutomaton::single().state();
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
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayDefenseButtons);
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
    GameLogic::GameAutomaton::changeable().sendMessage(bufferSrc,FirstCountry);
    
    QByteArray bufferDest;
    QDataStream streamDest(&bufferDest, QIODevice::WriteOnly);
    streamDest << src.name();
    GameLogic::GameAutomaton::changeable().sendMessage(bufferDest,SecondCountry);
    
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

  GameLogic::GameAutomaton::changeable().currentPlayer((Player*)(*GameLogic::GameAutomaton::changeable().playerList()->begin()));

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
  
  PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
  PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
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
    GameLogic::GameAutomaton::changeable().currentPlayer((Player*)(*it));
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
  m_animFighters->hideAndRemoveAll();
  toolBar("gameActionsToolBar")-> show();
  return attackEnd();
}

bool KGameWindow::attacker(const QPointF& point)
{
  Country* clickedCountry = clickIn(point);
  KMessageParts messageParts;
  if (clickedCountry == 0)
  {
    messageParts << I18N_NOOP("<font color=\"orange\">No country here !</font>");
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    displayNormalGameButtons();
    return false;
  }
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << clickedCountry->name();
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,FirstCountry);
  QByteArray buffer2;
  QDataStream stream2(&buffer2, QIODevice::WriteOnly);
  stream2 << "";
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,SecondCountry);
  
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
//  if (currentPlayer()-> isAI()) return 3;
        
  unsigned int res = 0;
  Country* secondCountry = clickIn(point);
  m_secondCountry = secondCountry;
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
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,SecondCountry);

  KMessageParts messageParts;

  if (!secondCountry || !secondCountry-> owner()) 
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
    messageParts << I18N_NOOP("%1! You cannot attack %2! It's yours!") << currentPlayer()-> name()
            << secondCountry-> name();
    displayNormalGameButtons();
  }
  else if (m_firstCountry-> owner() != currentPlayer()) 
  {
    messageParts << I18N_NOOP("%1 ! You are not the owner of %2!") << currentPlayer()-> name() << m_firstCountry-> name();
    displayNormalGameButtons();
  }
  else if ( (m_firstCountry == NULL) || (secondCountry == NULL)
          || (m_firstCountry-> owner() != currentPlayer()) )
  {
    messageParts << I18N_NOOP("Nothing to attack !");
    displayNormalGameButtons();
  }
  else if ((secondCountry-> nbArmies() > 1)
      && (currentPlayer()-> getNbAttack() >= 2))
  {
    messageParts
      << I18N_NOOP("%1, with how many armies do you defend %2 ?") 
      << secondCountry->owner()-> name()
      << secondCountry-> name();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << secondCountry->owner()->name();
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayDefenseButtons);
    res = 1;
  }
  else
  {
    messageParts
      << I18N_NOOP("%1, you defend %2 with its unique army.") 
      << secondCountry->owner()-> name()
      << secondCountry-> name();
    res = 2;
  }
  broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
  return res;
}

void KGameWindow::firstCountryAt(const QPointF& point)
{
  if (clickIn(point))
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << clickIn(point)->name();
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,FirstCountry);
  }
}

void KGameWindow::secondCountryAt(const QPointF& point)
{
  if (clickIn(point))
  {
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << clickIn(point)->name();
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,SecondCountry);
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

      if (GameLogic::GameAutomaton::changeable().isAdmin())
      {
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream << currentPlayer()->id();
        GameLogic::GameAutomaton::changeable().sendMessage(buffer,CheckGoal);
        
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
          PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
          PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
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
            GameLogic::GameAutomaton::changeable().sendMessage(buffer,KGameWinAvailArmies);
          }
          if (GameLogic::GameAutomaton::changeable().isAdmin())
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
    if ( GameLogic::GameAutomaton::changeable().isAdmin() )
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
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayRecyclingButtons);
  
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
      GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNextPlayerButton);
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
  kDebug() << "nextPlayerNormal" << endl;
  if (setCurrentPlayerToNext())
  {
    distributeArmies();
  
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,ShowArmiesToPlace);
    
    clear();
    QByteArray buffer2;
    QDataStream stream2(&buffer2, QIODevice::WriteOnly);
    GameLogic::GameAutomaton::changeable().sendMessage(buffer2,DisplayNextPlayerButton);
    return true;
  }
  else
  {
    QPixmap pm = currentPlayer()->getFlag()->image(0);
    KMessageParts messageParts;
    messageParts 
      << pm
      << I18N_NOOP("%1, it's up to you.") << currentPlayer()->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    clear();
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNormalGameButtons);
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
}

void KGameWindow::defense(unsigned int nb)
{
  clearGameActionsToolbar();
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

  if (m_firstCountry && m_firstCountry-> owner() && m_firstCountry-> owner()-> getFlag())
    m_barFlagButton-> setIcon(KIcon(m_firstCountry-> owner()->getFlag()-> image(0)));
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,InitCombatMovement);
  GameLogic::GameAutomaton::changeable().state(GameLogic::GameAutomaton::FIGHT_BRING);
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
    << I18N_NOOP("%1, it's up to you.") << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
}
        
void KGameWindow::shiftFinished()
{
  displayNormalGameButtons();
  QPixmap pm = currentPlayer()->getFlag()->image(0);
  KMessageParts messageParts;
  messageParts 
    << pm 
    << I18N_NOOP("%1, it's up to you.") << currentPlayer()->name();
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
    << I18N_NOOP("%1, it's up to you.") << currentPlayer()->name();
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
  if  ( ( GameLogic::GameAutomaton::changeable().playerList()->count() == 0 ) ||
  ( isMyState(GameLogic::GameAutomaton::GAME_OVER)  ) ||
        (KMessageBox::warningContinueCancel(this,i18n("Do you really want to end your current game and start a new one ?"),i18n("New game confirmation"),KStandardGuiItem::yes()) == KMessageBox::Continue ) )

  {
    // @todo if new game is canceled, removed buttons should be displayed again
//    clearGameActionsToolbar();
//     haltTimer();
/*    if (!(GameLogic::GameAutomaton::changeable().playerList()->isEmpty()))
    {
      GameLogic::GameAutomaton::changeable().playerList()->clear();
      GameLogic::GameAutomaton::changeable().currentPlayer(0);
      kDebug() << "  playerList size = " << GameLogic::GameAutomaton::changeable().playerList()->count() << endl;
    }
    theWorld()->reset();*/
    
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
    << GameAutomaton::single().skin().toUtf8().data() 
    << "\" state=\"" 
    << GameAutomaton::single().state() 
    << "\" >" 
    << std::endl;
  m_theWorld->saveXml(xmlStream);
  xmlStream << "<players nb=\""<<GameLogic::GameAutomaton::changeable().playerList()->count()<<"\">" << std::endl;
  PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->begin();
  PlayersArray::iterator it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
  for (; it != it_end; it++)
  {
    static_cast<Player*>(*it)->saveXml(xmlStream);
  }
  xmlStream << "</players>" << std::endl;
  Player* player = GameLogic::GameAutomaton::changeable().currentPlayer();
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
  it = GameLogic::GameAutomaton::changeable().playerList()->begin();
  it_end = GameLogic::GameAutomaton::changeable().playerList()->end();
  for (; it != it_end; it++)
  {
    static_cast<Player*>(*it)->goal()->saveXml(xmlStream);
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
void KGameWindow::addPlayer(const QString& playerName, 
      unsigned int nbAvailArmies, 
      unsigned int nbCountries, 
      const QString& nationName, 
      bool isAI,
      const QString& password,
      unsigned int nbAttack,
      unsigned int nbDefense)
{
  kDebug() << "Adding player (AI: " << isAI << ")" << endl;
  Player* p = 0;
  if (isAI)
  {
    AIPlayer *aiplayer = new AIColsonPlayer(playerName, nbAvailArmies,
                                      m_theWorld-> nationNamed(nationName), *GameLogic::GameAutomaton::changeable().playerList(), m_theWorld, &GameLogic::GameAutomaton::changeable());
    GameAutomaton::changeable().addPlayer(aiplayer);
//     kDebug() << "Created AI player " << aiplayer->name() << endl;
    aiplayer->stop();
    aiplayer->setNbCountries(nbCountries);
    aiplayer->setNbAvailArmies(nbAvailArmies);
    aiplayer->setNbAttack(nbAttack);
    aiplayer->setNbDefense(nbDefense);
    aiplayer->setPassword(password);
    kDebug() << "Calling AI player createIO" << endl;
    GameAutomaton::changeable().createIO(aiplayer, KGameIO::IOMode(AIPLAYERIO));
    p = aiplayer;
  }
  else
  {
    Player *player = new Player(playerName, nbAvailArmies, m_theWorld-> nationNamed(nationName));
    GameAutomaton::changeable().addPlayer(player);
    kDebug() << "Created player " << player->name() << endl;
    player->setNbCountries(nbCountries);
    player->setNbAvailArmies(nbAvailArmies);
    player->setNbAttack(nbAttack);
    player->setNbDefense(nbDefense);
    player->setPassword(password);
    kDebug() << "Calling player createIO" << endl;
    GameAutomaton::changeable().createIO(player,KGameIO::IOMode(KGameIO::MouseIO));
    p = player;
    m_chatDlg->setFromPlayer(p);
  }
  
}

std::map< QString, QString > KGameWindow::nationsList()
{
  std::map< QString, QString >  res;
  
  std::vector<Nationality*>& nationsList = m_theWorld->getNationalities();
//   kDebug() << "There is " << nationsList.count() << " nations" << endl;
  std::vector<Nationality*>::iterator nationsIt = nationsList.begin();
  for (; nationsIt != nationsList.end(); nationsIt++ ) 
  {
//     kDebug() << "Nation = " << *nationsIt << endl;
    Nationality* nation = *nationsIt; 
    res.insert(std::make_pair(nation->name(),nation->flagFileName()));
  } 
  return res;
}

/** @return true if the given player is the last one ; false otherwise */
bool KGameWindow::isLastPlayer(const Player& player)
{
  if (GameLogic::GameAutomaton::changeable().playerList()->begin() == GameLogic::GameAutomaton::changeable().playerList()->end())
  {
    kError() << "No player ; should not be able to call isLastPlayer !" << endl;
    exit(1);
  }
  PlayersArray::iterator it = GameLogic::GameAutomaton::changeable().playerList()->end();
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
  GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNextPlayerButton);
  KMessageParts messageParts;
  QPixmap pm = currentPlayer()->getFlag()->image(0);
  messageParts 
    << pm 
    << I18N_NOOP("%1, please change your distributions.") 
    << currentPlayer()->name();
  broadcastChangeItem(messageParts, ID_STATUS_MSG2);
//   initTimer();
}

void KGameWindow::actionRecyclingFinished()
{
//   kDebug() << "actionRecyclingFinished" << endl;
  if (GameLogic::GameAutomaton::changeable().isAdmin())
  {
    for (unsigned int i = 0; i < m_theWorld->getNbCountries(); i++) 
    {
      m_theWorld-> getCountries().at(i)-> nbAddedArmies(0);
    }
    QPixmap pm = currentPlayer()->getFlag()->image(0);
    KMessageParts messageParts;
    messageParts 
      << pm
      << I18N_NOOP("%1, it's up to you.") << currentPlayer()->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNormalGameButtons);
    GameLogic::GameAutomaton::changeable().state(GameLogic::GameAutomaton::WAIT);
  }
  //  initTimer();
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
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNormalGameButtons);
    break;
  case GameLogic::GameAutomaton::WAIT_RECYCLING:;
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNextPlayerButton);
    break;
  case GameLogic::GameAutomaton::NEWARMIES:;
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNextPlayerButton);
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
    GameLogic::GameAutomaton::changeable().sendMessage(buffer,DisplayNormalGameButtons);
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
  message1Parts << I18N_NOOP("All commands are issued through changing toolbar buttons and drag & drop. Hit...");
  broadcastChangeItem(message1Parts, ID_NO_STATUS_MSG);

  QString newGameImageFileName = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/" + CM_NEWGAME);
  if (newGameImageFileName.isNull())
  {
    QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
    exit(2);
  }
  QPixmap newGameButtonPix(newGameImageFileName); 
  KMessageParts message2Parts;
  message2Parts << "\t" << newGameButtonPix << I18N_NOOP(" to start a new game;");
  broadcastChangeItem(message2Parts, ID_NO_STATUS_MSG);

  QString joinGameImageFileName = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/" + CM_NEWNETGAME);
  if (joinGameImageFileName.isNull())
  {
    QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
    exit(2);
  }
  QPixmap joinGameButtonPix(joinGameImageFileName); 
  KMessageParts message3Parts;
  message3Parts << "\t" << joinGameButtonPix << I18N_NOOP(" to join a network game (starting or reloading); and");
  broadcastChangeItem(message3Parts, ID_NO_STATUS_MSG);

  QString openGameImageFileName = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/" + CM_OPENGAME);
  if (openGameImageFileName.isNull())
  {
    QMessageBox::critical(0, i18n("Error !"), i18n("Cannot load button image\nProgram cannot continue"));
    exit(2);
  }
  QPixmap openGameButtonPix(openGameImageFileName); 
  KMessageParts message4Parts;
  message4Parts << "\t" << openGameButtonPix << I18N_NOOP(" to open a saved game.");
  broadcastChangeItem(message4Parts, ID_NO_STATUS_MSG);

  KMessageParts message5Parts;
  message5Parts << I18N_NOOP("And then let the system guide you through messages and tooltips appearing on buttons when hovering above them.");
  broadcastChangeItem(message5Parts, ID_NO_STATUS_MSG);
}



} // closing namespace Ksirk

#include "kgamewin.moc"
