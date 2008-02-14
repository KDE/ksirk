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

/*  begin                : Sat Sep 1 2001  */

/**
 * KGameWindow methods for sprites handling function of the state
 */

// application specific includes
#include "kgamewin.h"
#include "Sprites/backgnd.h"
#include "ksirksettings.h"
#include "Sprites/animspritesgroup.h"
#include "Sprites/cannonsprite.h"
#include "Sprites/cavalrysprite.h"
#include "Sprites/infantrysprite.h"
#include "Sprites/skinSpritesData.h"
#include "GameLogic/KMessageParts.h"
#include "GameLogic/onu.h"
#include "fightArena.h"

// include files for QT
#include <qmetaobject.h>
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qsound.h>
#include <qinputdialog.h>
#include <qglobal.h> // pour pouvoir utiliser qDebug()

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardaction.h>
#include <phonon/mediaobject.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <KToolBar>
 
namespace Ksirk
{
using namespace GameLogic;

/**
 * Prepares the sprites to be moved : removes the nb necessary sprites from
 * source, creates the moving sprites and gives them their destination, etc
 */
bool KGameWindow::initArmiesMovement(unsigned int nbABouger, Country *firstCountry, Country *secondCountry)
{
  kDebug() << "KGameWindow::initArmiesMovement -> " << nbABouger  << endl;
  KMessageParts messageParts;

  if (firstCountry-> nbArmies() <= nbABouger)
  {
    messageParts << I18N_NOOP("Cannot move %1 armies from %2 to %3") << QString::number(nbABouger)
      << firstCountry->name() << secondCountry->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return false;
  }
  else
  {
    messageParts << I18N_NOOP("Moving %1 armies from %2 to %3") << QString::number(nbABouger) << firstCountry->name() << secondCountry->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);
  }

  AnimSpritesGroup* newGroup = new AnimSpritesGroup(this,SLOT(slotMovingArmiesArrived(AnimSpritesGroup*)));
  m_animSpritesGroups.push_back(newGroup);
  AnimSprite* sprite;
  if ((firstCountry-> nbArmies() > 10) && (nbABouger == 10)
          && (nbABouger < firstCountry-> nbArmies()))
  {
    sprite = new CannonSprite( 
                              Sprites::SkinSpritesData::single().strData("cannon-id"), 
                              Sprites::SkinSpritesData::single().intData("cannon-width"),
                              Sprites::SkinSpritesData::single().intData("cannon-height"),
                              Sprites::SkinSpritesData::single().intData("cannon-frames"),
                              Sprites::SkinSpritesData::single().intData("cannon-versions"),
                              m_theWorld->zoom(),
                              backGnd(),                              200);
    firstCountry-> decrNbArmies(10);
  }
  else if ((firstCountry-> nbArmies() > 5) && (nbABouger == 5)
          && (nbABouger < firstCountry-> nbArmies()))
  {
    sprite = new CavalrySprite( Sprites::
        SkinSpritesData::single().strData("cavalry-id"), 
                                Sprites::SkinSpritesData::single().intData("cavalry-width"),
                                Sprites::SkinSpritesData::single().intData("cavalry-height"),
                                Sprites::SkinSpritesData::single().intData("cavalry-frames"), 
                                Sprites::SkinSpritesData::single().intData("cavalry-versions"),       m_theWorld->zoom(),
                                backGnd(),                                200);
    firstCountry-> decrNbArmies(5);
  }
  else if ((firstCountry-> nbArmies() > 1) && (nbABouger == 1)
          && (nbABouger < firstCountry-> nbArmies()))
  {
    sprite = new InfantrySprite( 
        Sprites::SkinSpritesData::single().strData("infantry-id"),
                                Sprites::SkinSpritesData::single().intData("infantry-width"),
                                Sprites::SkinSpritesData::single().intData("infantry-height"),
        Sprites::SkinSpritesData::single().intData("infantry-frames"),
                                Sprites::SkinSpritesData::single().intData("infantry-versions"),       m_theWorld->zoom(),
                                backGnd(),
                                200);
    firstCountry->  decrNbArmies();
  }
  else 
  {
    messageParts << I18N_NOOP("Cannot move %1 armies from %2 to %3") 
      << QString::number(nbABouger) 
      << firstCountry->name() 
      << secondCountry->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return false;
  }
  connect(sprite,SIGNAL(atDestination(AnimSprite*)),this,SLOT(slotMovingArmyArrived(AnimSprite*)));
  sprite->setupTravel(firstCountry, secondCountry);
  newGroup->addSprite(sprite);
  firstCountry-> createArmiesSprites();
  sprite->setAnimated();
//   kDebug() << "initArmiesMovement returns true" << endl;
  return true;
}

