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

/*  begin                : Thu Jul 19 2001  */

#include "decoratedgameframe.h"
#include "GameLogic/player.h"

#include <QSize>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>

#include <kgamewin.h>
#include <KStandardGameAction>
#include <KStandardAction>
#include <KLocalizedString>
#include "ksirk_debug.h"
#include <KGamePopupItem>
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <libkdegamesprivate/kgame/kgameio.h>
#include <libkdegamesprivate/kgame/kplayer.h>

namespace Ksirk
{
using namespace GameLogic;


DecoratedGameFrame::DecoratedGameFrame(QWidget* parent,
      unsigned int mapW, unsigned int mapH, GameAutomaton* automaton)
  : QGraphicsView(parent), m_mapW(mapW), m_mapH(mapH), m_parent(parent),
  m_automaton(automaton)
{
  qCDebug(KSIRK_LOG) << "("<<mapW<<"x"<<mapH<<")";
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setCacheMode(QGraphicsView::CacheBackground);
  setMinimumSize(200,100);
  setMaximumSize(mapW,mapH);
  updateGeometry(); 
  setMouseTracking(true);

  QuitAction = KStandardGameAction::quit(m_automaton->game(), SLOT(close()), this);
  
  initMenu ();
  initAttackMenu();
  initMoveMenu();

  // redirect the mouse move event to the main windows
  connect(this, SIGNAL(mouseMoveEventReceived(QMouseEvent*)), automaton->game(), SLOT(mouseMoveEvent(QMouseEvent*)));
}

DecoratedGameFrame::~DecoratedGameFrame()
{
}

void DecoratedGameFrame::mouseMoveEvent ( QMouseEvent * event )
{
//   qCDebug(KSIRK_LOG);
  emit (mouseMoveEventReceived(event));
//   event->ignore();
}

QSize DecoratedGameFrame::sizeHint() const
{
//   qCDebug(KSIRK_LOG) << " " << m_mapW << "/" << m_mapH;
  return QSize(m_mapW, m_mapH);
}

void DecoratedGameFrame::initMenu ()
{
  menu = new QMenu(this);
    
  QAction* newAction = KStandardGameAction::gameNew(m_automaton->game(), SLOT(slotNewGame()), this);
  
  QAction* openAction = KStandardGameAction::load(m_automaton->game(), SLOT(slotOpenGame()), this);
  
  QAction* saveAction = KStandardGameAction::save(m_automaton->game(), SLOT(slotSaveGame()), this);
  
  QAction* zoomInAction = KStandardAction::zoomIn(m_automaton->game(), SLOT(slotZoomIn()), this);
  
  QAction* zoomOutAction = KStandardAction::zoomOut(m_automaton->game(), SLOT(slotZoomOut()), this);

  nextPlayer =   new QAction(i18n("Next Player"), this);
  connect(nextPlayer, &QAction::triggered, m_automaton->game(), &KGameWindow::slotNextPlayer);

  detailsAction = new QAction(i18n("Details"), this);
  connect(detailsAction, &QAction::triggered, this, &DecoratedGameFrame::slotDetails);

  goalAction = new QAction(QIcon(), i18n("Goal"), this);
  goalAction->setShortcut(Qt::CTRL+Qt::Key_G);
  connect(goalAction,&QAction::triggered,m_automaton->game(),&KGameWindow::slotShowGoal);
 
  menu->addAction(newAction);
  menu->addAction(openAction);
  menu->addAction(saveAction);
  menu->addSeparator();
  menu->addAction(zoomInAction);
  menu->addAction(zoomOutAction);
  menu->addAction(detailsAction);
  menu->addSeparator();
  menu->addAction(goalAction);
  menu->addAction(nextPlayer);
  menu->addSeparator();  
  menu->addAction(QuitAction);
}

void DecoratedGameFrame::setArenaOptionEnabled(bool option)
{
  qCDebug(KSIRK_LOG) << option;
  m_arenaAction->setEnabled(option);
  if (option == false)
  {
    emit(arenaStateSignal(false));
  }
}

void DecoratedGameFrame::initAttackMenu ()
{
    attackMenu = new QMenu(this);

    m_arenaAction = new ArenaAction(i18n("Enable Arena"), this);
    connect(m_arenaAction, &QAction::triggered, this, &DecoratedGameFrame::arenaState);
    connect(this, &DecoratedGameFrame::arenaStateSignal, m_automaton->game(), &KGameWindow::slotArena);
    
    Attack1Action = new QAction(i18n("Attack 1"), this);
    connect(Attack1Action, &QAction::triggered, m_automaton->game(), &KGameWindow::slotAttack1);

    Attack2Action = new QAction(i18n("Attack 2"), this);
    connect(Attack2Action, &QAction::triggered, m_automaton->game(), &KGameWindow::slotAttack2);

    Attack3Action = new QAction(i18n("Attack 3"), this);
    connect(Attack3Action, &QAction::triggered, m_automaton->game(), &KGameWindow::slotAttack3);

    AutoAction = new QAction(i18n("Auto attack"), this);
    connect(AutoAction, &QAction::triggered, this, &DecoratedGameFrame::attackAuto);

    attackMenu->addAction(AutoAction);
    attackMenu->addSeparator();  
    attackMenu->addAction(Attack1Action);
    attackMenu->addAction(Attack2Action);
    attackMenu->addAction(Attack3Action);
    attackMenu->addSeparator();  
    attackMenu->addAction(m_arenaAction);
    //     attackMenu->addSeparator();
//     attackMenu->addAction(QuitAction);
}

void DecoratedGameFrame::initMoveMenu ()
{
    moveMenu = new QMenu(this);

    Move1Action = new QAction(i18n("Move 1"), this);
    connect(Move1Action, &QAction::triggered,m_automaton->game(), &KGameWindow::slotInvade1);

    Move5Action = new QAction(i18n("Move 5"), this);
    connect(Move5Action, &QAction::triggered,m_automaton->game(), &KGameWindow::slotInvade5);

    Move10Action = new QAction(i18n("Move 10"), this);
    connect(Move10Action, &QAction::triggered,m_automaton->game(), &KGameWindow::slotInvade10);
  
    moveMenu->addAction(Move1Action);
    moveMenu->addAction(Move5Action);
    moveMenu->addAction(Move10Action);
//     moveMenu->addSeparator();
//     moveMenu->addAction(QuitAction);
}

void DecoratedGameFrame::contextMenuEvent( QContextMenuEvent * )
{
  menuPoint = QCursor::pos();
  qCDebug(KSIRK_LOG) << "************state decoratedgameframe" << m_automaton->stateName();
  if (m_automaton->state() != GameAutomaton::INIT
    && m_automaton->state() != GameAutomaton::INTERLUDE
    && m_automaton->state() != GameAutomaton::NEWARMIES
    && m_automaton->state() != GameAutomaton::WAIT_RECYCLING)
  {
    if (!m_automaton-> currentPlayer()->isAI()
        && !m_automaton-> currentPlayer()->isVirtual())
    {
      if (m_automaton->state() == GameAutomaton::WAIT)
      {
        nextPlayer->setVisible(true);
      }
      else
      {
        nextPlayer->setVisible(false);
      }

      // set the goal icon to the proper flag
      goalAction-> setIcon(QIcon(m_automaton-> currentPlayer()->getFlag()-> image(0)));

      goalAction->setVisible(true);

      // we cannot see detail of country during the AI or virtual player game
      // as the right widget is reserved for combat
      // Bug 309863. Disable country details during auto attack also.
      if(m_automaton->game()->theWorld()->countryAt(detailPoint)!=0 && !m_automaton->isAttackAuto())
      {
        detailsAction->setVisible(true);
      }
      else
      {
        detailsAction->setVisible(false);
      }
    }
    else
    {
      // as the context menu can be displayed when AI play
      // we disabled the nextPlayer action and the goal
      nextPlayer->setVisible(false);
      goalAction->setVisible(false);
      // Bug 309863. Details of the country and combat are both shown in the right widget.
      // Some of the resources are used by both which leads to the crash. Disable detaied view
      // of the country when AI is playing. Some of the previous attempts to resolve this (or similar) issue(s)
      // disabled this item from context menu, but didn't cover all possible paths. Here, just follow that idea.
      // It makes no sense to show just for a moment details of the country when AI combat details will override
      // and reuse right widget (no time for human to inspect country details in such short time).
      detailsAction->setVisible(false);
    }

    menu->exec(menuPoint);
  }
}

// set the contextual menu event Icon for non-standard action
void DecoratedGameFrame::setIcon()
{
  KConfig config(m_automaton->game()->theWorld()->getConfigFileName());
  KConfigGroup onugroup = config.group("onu");
  QString skin = onugroup.readEntry("skinpath");
  
  QString imageFileName;

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/attackOne.png");
  Attack1Action-> setIcon(QIcon(imageFileName));

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/attackTwo.png");
  Attack2Action-> setIcon(QIcon(imageFileName));

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/attackThree.png");
  Attack3Action-> setIcon(QIcon(imageFileName));

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/attackAuto.png");
  AutoAction-> setIcon(QIcon(imageFileName));

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/moveOne.png");
  Move1Action-> setIcon(QIcon(imageFileName));

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/moveFive.png");
  Move5Action-> setIcon(QIcon(imageFileName));

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/moveTen.png");
  Move10Action-> setIcon(QIcon(imageFileName));

  //temporary
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/moveArmies.png");
  m_arenaAction-> setIcon(QIcon(imageFileName));

  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + '/' + CM_NEXTPLAYER);
  nextPlayer-> setIcon(QIcon(imageFileName));

