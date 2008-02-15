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

/*  begin                : Thu Jul 19 2001  */

#include "decoratedgameframe.h"
#include "GameLogic/player.h"

#include <QSize>
#include <QEvent>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QScrollBar>

#include <kgamewin.h>
#include <kstandardgameaction.h>
#include <kstandardaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kgame/kgameio.h>
#include <kgame/kplayer.h>
#include <KAction>

namespace Ksirk
{
using namespace GameLogic;

DecoratedGameFrame::DecoratedGameFrame(QWidget* parent, 
      unsigned int mapW, unsigned int mapH, GameAutomaton* automaton)
  : QGraphicsView(parent), m_mapW(mapW), m_mapH(mapH)
{
  kDebug() << "("<<mapW<<"x"<<mapH<<")" << endl;
  setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  setCacheMode(QGraphicsView::CacheBackground);
  setMinimumSize(200,100);
  setMaximumSize(mapW,mapH);
  updateGeometry(); 
  setMouseTracking(true);

  this->m_parent = parent;
  QuitAction = KStandardGameAction::quit(this->m_parent, SLOT(close()), this);
  
  initMenu ();
  initAttackMenu();
  initMoveMenu();

  this->m_automaton = automaton;

  // redirect the mouse move event to the main windows
  connect(this, SIGNAL(mouseMoveEventReceived(QMouseEvent *)), parent, SLOT(mouseMoveEvent(QMouseEvent *)));
}

DecoratedGameFrame::~DecoratedGameFrame()
{
}

void DecoratedGameFrame::mouseMoveEvent ( QMouseEvent * event )
{
//   kDebug() << "DecoratedGameFrame::mouseMoveEvent" << endl;
  emit (mouseMoveEventReceived(event));
  event->ignore();
}

QSize DecoratedGameFrame::sizeHint() const
{
//   kDebug() << " " << m_mapW << "/" << m_mapH << endl;
  return QSize(m_mapW, m_mapH);
}

void DecoratedGameFrame::initMenu ()
{
  this->menu = new QMenu(this);
    
  QAction* newAction = KStandardGameAction::gameNew(this->m_parent, SLOT(slotNewGame()), this);
  
  QAction* openAction = KStandardGameAction::load(this->m_parent, SLOT(slotOpenGame()), this);
  
  QAction* saveAction = KStandardGameAction::save(this->m_parent, SLOT(slotSaveGame()), this);
  
  QAction* zoomInAction = KStandardAction::zoomIn(this->m_parent, SLOT(slotZoomIn()), this);
  
  QAction* zoomOutAction = KStandardAction::zoomOut(this->m_parent, SLOT(slotZoomOut()), this);

  nextPlayer = 	new QAction(i18n("Next Player"), this);
  connect(nextPlayer, SIGNAL(triggered()), this->m_parent, SLOT(slotNextPlayer()));

  detailsAction = new QAction(i18n("Details"), this);
  connect(detailsAction, SIGNAL(triggered()), this, SLOT(slotDetails()));

  goalAction = new QAction(QIcon(), i18n("Goal"), this);
  goalAction->setShortcut(Qt::CTRL+Qt::Key_G);
  connect(goalAction,SIGNAL(triggered(bool)),this->m_parent,SLOT(slotShowGoal()));
 
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

void DecoratedGameFrame::initAttackMenu ()
{
    this->attackMenu = new QMenu(this);

    ArenaAction = new QAction(i18n("Enable Arena"), this);
    connect(ArenaAction, SIGNAL(triggered()), this, SLOT(arenaState()));
    connect(this, SIGNAL(arenaStateSignal(bool)), this->m_parent, SLOT(slotArena(bool)));

    Attack1Action = new QAction(i18n("Attack1"), this);
    connect(Attack1Action, SIGNAL(triggered()), this->m_parent, SLOT(slotAttack1()));

    Attack2Action = new QAction(i18n("Attack2"), this);
    connect(Attack2Action, SIGNAL(triggered()), this->m_parent, SLOT(slotAttack2()));

    Attack3Action = new QAction(i18n("Attack3"), this);
    connect(Attack3Action, SIGNAL(triggered()), this->m_parent, SLOT(slotAttack3()));

    AutoAction = new QAction(i18n("Attack-auto"), this);
    connect(AutoAction, SIGNAL(triggered()), this, SLOT(attackAuto()));

    attackMenu->addAction(ArenaAction);
    attackMenu->addSeparator();	
    attackMenu->addAction(Attack1Action);
    attackMenu->addAction(Attack2Action);
    attackMenu->addAction(Attack3Action);
    attackMenu->addSeparator();	
    attackMenu->addAction(AutoAction);
    attackMenu->addSeparator();
    attackMenu->addAction(QuitAction);
}

void DecoratedGameFrame::initMoveMenu ()
{
    this->moveMenu = new QMenu(this);

    Move1Action = new QAction(i18n("Move1"), this);
    connect(Move1Action, SIGNAL(triggered()),this->m_parent, SLOT(slotInvade1()));

    Move5Action = new QAction(i18n("Move5"), this);
    connect(Move5Action, SIGNAL(triggered()),this->m_parent, SLOT(slotInvade5()));

    Move10Action = new QAction(i18n("Move10"), this);
    connect(Move10Action, SIGNAL(triggered()),this->m_parent, SLOT(slotInvade10()));
		
    moveMenu->addAction(Move1Action);
    moveMenu->addAction(Move5Action);
    moveMenu->addAction(Move10Action);
    moveMenu->addSeparator();
    moveMenu->addAction(QuitAction);
}

void DecoratedGameFrame::contextMenuEvent( QContextMenuEvent * )
{
    menuPoint = QCursor::pos();
    kDebug() << "************state decoratedgameframe" << m_automaton->stateName() << endl;
    if (m_automaton->stateName() != "INIT" && m_automaton->stateName() != "INTERLUDE" && m_automaton->stateName() != "NEWARMIES" && m_automaton->stateName() != "WAIT_RECYCLING")
    {
    	if (!m_automaton-> currentPlayer()->isAI() && !m_automaton-> currentPlayer()->isVirtual())
	{
		if (m_automaton->stateName() == "WAIT")
		{
			nextPlayer->setVisible(true);
		}
		else
		{
			nextPlayer->setVisible(false);
		}
		
		// set the goal icon to the proper flag
		goalAction-> setIcon(KIcon(m_automaton-> currentPlayer()->getFlag()-> image(0)));

		goalAction->setVisible(true);

		// we cannot see detail of country during the AI or virtual player game
		// as the right widget is reserved for combat
		if(m_automaton->game()->theWorld()->countryAt(detailPoint)!=0)
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

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/attackOne.png");
	Attack1Action-> setIcon(QIcon(imageFileName));

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/attackTwo.png");
	Attack2Action-> setIcon(QIcon(imageFileName));

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/attackThree.png");
	Attack3Action-> setIcon(QIcon(imageFileName));

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/attackAuto.png");
	AutoAction-> setIcon(QIcon(imageFileName));

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/moveOne.png");
	Move1Action-> setIcon(QIcon(imageFileName));

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/moveFive.png");
	Move5Action-> setIcon(QIcon(imageFileName));

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/moveTen.png");
	Move10Action-> setIcon(QIcon(imageFileName));

	//temporary
	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/moveArmies.png");
	ArenaAction-> setIcon(QIcon(imageFileName));

	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/joueurSuivant.png");
	nextPlayer-> setIcon(QIcon(imageFileName));

	// temporary
	imageFileName = KGlobal::dirs()->findResource("appdata", skin + "/Images/newNetGame.png");
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
  kDebug() << "DecoratedGameFrame::slotMouseInput()" << endl;
  
  KPlayer *player=input->player();
  if (!player->myTurn())
  {
    kDebug() << "Player " << dynamic_cast<Player*>(player)->name() << ": not my turn!" << endl;
    *eatevent=false;
    e->setAccepted(false);
    return;
  }
  kDebug() << "Player " << dynamic_cast<Player*>(player)->name() << " " << e->type() << endl;
  if (e->type() == QEvent::GraphicsSceneMousePress)
  {
    kDebug() << "\tQEvent::MouseButtonPress" << endl;
    if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::LeftButton)
    {
      kDebug() << "\tLeft" << endl;
      stream << QString("actionLButtonDown");
    }
    else if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::RightButton)
    {
      kDebug() << "\tRight" << endl;
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
    kDebug() << "\tQEvent::MouseButtonRelease" << endl;
    if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::LeftButton)
    {
      kDebug() << "\tLeft" << endl;
      stream << QString("actionLButtonUp");
    }
    else if (((QGraphicsSceneMouseEvent*)e)->button() == Qt::RightButton)
    {
      kDebug() << "\tRight" << endl;
      stream << QString("actionRButtonUp");
    }
    else
    {
      kDebug() << "\tOther:" << e->button() << endl;
      *eatevent=false;
      e->setAccepted(false);
      return;
    }
  }
  else
  {
    *eatevent=false;
    e->setAccepted(false);
    return;
  }
  QPointF newPoint = ((QGraphicsSceneMouseEvent*)e)->scenePos();
  kDebug() << "\tPosition: " << newPoint << endl;

  detailPoint.setX((int)newPoint.x());
  detailPoint.setY((int)newPoint.y());

  stream << newPoint;
  *eatevent=true;
  kDebug() << "Mouse input done... eatevent=true" << endl;
}

void DecoratedGameFrame::arenaState()
{
	//if (ArenaAction->isChecked())
	if (ArenaAction->text().contains("Enable Arena", Qt::CaseInsensitive))
	{
		ArenaAction->setText("Disable Arena");
		emit(arenaStateSignal(true));
	}
	else
	{
		ArenaAction->setText("Enable Arena");
		emit(arenaStateSignal(false));
	}

	attackMenu->exec(menuPoint);
}

void DecoratedGameFrame::attackAuto() {
	m_automaton->setAttackAuto(true);
	if (m_automaton->game()->firstCountry()->nbArmies() > 3) {
		m_automaton->game()->slotAttack3();
	} else if (m_automaton->game()->firstCountry()->nbArmies() > 2) {
		m_automaton->game()->slotAttack2();
	} else if (m_automaton->game()->firstCountry()->nbArmies() > 1) {
		m_automaton->game()->slotAttack1();
	} else {
		m_automaton->setAttackAuto(false);
	}
}

void DecoratedGameFrame::slotDetails()
{
	QPointF *point = new QPointF(detailPoint);

	m_automaton->game()->getRightDialog()->displayCountryDetails(point);
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

}

#include "decoratedgameframe.moc"