void KGameWindow::initCombatMovement(Country *paysAttaquant, Country *paysDefenseur)
{
  kDebug() << "1" << endl;

  gameActionsToolBar-> hide();
  
  m_animFighters->clear();
  m_animFighters->changeTarget(this,SLOT(slotMovingFightersArrived(AnimSpritesGroup*)));

  //  - attacker's flag point
  qreal pointFlagAttaquantX = paysAttaquant-> pointFlag().x()*      m_theWorld->zoom();

  //  - defender's flag point
  qreal pointFlagDefenseurX;
  if (!backGnd()->bgIsArena()) {
    pointFlagDefenseurX = paysDefenseur-> pointFlag().x()*      m_theWorld->zoom();
  } else {
    // in case of arena, the meet will be between the two countries
    pointFlagDefenseurX = backGnd()-> boundingRect().width() / 2;
  }

  //  - attacker's arrival point (resp. defender's) (left or right of the
  //    defender's flag point depending on the attacker's flag point position)
  qreal pointArriveeAttaquantX;
  qreal pointArriveeY;
  qreal pointArriveeDefenseurX;
  qreal pointDepartAttaquantX;
  qreal pointDepartDefenseurX;
  qreal pointDepartAttaquantY;
  qreal pointDepartDefenseurY;
  qreal leftRelativePos;

  if (!paysAttaquant->spritesInfantry().isEmpty())
  {
  	// We must know
	//  - attacker's departure point (pointInfantry)
  	pointDepartAttaquantX = paysAttaquant-> pointInfantry().x()*      m_theWorld->zoom();
  	pointDepartAttaquantY = paysAttaquant-> pointInfantry().y()*      m_theWorld->zoom();
  }
  else
  {
	if (!paysAttaquant->spritesCavalry().isEmpty())
  	{
		// We must know
 		//  - attacker's departure point (pointCavalry)
  		pointDepartAttaquantX = paysAttaquant-> pointCavalry().x()*      m_theWorld->zoom();
		pointDepartAttaquantY = paysAttaquant-> pointCavalry().y()*      m_theWorld->zoom();
	}
	else
	{
		// We must know
 		//  - attacker's departure point (pointCannon)
  		pointDepartAttaquantX = paysAttaquant-> pointCannon().x()*      m_theWorld->zoom();
  		pointDepartAttaquantY = paysAttaquant-> pointCannon().y()*      m_theWorld->zoom();
	}
  }

  if (!paysDefenseur->spritesInfantry().isEmpty())
  {
  	//  - defender's departure point (pointInfantry)
  	pointDepartDefenseurX = paysDefenseur-> pointInfantry().x()*      m_theWorld->zoom();
  	pointDepartDefenseurY = paysDefenseur-> pointInfantry().y()*      m_theWorld->zoom();
	
  }
  else
  {
	if (!paysDefenseur->spritesCavalry().isEmpty())
  	{
  		//  - defender's departure point (pointCavalry)
  		pointDepartDefenseurX = paysDefenseur-> pointCavalry().x()*      m_theWorld->zoom();
  		pointDepartDefenseurY = paysDefenseur-> pointCavalry().y()*      m_theWorld->zoom();
	}
	else
	{
  		//  - defender's departure point (pointCannon)
  		pointDepartDefenseurX = paysDefenseur-> pointCannon().x()*      m_theWorld->zoom();
  		pointDepartDefenseurY = paysDefenseur-> pointCannon().y()*      m_theWorld->zoom();
	}
  }


  // vertical meet point
  if (!backGnd()->bgIsArena()) {
    pointArriveeY = (((paysDefenseur-> pointFlag().y() + Sprites::SkinSpritesData::single().intData("fighters-flag-y-diff")))* m_theWorld->zoom()) ;
  } else {
    // in case of arena, the vertical meet will be as soon as it's possible
    pointArriveeY = (pointDepartAttaquantY+pointDepartDefenseurY)/2;
  }
  
  kDebug() << "2" << endl;
  if (!paysAttaquant->communicateWith(paysDefenseur))
  {
      kError() << "Error in KGameWindow::initCombatMovement: " << paysAttaquant-> name() << "  and "
              << paysDefenseur-> name() << " do not communicate!" << endl;
      exit(2);
  }
  // If the flag of the attacker is to the left of the flag of the defender,
  // then the arriving point of the attacker is to the left, else it is to
  // the right
  // The situation is reversed if the one of the attacker will need to go
  // through the world limit

  if (!paysAttaquant->spritesInfantry().isEmpty())
  {
  	leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("infantry-width"))*m_theWorld->zoom();
  }
  else
  {
	if (!paysAttaquant->spritesCavalry().isEmpty())
  	{
		leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cavalry-width"))*m_theWorld->zoom();
	}
	else
	{
		leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cannon-width"))*m_theWorld->zoom();
	}
  }
  
  