  // temporary
  imageFileName = QStandardPaths::locate(QStandardPaths::AppDataLocation, skin + "/Images/newNetGame.png");
  detailsAction-> setIcon(QIcon(imageFileName));

}

/**
 * This slot is called when a mouse key is pressed. As the mouse is used as
 * input for all players
 * this slot is called to generate a player move out of a mouse input, i.e.
 * it converts a QMouseEvent into a move for the game. We do here some
 * simple nonsense and use the position of the mouse to generate
 * moves containing the keycodes
 */
void DecoratedGameFrame::slotMouseInput(KGameIO *input,QDataStream &stream,QMouseEvent *e,bool *eatevent)
{
  qCDebug(KSIRK_LOG) << e;
  
  QGraphicsItem* item = itemAt(mapFromScene(((QGraphicsSceneMouseEvent*)e)->scenePos()));
  if (item == 0)
  {
    *eatevent=false;
    e->setAccepted(false);
    return;
  }

  while (item->parentItem()!=0 && dynamic_cast<KGamePopupItem*>(item)==0)
  {
    item = item->parentItem();
  }

  if (dynamic_cast<KGamePopupItem*>(item)!=0)
  {
    *eatevent=false;
    e->setAccepted(false);
    return;
  }

  KPlayer *player=input->player();
  if (!player->myTurn())
  {
    qCDebug(KSIRK_LOG) << "Player " << dynamic_cast<Player*>(player)->name() << ": not my turn!";
    *eatevent=false;
    e->setAccepted(false);
    return;
  }
  qCDebug(KSIRK_LOG) << "Player " << dynamic_cast<Player*>(player)->name() << " " << e->type();
  if (e->type() == QEvent::GraphicsSceneMousePress)
  {
    qCDebug(KSIRK_LOG) << "\tQEvent::MouseButtonPress";
    if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::LeftButton)
    {
      qCDebug(KSIRK_LOG) << "\tLeft";
      stream << QString("actionLButtonDown");
    }
    else if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::RightButton)
    {
      qCDebug(KSIRK_LOG) << "\tRight";
      stream << QString("actionRButtonDown");
    }
    else
    {
      *eatevent=false;
      e->setAccepted(false);
      return;
    }
  }
  else if (e->type() == QEvent::GraphicsSceneMouseRelease)
  {
    qCDebug(KSIRK_LOG) << "\tQEvent::MouseButtonRelease";
    if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::LeftButton)
    {
      qCDebug(KSIRK_LOG) << "\tLeft";
      stream << QString("actionLButtonUp");
    }
    else if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::RightButton)
    {
      qCDebug(KSIRK_LOG) << "\tRight";
      stream << QString("actionRButtonUp");
    }
    else
    {
      qCDebug(KSIRK_LOG) << "\tOther:" << e->button();
      *eatevent=false;
      e->setAccepted(false);
      return;
    }
  }
  else if (e->type() == QWheelEvent::GraphicsSceneWheel)
  {
    qCDebug(KSIRK_LOG) << "Using mouse scroll";
  }
  else
  {
    *eatevent=false;
    e->setAccepted(false);
    return;
  }
  QPointF newPoint = ((QGraphicsSceneMouseEvent*)e)->scenePos();
  qCDebug(KSIRK_LOG) << "\tPosition: " << newPoint;

  detailPoint.setX((int)newPoint.x());
  detailPoint.setY((int)newPoint.y());

  stream << newPoint;
  *eatevent=true;
  qCDebug(KSIRK_LOG) << "Mouse input done... eatevent=true";
}