qreal rightRelativePos = (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("flag-width"))*m_theWorld->zoom();

  if (!(qAbs(pointFlagAttaquantX-pointFlagDefenseurX) > (backGnd()-> boundingRect().width() / 2)) || backGnd()->bgIsArena())
  {
      if ( pointFlagAttaquantX <= pointFlagDefenseurX )
      {
          pointArriveeAttaquantX = pointFlagDefenseurX + leftRelativePos;
          pointArriveeDefenseurX = pointFlagDefenseurX + rightRelativePos;
      }
      else
      {
          pointArriveeAttaquantX = pointFlagDefenseurX + rightRelativePos;
          pointArriveeDefenseurX = pointFlagDefenseurX + leftRelativePos;
      }
  }
  else
  {
      if ( pointFlagAttaquantX <= pointFlagDefenseurX )
      {
          pointArriveeAttaquantX = pointFlagDefenseurX + rightRelativePos;
          pointArriveeDefenseurX = pointFlagDefenseurX + leftRelativePos;
      }
      else
      {
          pointArriveeAttaquantX = pointFlagDefenseurX +leftRelativePos;
          pointArriveeDefenseurX = pointFlagDefenseurX + rightRelativePos;
      }
  }

  QPointF pointArriveeAttaquant(pointArriveeAttaquantX,pointArriveeY);
  QPointF pointArriveeDefenseur(pointArriveeDefenseurX,pointArriveeY);
  kDebug() << "3: " << pointArriveeAttaquant << " ; " << pointArriveeDefenseur << endl;

  QString sndRoulePath;
  AnimSprite* defenderSprite;
  AnimSprite* attackingSprite;
  
  if (!paysAttaquant->spritesInfantry().isEmpty())
  {
	/*InfantrySprite**/ attackingSprite = new InfantrySprite(
	Sprites::SkinSpritesData::single().strData("infantry-id"),
	Sprites::SkinSpritesData::single().intData("infantry-width"),
	Sprites::SkinSpritesData::single().intData("infantry-height"),
	Sprites::SkinSpritesData::single().intData("infantry-frames"),
	Sprites::SkinSpritesData::single().intData("infantry-versions"),
	m_theWorld->zoom(),
	backGnd(),
	200);

	paysAttaquant->spritesInfantry().hideAndRemoveFirst();
  }
  else
  {
	if (!paysAttaquant->spritesCavalry().isEmpty())
  	{
		/*CavalrySprite**/ attackingSprite = new CavalrySprite(
      		Sprites::SkinSpritesData::single().strData("cavalry-id"),
		Sprites::SkinSpritesData::single().intData("cavalry-width"),
		Sprites::SkinSpritesData::single().intData("cavalry-height"),
		Sprites::SkinSpritesData::single().intData("cavalry-frames"),
		Sprites::SkinSpritesData::single().intData("cavalry-versions"),
		m_theWorld->zoom(),
		backGnd(),
		200);

		paysAttaquant->spritesCavalry().hideAndRemoveFirst();
	}
	else
	{
		/*CannonSprite**/ attackingSprite = new CannonSprite(
      		Sprites::SkinSpritesData::single().strData("cannon-id"),
      		Sprites::SkinSpritesData::single().intData("cannon-width"),
      		Sprites::SkinSpritesData::single().intData("cannon-height"),
      		Sprites::SkinSpritesData::single().intData("cannon-frames"),
      		Sprites::SkinSpritesData::single().intData("cannon-versions"),
      		m_theWorld->zoom(),
      		backGnd(),
      		200);

		paysAttaquant->spritesCannons().hideAndRemoveFirst();
	}
  }

  attackingSprite-> setAttacker();
  attackingSprite->setupTravel(paysAttaquant, paysDefenseur, &pointArriveeAttaquant);
  (pointDepartAttaquantX <= pointArriveeAttaquantX) ? attackingSprite-> setLookRight() : attackingSprite-> setLookLeft();
  m_animFighters->addSprite(attackingSprite);
		
  sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roule.wav");
  if (sndRoulePath.isNull())
  {
	KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br/>Program cannot continue"), i18n("Error !"));
	exit(2);
  }
  if (KsirkSettings::soundEnabled())
  {
	m_audioPlayer->setCurrentSource(sndRoulePath);
	m_audioPlayer->play();
  }

  if (!paysDefenseur->spritesInfantry().isEmpty())
  {
  	kDebug() << "4" << endl;
		
	/*InfantrySprite**/ defenderSprite = new InfantrySprite(
	Sprites::SkinSpritesData::single().strData("infantry-id"),
	Sprites::SkinSpritesData::single().intData("infantry-width"),
	Sprites::SkinSpritesData::single().intData("infantry-height"),
	Sprites::SkinSpritesData::single().intData("infantry-frames"),
	Sprites::SkinSpritesData::single().intData("infantry-versions"),
	m_theWorld->zoom(),
	backGnd(),
	200);

	paysDefenseur->spritesInfantry().hideAndRemoveFirst();
  }
  else
  {
	if (!paysDefenseur->spritesCavalry().isEmpty())
  	{
		kDebug() << "4" << endl;
		
		/*CavalrySprite**/ defenderSprite = new CavalrySprite(
		Sprites::SkinSpritesData::single().strData("cavalry-id"),
		Sprites::SkinSpritesData::single().intData("cavalry-width"),
		Sprites::SkinSpritesData::single().intData("cavalry-height"),
		Sprites::SkinSpritesData::single().intData("cavalry-frames"),
		Sprites::SkinSpritesData::single().intData("cavalry-versions"),
		m_theWorld->zoom(),
		backGnd(),
		200);
		
		paysDefenseur->spritesCavalry().hideAndRemoveFirst();
	}
	else
	{
		kDebug() << "4" << endl;
  		/*CannonSprite**/ defenderSprite = new CannonSprite(
		Sprites::SkinSpritesData::single().strData("cannon-id"),
		Sprites::SkinSpritesData::single().intData("cannon-width"),
		Sprites::SkinSpritesData::single().intData("cannon-height"),
		Sprites::SkinSpritesData::single().intData("cannon-frames"),
		Sprites::SkinSpritesData::single().intData("cannon-versions"),
		m_theWorld->zoom(),
		backGnd(),
		200);

		paysDefenseur->spritesCannons().hideAndRemoveFirst();
	}
  }

  defenderSprite-> setDefendant();
  defenderSprite-> setupTravel(paysDefenseur, paysDefenseur, &pointArriveeDefenseur);
  (pointDepartDefenseurX <= pointArriveeDefenseurX) ? defenderSprite-> setLookRight() : defenderSprite-> setLookLeft();
  m_animFighters->addSprite(defenderSprite);
		
  sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roule.wav");
  if (sndRoulePath.isNull())
  {
	KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br/>Program cannot continue"), i18n("Error !"));
	exit(2);
  }
  if (KsirkSettings::soundEnabled())
  {
	m_audioPlayer->setCurrentSource(sndRoulePath);
	m_audioPlayer->play();
  }
  kDebug() << "5" << endl;
}

void KGameWindow::animCombat()
{
 kDebug()<< endl;
  m_animFighters->changeTarget(this, SLOT(slotFiringFinished(AnimSpritesGroup*)));

  AnimSpritesGroup::iterator it, it_end;
  it = m_animFighters->begin(); it_end = m_animFighters->end();
  for (; it != it_end; it++)
  {
    kDebug() << "a sprite position: " << (*it)->pos() << endl;
    CannonSprite* sprite = (CannonSprite*)(*it);
    sprite-> changeSequence(
        Sprites::SkinSpritesData::single().strData("firing-id"),
        Sprites::SkinSpritesData::single().intData("firing-width"),
        Sprites::SkinSpritesData::single().intData("firing-height"),
        Sprites::SkinSpritesData::single().intData("firing-frames"),
        Sprites::SkinSpritesData::single().intData("firing-versions"));

    qreal firingRelativePos = (Sprites::SkinSpritesData::single().intData("cannon-width") - Sprites::SkinSpritesData::single().intData("firing-width"))*m_theWorld->zoom();
    if (sprite-> looksToLeft()) 
    {
      sprite-> setPos(
        sprite-> x() + firingRelativePos,
        sprite-> y() );
    }
    else
    {
      sprite-> setPos(
      sprite-> x() + firingRelativePos,
                      sprite-> y() );
    }
    sprite->setAnimated(1);

    QString sndCanonPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/canon.wav");
    if (sndCanonPath.isNull())
    {
      KMessageBox::error(0,
          i18n("Sound canon not found - Verify your installation<br/>Program cannot continue"), i18n("Error !"));
      exit(2);
    }
    if (KsirkSettings::soundEnabled())
    {
      m_audioPlayer->setCurrentSource(sndCanonPath);
      m_audioPlayer->play();
    }
  }
}