void DecoratedGameFrame::arenaState()
{
  qCDebug(KSIRK_LOG) << m_arenaAction->isArenaEnabled();
  //if (m_arenaAction->isChecked())
  if (m_arenaAction->isArenaEnabled())
  {
    m_arenaAction->setText(i18n("Enable Arena"));
    m_arenaAction->setArenaEnabled(false);
    emit(arenaStateSignal(false));
  }
  else
  {
    m_arenaAction->setText(i18n("Disable Arena"));
    m_arenaAction->setArenaEnabled(true);
    emit(arenaStateSignal(true));
  }

  attackMenu->exec(menuPoint);
}

void DecoratedGameFrame::attackAuto()
{
  qCDebug(KSIRK_LOG);
  unsigned int firstCountryNbArmies = ( m_automaton!=0 && m_automaton->game()!=0 && m_automaton->game()->firstCountry()!=0 )
      ? m_automaton->game()->firstCountry()->nbArmies() : 0 ;
  m_automaton->setAttackAuto(firstCountryNbArmies>0);
  if (firstCountryNbArmies > 3)
  {
    m_automaton->game()->slotAttack3();
  }
  else if (firstCountryNbArmies > 2)
  {
    m_automaton->game()->slotAttack2();
  }
  else if (firstCountryNbArmies > 1)
  {
    m_automaton->game()->slotAttack1();
  }
}