// not currently in use
void KGameWindow::stopCombat()
{
/*
//   kDebug()<<"KGameWindow::stopCombat"<< endl;
  AnimSpritesGroup::iterator it = m_animFighters->begin();
  while (it != m_animFighters->end())
  {
    CannonSprite* sprite = (CannonSprite*)(*it);
//     it = m_animFighters.remove(it);
    sprite-> changeSequence(
        Sprites::SkinSpritesData::single().strData("cannon-id"),
                            Sprites::SkinSpritesData::single().intData("cannon-width"),
                            Sprites::SkinSpritesData::single().intData("cannon-height"),
                            Sprites::SkinSpritesData::single().intData("cannon-frames"), 
        Sprites::SkinSpritesData::single().intData("cannon-versions"));
    if (sprite-> looksToLeft()) 
    {
      sprite-> setPos(
        sprite->x() + (Sprites::SkinSpritesData::single().intData("cannon-width"))*m_theWorld->zoom(),
        sprite-> y() );
    }
    else 
    {
//       sprite-> setX(sprite-> x());
    }
  }*/
}

void KGameWindow::animExplosion(int who,Country *paysAttaquant, Country *paysDefenseur)
{
  kDebug() << "KGameWindow::animExplosion " << who << endl;

  m_animFighters->changeTarget(this, SLOT(slotExplosionFinished(AnimSpritesGroup*)));

  kDebug() << "KGameWindow::animExplosion hidden; " << m_animFighters->size() << " fighters" << endl;
  AnimSpritesGroup::iterator it = m_animFighters->begin();
  for (;it != m_animFighters->end();it++)
  {
    AnimSprite* sprite = (AnimSprite*)(*it);
    if ( (who == 2) // both are killed
        || ((who == 0) && (sprite-> isAttacker()))  // Attacker is killed
        || ((who == 1) && (sprite-> isDefendant())) ) // Defender is killed
    {
	sprite-> changeSequence(
			Sprites::SkinSpritesData::single().strData("exploding-id"),
			Sprites::SkinSpritesData::single().intData("exploding-width"),
			Sprites::SkinSpritesData::single().intData("exploding-height"),
			Sprites::SkinSpritesData::single().intData("exploding-frames"),
			Sprites::SkinSpritesData::single().intData("exploding-versions"));

	if (sprite->isAttacker())
	{
		kDebug() << "  removing a sprite" << endl;
		//kDebug() << "i attack" << i << endl;
		kDebug() << "NKA" << NKA << endl;
			
		sprite->setAnimated(NKA);

		QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
		if (sndCrashPath.isNull())
		{
			KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error !"));
			exit(2);
		}
		if (KsirkSettings::soundEnabled())
		{
			m_audioPlayer->setCurrentSource(sndCrashPath);
			m_audioPlayer->play();
		}
	}
	
	if (sprite->isDefendant())
	{
		kDebug() << "  removing a sprite" << endl;
		kDebug() << "NKD" << NKD << endl;
			
		sprite->setAnimated(NKD);

		QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
		if (sndCrashPath.isNull())
		{
			KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error !"));
			exit(2);
		}
		if (KsirkSettings::soundEnabled())
		{
			m_audioPlayer->setCurrentSource(sndCrashPath);
			m_audioPlayer->play();
		}
	}
    }
    else  // the sprite is not the one (or one the several) killed
    {
      kDebug() << "  keeping a sprite" << endl;
      
      if (sprite->isAttacker())
      {
		if ((paysAttaquant->nbArmies() % 10) == 0)
		{
			sprite-> changeSequence(
          			Sprites::SkinSpritesData::single().strData("cannon-id"),
                              	Sprites::SkinSpritesData::single().intData("cannon-width"),
                              	Sprites::SkinSpritesData::single().intData("cannon-height"),
                              	Sprites::SkinSpritesData::single().intData("cannon-frames"), 
          			Sprites::SkinSpritesData::single().intData("cannon-versions"));
		}
		else
		{
			if ((paysAttaquant->nbArmies() % 5) == 0)
			{
				sprite-> changeSequence(
          				Sprites::SkinSpritesData::single().strData("cavalry-id"),
					Sprites::SkinSpritesData::single().intData("cavalry-width"),
					Sprites::SkinSpritesData::single().intData("cavalry-height"),
					Sprites::SkinSpritesData::single().intData("cavalry-frames"), 
					Sprites::SkinSpritesData::single().intData("cavalry-versions"));
			}
			else
			{
				sprite-> changeSequence(
          				Sprites::SkinSpritesData::single().strData("infantry-id"),
					Sprites::SkinSpritesData::single().intData("infantry-width"),
					Sprites::SkinSpritesData::single().intData("infantry-height"),
					Sprites::SkinSpritesData::single().intData("infantry-frames"), 
					Sprites::SkinSpritesData::single().intData("infantry-versions"));
			}
		}
      }

      if (sprite->isDefendant())
      {
		if ((paysDefenseur->nbArmies() % 10) == 0)
		{
			sprite-> changeSequence(
          			Sprites::SkinSpritesData::single().strData("cannon-id"),
                              	Sprites::SkinSpritesData::single().intData("cannon-width"),
                              	Sprites::SkinSpritesData::single().intData("cannon-height"),
                              	Sprites::SkinSpritesData::single().intData("cannon-frames"), 
          			Sprites::SkinSpritesData::single().intData("cannon-versions"));
		}
		else
		{
			if ((paysDefenseur->nbArmies() % 5) == 0)
			{
				sprite-> changeSequence(
          				Sprites::SkinSpritesData::single().strData("cavalry-id"),
					Sprites::SkinSpritesData::single().intData("cavalry-width"),
					Sprites::SkinSpritesData::single().intData("cavalry-height"),
					Sprites::SkinSpritesData::single().intData("cavalry-frames"), 
					Sprites::SkinSpritesData::single().intData("cavalry-versions"));
			}
			else
			{
				sprite-> changeSequence(
          				Sprites::SkinSpritesData::single().strData("infantry-id"),
					Sprites::SkinSpritesData::single().intData("infantry-width"),
					Sprites::SkinSpritesData::single().intData("infantry-height"),
					Sprites::SkinSpritesData::single().intData("infantry-frames"), 
					Sprites::SkinSpritesData::single().intData("infantry-versions"));
			}
		}
      } 

      sprite->setStatic();
      m_animFighters->oneArrived(0);
    }
  }

  
  kDebug() << "  loop done" << endl;

  /*QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
  if (sndCrashPath.isNull())
  {
    KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error !"));
    exit(2);
  }
  if (KsirkSettings::soundEnabled())
  {
    m_audioPlayer->setCurrentSource(sndCrashPath);
    m_audioPlayer->play();
  }*/
  kDebug() << "KGameWindow::animExplosion finished" << endl;
//   m_frame-> initTimer();
//    kDebug()<<"OUT KGameWindow::animExplosion"<<endl;
}

void KGameWindow::stopExplosion()
{
    kDebug()<<"KGameWindow::stopExplosion"<<endl;
    m_animFighters->hideAndRemoveAll();
}

void KGameWindow::initCombatBringBack(Country *paysAttaquant, Country *paysDefenseur)
{
    kDebug()<<"KGameWindow::initCombatBringBack"<<endl;
    int who = 0;
    if ((NKD != 0)&&(NKA != 0)) who = 2;
    else if (NKA != 0) who = 0;
    else if (NKD != 0) who = 1;
    else KMessageBox::information(0, i18n("Problem : no one destroyed"));
    
    CannonSprite *newSprite;

    qreal flagYDiff = Sprites::SkinSpritesData::single().intData("fighters-flag-y-diff")*m_theWorld->zoom();
    qreal leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cannon-width"))*m_theWorld->zoom();
    qreal rightRelativePos = (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("flag-width"))*m_theWorld->zoom();
    
    if (who == 0) //Attaquant detruit, ramene defenseur
    {
        newSprite = new CannonSprite(
            Sprites::SkinSpritesData::single().strData("cannon-id"),
            Sprites::SkinSpritesData::single().intData("cannon-width"),
            Sprites::SkinSpritesData::single().intData("cannon-height"),
            Sprites::SkinSpritesData::single().intData("cannon-frames"),
            Sprites::SkinSpritesData::single().intData("cannon-versions"),
            m_theWorld->zoom(),
            backGnd(),
            200);
        if ((paysAttaquant-> pointFlag().x() <= paysDefenseur-> pointFlag().x())
        && !(
        (qAbs(paysAttaquant-> pointFlag().x()-paysDefenseur-> pointFlag().x()) > (backGnd()-> boundingRect().width() / 2))
            && (paysAttaquant->communicateWith(paysDefenseur))
        ))
        {
            newSprite-> setPos(  (paysDefenseur-> pointFlag())+QPointF(rightRelativePos, flagYDiff));
        }
        else
        {
            newSprite-> setPos(  (paysDefenseur-> pointFlag())+QPointF(leftRelativePos, flagYDiff));
        }
	connect(newSprite,SIGNAL(atDestination(AnimSprite*)),this,SLOT(slotBring(AnimSprite*)));
        ((AnimSprite*)newSprite)-> setupTravel(paysDefenseur, paysDefenseur, newSprite-> pos(), paysDefenseur-> pointCannon());
        newSprite-> turnTowardDestination();
	m_animFighters->addSprite(newSprite);

        QString sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roule.wav");
        if (sndRoulePath.isNull())
        {
            KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br/>Program cannot continue"), i18n("Error !"));
            exit(2);
        }
        m_audioPlayer->setCurrentSource(sndRoulePath);
        m_audioPlayer->play();
    }
    else if (who == 1) //Defenseur detruit, ramene Attaquant
    {
      newSprite = new CannonSprite(
          Sprites::SkinSpritesData::single().strData("cannon-id"),
          Sprites::SkinSpritesData::single().intData("cannon-width"),
          Sprites::SkinSpritesData::single().intData("cannon-height"),
          Sprites::SkinSpritesData::single().intData("cannon-frames"),
          Sprites::SkinSpritesData::single().intData("cannon-versions"),
          m_theWorld->zoom(),
          backGnd(),
          200);
        if ((paysAttaquant-> pointFlag().x() <= paysDefenseur-> pointFlag().x())
        && !(
        (qAbs(paysAttaquant-> pointFlag().x()-paysDefenseur-> pointFlag().x()) > (backGnd()-> boundingRect().width() / 2))
            && (paysAttaquant->communicateWith(paysDefenseur))
        ))
        {
            newSprite-> setPos( (paysDefenseur-> pointFlag())+QPointF(leftRelativePos,flagYDiff));
        }
        else
        {
            newSprite-> setPos( (paysDefenseur-> pointFlag())+QPointF(rightRelativePos,flagYDiff));
        }
        connect(newSprite,SIGNAL(atDestination(AnimSprite*)),this,SLOT(slotBring(AnimSprite*)));
	((AnimSprite*)newSprite)-> setupTravel(paysDefenseur, paysAttaquant, newSprite-> pos(), paysAttaquant-> pointCannon());
        newSprite-> turnTowardDestination();
	m_animFighters->addSprite(newSprite);

        QString sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roule.wav");
        if (sndRoulePath.isNull())
        {
            KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br/>Program cannot continue"), i18n("Error !"));
            exit(2);
        }
        m_audioPlayer->setCurrentSource(sndRoulePath);
        m_audioPlayer->play();
    }
    else if (who == 2) 
    {
    } //Attaquant ET Defenseur detruits
    else  // error
    {
      kError() << k_funcinfo << __FILE__ << __LINE__ << i18n("Bug: who should be 0, 1 or 2.");
      exit(1);
    }
}