void DecoratedGameFrame::slotDetails()
{
  qCDebug(KSIRK_LOG);
  m_automaton->game()->getRightDialog()->displayCountryDetails(detailPoint);
  m_automaton->game()->getRightDialog()->open();
}


QMenu * DecoratedGameFrame::getContextMenu()
{
  return this->menu;
}

QMenu * DecoratedGameFrame::getAttackContextMenu()
{
  return this->attackMenu;
}

QMenu * DecoratedGameFrame::getMoveContextMenu()
{
  qCDebug(KSIRK_LOG);
  return this->moveMenu;
}

QAction* DecoratedGameFrame::getAttack1Action()
{
  return this->Attack1Action;
}

QAction* DecoratedGameFrame::getAttack2Action()
{
  return this->Attack2Action;
}

QAction* DecoratedGameFrame::getAttack3Action()
{
  return this->Attack3Action;
}

QAction* DecoratedGameFrame::getMove1Action()
{
  return this->Move1Action;
}

QAction* DecoratedGameFrame::getMove5Action()
{
  return this->Move5Action;
}

QAction* DecoratedGameFrame::getMove10Action()
{
  return this->Move10Action;
}

void DecoratedGameFrame::setMenuPoint(QPoint menuPoint)
{
  this->menuPoint = menuPoint;
}

bool DecoratedGameFrame::viewportEvent(QEvent* event)
{
  bool res = QGraphicsView::viewportEvent(event);
  if (event->type() == QEvent::Resize)
  {
    qCDebug(KSIRK_LOG);
    m_automaton->game()->updateScrollArrows();
  }
  return res;
}

}