/**
  * Disconnects the mouse events signals from their slots to avoid human
  * player actions when it is the turn of the AI
  */
void KGameWindow::disconnectMouse()
{
/*  if ( ! disconnect(m_frame, SIGNAL(signalLeftButtonDown(const QPoint &)),
                    this, SLOT(slotLeftButtonDown(const QPoint&))))
    kError() << "cannot connect slotLeftButtonDown !" << endl;
  if ( ! disconnect(m_frame, SIGNAL(signalLeftButtonUp(const QPoint &)),
                    this, SLOT(slotLeftButtonUp(const QPoint &))))
    kError() << "cannot connect slotLeftButtonUp !" << endl;
  if ( ! disconnect(m_frame, SIGNAL(signalRightButtonDown(const QPoint &)),
                    this, SLOT(slotRightButtonDown(const QPoint &))))
    kError() << "cannot connect slotRightButtonDown !" << endl;*/
}

/**
  * Reconnect the mouse events signals to their slots to allow human players to
  * play
  */
void KGameWindow::reconnectMouse()
{
/*  if ( ! connect(m_frame, SIGNAL(signalLeftButtonDown(const QPoint &)),
                    this, SLOT(slotLeftButtonDown(const QPoint&))))
  kError() << "cannot connect slotLeftButtonDown !" << endl;
  if ( ! connect(m_frame, SIGNAL(signalLeftButtonUp(const QPoint &)),
                    this, SLOT(slotLeftButtonUp(const QPoint &))))
  kError() << "cannot connect slotLeftButtonUp !" << endl;
  if ( ! connect(m_frame, SIGNAL(signalRightButtonDown(const QPoint &)),
                    this, SLOT(slotRightButtonDown(const QPoint &))))
  kError() << "cannot connect slotRightButtonDown !" << endl;*/
}

bool KGameWindow::haveAnimFighters() const
{
    return !m_animFighters->empty();
}

} // closing namespace KsirK
