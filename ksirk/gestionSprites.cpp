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
#include "Sprites/arrowsprite.h"
#include "Sprites/skinSpritesData.h"
#include "GameLogic/KMessageParts.h"
#include "GameLogic/onu.h"
#include "fightArena.h"

// include files for QT
#include <qmetaobject.h>
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qinputdialog.h>

// include files for KDE
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <KLocalizedString>
#include <kconfig.h>
#include <kstandardaction.h>
#include <phonon/mediaobject.h>
#include <kstandarddirs.h>
#include "ksirk_debug.h"
#include <KToolBar>

#include <assert.h>

namespace Ksirk
{
using namespace GameLogic;

/**
 * Prepares the sprites to be moved : removes the nb necessary sprites from
 * source, creates the moving sprites and gives them their destination, etc
 */
bool KGameWindow::initArmiesMovement(unsigned int nbABouger, Country *firstCountry, Country *secondCountry)
{
  qCDebug(KSIRK_LOG) << "-> " << nbABouger ;
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
    sprite = new CannonSprite(m_theWorld->zoom(), backGnd(), 200);
    firstCountry-> decrNbArmies(10);
  }
  else if ((firstCountry-> nbArmies() > 5) && (nbABouger == 5)
          && (nbABouger < firstCountry-> nbArmies()))
  {
    sprite = new CavalrySprite(m_theWorld->zoom(), backGnd(), 200);
    firstCountry-> decrNbArmies(5);
  }
  else if ((firstCountry-> nbArmies() > 1) && (nbABouger == 1)
          && (nbABouger < firstCountry-> nbArmies()))
  {
    sprite = new InfantrySprite(m_theWorld->zoom(), backGnd(), 200);
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
//   qCDebug(KSIRK_LOG) << "initArmiesMovement returns true";
  return true;
}


AnimSprite* KGameWindow::initArmiesMultipleCombat(unsigned int nbA,
    Country *firstCountry,
    Country *secondCountry, QPointF dest)
{
  qCDebug(KSIRK_LOG) << "-> " << nbA ;
  KMessageParts messageParts;

  //AnimSpritesGroup* newGroup = new AnimSpritesGroup(this,SLOT(slotMovingArmiesArrived(AnimSpritesGroup*)));
  //m_animSpritesGroups.push_back(newGroup);
  AnimSprite* sprite = 0;
  if ((!firstCountry->spritesCannons().isEmpty()) && (nbA == 10))
  {
      qCDebug(KSIRK_LOG) << "cannon";
      sprite = new CannonSprite(m_theWorld->zoom(), backGnd(), 200);

    firstCountry->spritesCannons().hideAndRemoveFirst();
  }
  else if ((!firstCountry->spritesCavalry().isEmpty()) && (nbA == 5))
  {
      qCDebug(KSIRK_LOG) << "cavalry";
      sprite = new CavalrySprite(m_theWorld->zoom(), backGnd(), 200);

      firstCountry->spritesCavalry().hideAndRemoveFirst();
  }
  else if ((!firstCountry->spritesInfantry().isEmpty()) && (nbA == 1))
  {
    qCDebug(KSIRK_LOG) << "infantry";
    sprite = new InfantrySprite(m_theWorld->zoom(), backGnd(), 200);

    firstCountry->spritesInfantry().hideAndRemoveFirst();
  }
  // cannot comment out this comment below otherwise will use sprite uninitialized
  else
  {
    messageParts << I18N_NOOP("Cannot move %1 armies from %2 to %3") 
      << QString::number(nbA) 
      << firstCountry->name() 
      << secondCountry->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return 0;
  }
  //connect(sprite,SIGNAL(atDestination(AnimSprite*)),this,SLOT(slotMovingArmyArrived(AnimSprite*)));
  
  if (backGnd()->bgIsArena())
  {
    int relativePosInArena=0;

    if (firstCountry->name() == secondCountry->name())
    {
      sprite->setDefendant();
      relativePosInArena = relativePosInArenaDefense;
      //relativePosInArenaDefense++;
    }
    else
    {
      sprite->setAttacker();
      relativePosInArena = relativePosInArenaAttack;
      //relativePosInArenaAttack++;
    }

    qCDebug(KSIRK_LOG) << "****Test relativePos**** " << relativePosInArena;
    QPointF dep = determinePointDepartArena(firstCountry,relativePosInArena);

    sprite-> setPos(dep);

    qCDebug(KSIRK_LOG) << "****dep  point**** " << dep;
    qCDebug(KSIRK_LOG) << "****dest point**** " << dest;
    sprite->setupTravel(firstCountry, secondCountry,dep,dest);
  }
  else
  {
    qCDebug(KSIRK_LOG) << "***************Setup travel normal ************** ";
    sprite->setupTravel(firstCountry, secondCountry,&dest);
  }

  qCDebug(KSIRK_LOG) << "-> "<< firstCountry->name() << secondCountry->name();
  //sprite->setupTravel(firstCountry, secondCountry,&dest);
  //newGroup->addSprite(sprite);
//   qCDebug(KSIRK_LOG) << "add a sprite 1";
//   m_animFighters->addSprite(sprite);
  //firstCountry-> createArmiesSprites();
  //sprite->setAnimated();
//   qCDebug(KSIRK_LOG) << "initArmiesMovement returns true";
  return sprite;
}


QPointF KGameWindow::determinePointDepartArena(Country *pays, int relativePos)
{
  return( QPointF(pays-> pointInfantry().x()*m_theWorld->zoom(),(pays-> pointInfantry().y()+(1-2*(relativePos%2))*(Sprites::SkinSpritesData::single().intData("infantry-height")+8)*((relativePos+1)/2))*m_theWorld->zoom()));
}


void KGameWindow::determinePointArrivee(
        QPointF& pointArriveeAttaquant,
        QPointF& pointArriveeDefenseur)
{
  Country *attackingCountry = firstCountry();
  Country *defendingCountry = secondCountry();
  if (attackingCountry == 0 || m_secondCountry == 0)
  {
    qCCritical(KSIRK_LOG) << "attackingCountry=" << (void*)attackingCountry << "defendingCountry=" << (void*)defendingCountry;
    return;
  }
  
  //  - attacker's flag point
  qreal pointFlagAttaquantX = attackingCountry-> pointFlag().x()*m_theWorld->zoom();

  //  - defender's flag point
  qreal pointFlagDefenseurX;
  pointFlagDefenseurX = defendingCountry-> pointFlag().x()*m_theWorld->zoom();
  
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

  if (!attackingCountry->spritesInfantry().isEmpty())
  {
    // We must know
    //  - attacker's departure point (pointInfantry)
    pointDepartAttaquantX = attackingCountry-> pointInfantry().x()*m_theWorld->zoom();
    pointDepartAttaquantY = attackingCountry-> pointInfantry().y()*m_theWorld->zoom();
  }
  else if (!attackingCountry->spritesCavalry().isEmpty())
  {
    // We must know
    //  - attacker's departure point (pointCavalry)
    pointDepartAttaquantX = attackingCountry-> pointCavalry().x()*m_theWorld->zoom();
    pointDepartAttaquantY = attackingCountry-> pointCavalry().y()*m_theWorld->zoom();
  }
  else
  {
    // We must know
    //  - attacker's departure point (pointCannon)
      pointDepartAttaquantX = attackingCountry-> pointCannon().x()*m_theWorld->zoom();
      pointDepartAttaquantY = attackingCountry-> pointCannon().y()*m_theWorld->zoom();
  }

  if (!defendingCountry->spritesInfantry().isEmpty())
  {
    //  - defender's departure point (pointInfantry)
    pointDepartDefenseurX = defendingCountry-> pointInfantry().x()*m_theWorld->zoom();
    pointDepartDefenseurY = defendingCountry-> pointInfantry().y()*m_theWorld->zoom();
  
  }
  else if (!defendingCountry->spritesCavalry().isEmpty())
  {
    //  - defender's departure point (pointCavalry)
    pointDepartDefenseurX = defendingCountry-> pointCavalry().x()* m_theWorld->zoom();
    pointDepartDefenseurY = defendingCountry-> pointCavalry().y()* m_theWorld->zoom();
  }
  else
  {
    //  - defender's departure point (pointCannon)
    pointDepartDefenseurX = defendingCountry-> pointCannon().x()*  m_theWorld->zoom();
    pointDepartDefenseurY = defendingCountry-> pointCannon().y()*  m_theWorld->zoom();
  }

  // vertical meet point
  int diff = Sprites::SkinSpritesData::single().intData("flag-height") - Sprites::SkinSpritesData::single().intData("cannon-height");
  pointArriveeY = (((defendingCountry-> pointFlag().y() + diff))* m_theWorld->zoom()) ;
  
  qCDebug(KSIRK_LOG) << "2 " << defendingCountry-> pointFlag().y() << pointArriveeY;
  if (!attackingCountry->communicateWith(defendingCountry))
  {
    qCCritical(KSIRK_LOG) << "Error in KGameWindow::determinePointDepartArena: " << attackingCountry-> name() << "  and "
            << defendingCountry-> name() << " do not communicate!";
    exit(2);
  }
  
  // If the flag of the attacker is to the left of the flag of the defender,
  // then the arriving point of the attacker is to the left, else it is to
  // the right
  // The situation is reversed if the one of the attacker will need to go
  // through the world limit
  if (!attackingCountry->spritesInfantry().isEmpty())
  {
    leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("infantry-width"))*m_theWorld->zoom();
  }
  else
  {
    if (!attackingCountry->spritesCavalry().isEmpty())
    {
      leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cavalry-width"))*m_theWorld->zoom();
    }
    else
    {
      leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cannon-width"))*m_theWorld->zoom();
    }
  }
  
  qreal rightRelativePos = (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("flag-width"))*m_theWorld->zoom();

  if (!(qAbs(pointFlagAttaquantX-pointFlagDefenseurX) > (backGnd()-> boundingRect().width() / 2)))
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

  pointArriveeAttaquant.setX(pointArriveeAttaquantX);
  pointArriveeAttaquant.setY(pointArriveeY);
  pointArriveeDefenseur.setX(pointArriveeDefenseurX);
  pointArriveeDefenseur.setY(pointArriveeY);
  qCDebug(KSIRK_LOG) << "3: " << pointArriveeAttaquant << " ; " << pointArriveeDefenseur;
}


// point arriveedefenseur inutile
void KGameWindow::determinePointArriveeForArena(
    int relative,
    QPointF& pointArriveeAttaquant,
    QPointF& pointArriveeDefenseur)
{
  Country *attackingCountry = firstCountry();
  Country *defendingCountry = secondCountry();
  
  qCDebug(KSIRK_LOG) << m_firstCountry->name() << "("<<(void*)m_firstCountry<<")"
      << m_secondCountry->name() << "("<<(void*)m_secondCountry<<")";
  qCDebug(KSIRK_LOG) << attackingCountry->name() << "("<<(void*)attackingCountry<<")"
      << defendingCountry->name() << "("<<(void*)defendingCountry<<")"
      << relative << pointArriveeAttaquant << pointArriveeDefenseur;
  //  - attacker's flag point
  qreal pointFlagAttaquantX = attackingCountry-> pointFlag().x()*m_theWorld->zoom();

  // in case of arena, the meet will be between the two countries
  qreal pointFlagDefenseurX = backGnd()-> boundingRect().width() / 2;
  
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

  qCDebug(KSIRK_LOG) << "1" << attackingCountry->spritesInfantry().isEmpty()
  << attackingCountry->name()
  << attackingCountry->nbArmies()
  << attackingCountry->owner()->name()
  << ((attackingCountry->name()==m_firstCountry->name())?attackingCountry->owner()->getNbAttack():attackingCountry->owner()->getNbDefense());
  
  if ( attackingCountry->nbArmies()%5 != 0
    && ((attackingCountry->nbArmies() % 5) >= ((attackingCountry->name()==m_firstCountry->name())?attackingCountry->owner()->getNbAttack():attackingCountry->owner()->getNbDefense())))
//   if (!attackingCountry->spritesInfantry().isEmpty()
//       && ((attackingCountry->nbArmies() % 5) >= ((attackingCountry==m_firstCountry)?attackingCountry->owner()->getNbAttack():attackingCountry->owner()->getNbDefense())))
      {
    qCDebug(KSIRK_LOG) << "infantry" << attackingCountry->name() << relative;
    // We must know
    //  - attacker's departure point (pointInfantry)
    QPointF dep = determinePointDepartArena(attackingCountry, relative);
    qCDebug(KSIRK_LOG) << "dep=" << dep;
    pointDepartAttaquantX = dep.x();
    pointDepartAttaquantY = dep.y();
  }
  else if (!attackingCountry->spritesCavalry().isEmpty())
  {
    qCDebug(KSIRK_LOG) << "cavalry" << attackingCountry->pointCavalry();
    // We must know
    //  - attacker's departure point (pointCavalry)
    pointDepartAttaquantX = attackingCountry-> pointCavalry().x()*m_theWorld->zoom();
    pointDepartAttaquantY = attackingCountry-> pointCavalry().y()*m_theWorld->zoom();
  }
  else
  {
    qCDebug(KSIRK_LOG) << "cannon" << attackingCountry->pointCannon();
    // We must know
    //  - attacker's departure point (pointCannon)
      pointDepartAttaquantX = attackingCountry-> pointCannon().x()*m_theWorld->zoom();
      pointDepartAttaquantY = attackingCountry-> pointCannon().y()*m_theWorld->zoom();
  }

  // problem here: when handling defender, attacker sprites are already moving so
  // if it has no more infantry sprites, it will not be able to enter this loop
  // solution: use the number of armies (equivalent to the number of stationary
  // sprites) and not the number of infantry sprites
  // defendingCountry->spritesInfantry().isEmpty() === (defendingCountry->nbArmies()%5==0)
  unsigned int num = 0;
  if (defendingCountry->name()==m_firstCountry->name())
  {
    num = defendingCountry->owner()->getNbAttack();
  }
  else
  {
    num = defendingCountry->owner()->getNbDefense();
  }
  qCDebug(KSIRK_LOG) << "2"
    << defendingCountry->spritesInfantry().isEmpty()
    << defendingCountry->name()
    << defendingCountry->nbArmies()
    << defendingCountry->owner()->name()
    << num;
  if ( defendingCountry->nbArmies()%5 != 0
      && ((defendingCountry->nbArmies() % 5) >= num)
      )
//   if (!defendingCountry->spritesInfantry().isEmpty()
//       && ((defendingCountry->nbArmies() % 5) >= defendingCountry->owner()->getNbDefense()))
  {
    qCDebug(KSIRK_LOG) << "infantry" << defendingCountry->name() << relative;
    //  - defender's departure point (pointInfantry)
    QPointF dep = determinePointDepartArena(defendingCountry, relative);
    pointDepartDefenseurX = dep.x();
    pointDepartDefenseurY = dep.y();
    qCDebug(KSIRK_LOG) << "dep=" << dep;
  }
  else if (!defendingCountry->spritesCavalry().isEmpty())
  {
    qCDebug(KSIRK_LOG) << "cavalry" << defendingCountry->pointCavalry();
    //  - defender's departure point (pointCavalry)
    pointDepartDefenseurX = defendingCountry-> pointCavalry().x()*m_theWorld->zoom();
    pointDepartDefenseurY = defendingCountry-> pointCavalry().y()*m_theWorld->zoom();
  }
  else
  {
    qCDebug(KSIRK_LOG) << "cannon" << defendingCountry->pointCannon();
    //  - defender's departure point (pointCannon)
    pointDepartDefenseurX = defendingCountry-> pointCannon().x()*m_theWorld->zoom();
    pointDepartDefenseurY = defendingCountry-> pointCannon().y()*m_theWorld->zoom();
  }


  // vertical meet point
  // in case of arena, the vertical meet will be as soon as it's possible
  qCDebug(KSIRK_LOG) << "Argh " << pointDepartAttaquantY << pointDepartDefenseurY;
  pointArriveeY = (pointDepartAttaquantY+pointDepartDefenseurY)/2;
  
  if (!attackingCountry->communicateWith(defendingCountry))
  {
    qCCritical(KSIRK_LOG) << "Error in KGameWindow::determinePointArriveeForArena: " << attackingCountry-> name() << "  and "
            << defendingCountry-> name() << " do not communicate!";
    exit(2);
  }
  // If the flag of the attacker is to the left of the flag of the defender,
  // then the arriving point of the attacker is to the left, else it is to
  // the right
  // The situation is reversed if the one of the attacker will need to go
  // through the world limit

  qreal factor = 0;
  if (!attackingCountry->spritesInfantry().isEmpty()
    && ((attackingCountry->nbArmies() % 5) >= attackingCountry->owner()->getNbAttack()))
  {
    factor = Sprites::SkinSpritesData::single().intData("infantry-width");
  }
  else if (!attackingCountry->spritesCavalry().isEmpty())
  {
    factor = Sprites::SkinSpritesData::single().intData("cavalry-width");
  }
  else
  {
    factor = Sprites::SkinSpritesData::single().intData("cannon-width");
  }
  leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + factor)*m_theWorld->zoom();

  
  qreal rightRelativePos = (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("flag-width"))*m_theWorld->zoom();

  //if (!(qAbs(pointFlagAttaquantX-pointFlagDefenseurX) > (backGnd()-> boundingRect().width() / 2)) || backGnd()->bgIsArena())
  //{
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

  pointArriveeAttaquant.setX(pointArriveeAttaquantX);
  pointArriveeAttaquant.setY(pointArriveeY);
  pointArriveeDefenseur.setX(pointArriveeDefenseurX);
  pointArriveeDefenseur.setY(pointArriveeY);
  qCDebug(KSIRK_LOG) << "Done: " << pointArriveeAttaquant << " ; " << pointArriveeDefenseur;
}


void KGameWindow::initCombatMovement()
{
  qCDebug(KSIRK_LOG) << "1";

  Country *attackingCountry = firstCountry();
  Country *defendingCountry = secondCountry();
  if (attackingCountry == 0 || defendingCountry == 0)
  {
    qCCritical(KSIRK_LOG) << "attackingCountry=" << (void*)attackingCountry << "defendingCountry=" << (void*)defendingCountry;
    return;
  }
  
  getRightDialog()->close();
  if (firstCountry() != 0 && firstCountry()->owner() != 0 
    && secondCountry() != 0 && secondCountry()->owner() != 0)
  {
    getRightDialog()->displayFightDetails(firstCountry(), secondCountry(),
        firstCountry()->owner()->getNbAttack(), secondCountry()->owner()->getNbDefense());
  }
  centerOnFight();  //center the game on the fight
  if  (isArena()
      && ! m_automaton->currentPlayer()->isAI()
      && !m_automaton->currentPlayer()->isVirtual())
  {
    qCDebug(KSIRK_LOG) << "Attack with arena" << endl;
    // init and display the arena view
    showArena();
  }

  m_animFighters->clear();
  m_animFighters->changeTarget(this,SLOT(slotMovingFightersArrived(AnimSpritesGroup*)));

  QString sndRoulePath;
  AnimSprite* defenderSprite = 0;
  AnimSprite* attackingSprite = 0;

  QPointF pointArriveeAttaquant(0,0);
  QPointF pointArriveeDefenseur(0,0);
 
  if (backGnd()->bgIsArena())
  {
    determinePointArriveeForArena(0,
        pointArriveeAttaquant, pointArriveeDefenseur);
  }
  else
  {
    determinePointArrivee(pointArriveeAttaquant, pointArriveeDefenseur);
  }

  if (!attackingCountry->spritesInfantry().isEmpty()
      && ((attackingCountry->nbArmies() % 5) >= attackingCountry->owner()->getNbAttack())
      && backGnd()->bgIsArena())
  {
    qCDebug(KSIRK_LOG) << "**NB-ATTACK**" << attackingCountry->owner()->getNbAttack();

    for (unsigned int i=0; i < attackingCountry->owner()->getNbAttack() ; i++)
    {
      attackingSprite = simultaneousAttack(1,KGameWindow::Attack);
      if (attackingSprite != 0)
      {
        qCDebug(KSIRK_LOG) << "add a sprite";
        m_animFighters->addSprite(attackingSprite);
      }
    }
    nbSpriteAttacking=attackingCountry->owner()->getNbAttack();
  }
  else
  {
    qCDebug(KSIRK_LOG) << "creating attackingSprite"
              << attackingCountry->name()
              << attackingCountry->nbArmies()
              << attackingCountry->spritesInfantry().size()
              << attackingCountry->spritesCavalry().size()
              << attackingCountry->spritesCannons().size();
    nbSpriteAttacking=1;
    if ( (attackingCountry->nbArmies() != 0)
      && attackingCountry->spritesInfantry().isEmpty()
      && attackingCountry->spritesCavalry().isEmpty()
      && attackingCountry->spritesCannons().isEmpty())
    {
      attackingCountry->createArmiesSprites();
    }
    if (!attackingCountry->spritesCavalry().isEmpty())
    {
      qCDebug(KSIRK_LOG) << "cavalry" << backGnd()->bgIsArena();
      if (backGnd()->bgIsArena())
      {
        attackingSprite = simultaneousAttack(5,KGameWindow::Attack);
      }
      else
      {
        attackingSprite = new CavalrySprite(m_theWorld->zoom(), backGnd(), 200);

        attackingCountry->spritesCavalry().hideAndRemoveFirst();
      }
    }
    else if (!attackingCountry->spritesCannons().isEmpty())
    {
      qCDebug(KSIRK_LOG) << "cannon" << backGnd()->bgIsArena();
      if (backGnd()->bgIsArena())
      {
        attackingSprite = simultaneousAttack(10,KGameWindow::Attack);
      }
      else
      {
        attackingSprite = new CannonSprite(m_theWorld->zoom(), backGnd(), 200);

        attackingCountry->spritesCannons().hideAndRemoveFirst();
      }
    }
    else if (!attackingCountry->spritesInfantry().isEmpty())
    {
      qCDebug(KSIRK_LOG) << "infantry" << backGnd()->bgIsArena();
      if (backGnd()->bgIsArena())
      {
        attackingSprite = simultaneousAttack(1,KGameWindow::Attack);
      }
      else
      {
        attackingSprite = new InfantrySprite(m_theWorld->zoom(), backGnd(), 200);

        attackingCountry->spritesInfantry().hideAndRemoveFirst();
      }
    }
    else 
    {
      qCDebug(KSIRK_LOG) << "No sprite on attacking country!";
      assert(false);
    }

    if (attackingSprite == 0)
    {
      qCCritical(KSIRK_LOG) << "ERROR: null attackingSprite at " << __FILE__ << ", line " << __LINE__;
//       return;
    }
    else
    {
      qCDebug(KSIRK_LOG) << "attackingSprite created";

      // Adding the number of attackers decoration
      if (attackingCountry->owner()->getNbAttack() == 1)
      {
        attackingSprite->addDecoration("mark1", QRect(0,0,10,10));
      }
      else if (attackingCountry->owner()->getNbAttack() == 2)
      {
        attackingSprite->addDecoration("mark2", QRect(0,0,10,10));
      }
      else
      {
        attackingSprite->addDecoration("mark3", QRect(0,0,10,10));
      }

      attackingSprite-> setAttacker();
      attackingSprite->setupTravel(attackingCountry, defendingCountry, &pointArriveeAttaquant);
      qCDebug(KSIRK_LOG) << "add a sprite 2";
      m_animFighters->addSprite(attackingSprite);
      //(pointDepartAttaquantX <= pointArriveeAttaquantX) ? attackingSprite-> setLookRight() : attackingSprite-> setLookLeft();
      sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roll.wav");
      if (sndRoulePath.isNull())
      {
        KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br>Program cannot continue"), i18n("Error!"));
        exit(2);
      }
      if (KsirkSettings::soundEnabled())
      {
        m_audioPlayer->setCurrentSource(sndRoulePath);
        m_audioPlayer->play();
      }
    }
  }

  if (!defendingCountry->spritesInfantry().isEmpty()
    && ((defendingCountry->nbArmies() % 5) >= defendingCountry->owner()->getNbDefense())
    && backGnd()->bgIsArena())
  {
    qCDebug(KSIRK_LOG) << "**NB-DEFENSE**" << defendingCountry->owner()->getNbDefense();

    for (unsigned int i=0;i< defendingCountry->owner()->getNbDefense();i++)
    {
      defenderSprite = simultaneousAttack(1,KGameWindow::Defense);
      if (defenderSprite != 0)
      {
        qCDebug(KSIRK_LOG) << "add a sprite";
        m_animFighters->addSprite(defenderSprite);
      }
    }
    nbSpriteDefending=defendingCountry->owner()->getNbDefense();
  }
  else
  {
    qCDebug(KSIRK_LOG) << "creating defenderSprite" << defendingCountry->name()
              << defendingCountry->nbArmies()
              << defendingCountry->spritesInfantry().size()
              << defendingCountry->spritesCavalry().size()
              << defendingCountry->spritesCannons().size();
    nbSpriteDefending=1;
    if ( (defendingCountry->nbArmies() != 0) 
      && defendingCountry->spritesInfantry().isEmpty()
      && defendingCountry->spritesCavalry().isEmpty()
      && defendingCountry->spritesCannons().isEmpty())
    {
      defendingCountry->createArmiesSprites();
    }
    if (!defendingCountry->spritesCavalry().isEmpty())
    {
      qCDebug(KSIRK_LOG) << "cavalry" << backGnd()->bgIsArena();
      if (backGnd()->bgIsArena())
      {
        defenderSprite = simultaneousAttack(5,KGameWindow::Defense);
      }
      else
      {
        defenderSprite = new CavalrySprite(m_theWorld->zoom(), backGnd(), 200);

        defendingCountry->spritesCavalry().hideAndRemoveFirst();
      }
    }
    else if (!defendingCountry->spritesCannons().isEmpty())
    {
      qCDebug(KSIRK_LOG) << "cannon" << backGnd()->bgIsArena();
      if (backGnd()->bgIsArena())
      {
        defenderSprite = simultaneousAttack(10,KGameWindow::Defense);
      }
      else
      {
        defenderSprite = new CannonSprite(m_theWorld->zoom(), backGnd(), 200);

        defendingCountry->spritesCannons().hideAndRemoveFirst();
      }
    }
    else if (!defendingCountry->spritesInfantry().isEmpty())
    {
      qCDebug(KSIRK_LOG) << "infantry" << backGnd()->bgIsArena();
      if (backGnd()->bgIsArena())
      {
        defenderSprite = simultaneousAttack(1,KGameWindow::Defense);
      }
      else
      {
        defenderSprite = new InfantrySprite(m_theWorld->zoom(), backGnd(), 200);

        defendingCountry->spritesInfantry().hideAndRemoveFirst();
      }
    }
    else
    {
      qCCritical(KSIRK_LOG) << "No sprite on defending country" << defendingCountry->name();
      return;
    }

    if (defenderSprite==0)
    {
      qCCritical(KSIRK_LOG) << "ERROR: null defenderSprite at " << __FILE__ << ", line " << __LINE__;
      return;
    }
    else
    {
      if (defendingCountry->owner()->getNbDefense() == 1)
      {
        defenderSprite->addDecoration("mark1", QRect(0,0,10,10));
      }
      else if (defendingCountry->owner()->getNbDefense() == 2)
      {
        defenderSprite->addDecoration("mark2", QRect(0,0,10,10));
      }

      defenderSprite-> setDefendant();
      defenderSprite-> setupTravel(defendingCountry, defendingCountry, &pointArriveeDefenseur);
      //(pointDepartDefenseurX <= pointArriveeDefenseurX) ? defenderSprite-> setLookRight() : defenderSprite-> setLookLeft();
      qCDebug(KSIRK_LOG) << "add a sprite 3";
      m_animFighters->addSprite(defenderSprite);
      sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roll.wav");
      if (sndRoulePath.isNull())
      {
        KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br>Program cannot continue"), i18n("Error!"));
        exit(2);
      }
      if (KsirkSettings::soundEnabled())
      {
        m_audioPlayer->setCurrentSource(sndRoulePath);
        m_audioPlayer->play();
      }
    }
  }
  qCDebug(KSIRK_LOG) << "Done";
}

void KGameWindow::animCombat()
{
  qCDebug(KSIRK_LOG);
  m_animFighters->changeTarget(this, SLOT(slotFiringFinished(AnimSpritesGroup*)));

  AnimSpritesGroup::iterator it, it_end;
  it = m_animFighters->begin(); it_end = m_animFighters->end();
  for (; it != it_end; it++)
  {
    qCDebug(KSIRK_LOG) << "a sprite position: " << (*it)->pos();
    CannonSprite* sprite = (CannonSprite*)(*it);
    sprite-> changeSequence("firing");

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

    QString sndCanonPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/cannon.wav");
    if (sndCanonPath.isNull())
    {
      KMessageBox::error(0,
          i18n("Sound cannon not found - Verify your installation<br>Program cannot continue"), i18n("Error!"));
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
//   qCDebug(KSIRK_LOG);
  AnimSpritesGroup::iterator it = m_animFighters->begin();
  while (it != m_animFighters->end())
  {
    CannonSprite* sprite = (CannonSprite*)(*it);
//     it = m_animFighters.remove(it);
    sprite-> changeSequence("cannon");

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


void KGameWindow::animExplosion(int who)
{
  qCDebug(KSIRK_LOG) << who;
  Country *attackingCountry = firstCountry();
  
  m_animFighters->changeTarget(this, SLOT(slotExplosionFinished(AnimSpritesGroup*)));

  qCDebug(KSIRK_LOG) << m_animFighters->size() << " fighters";
  unsigned int toArrive = 0;
  AnimSpritesGroup::iterator it = m_animFighters->begin();
  for (;it != m_animFighters->end();it++)
  {
    AnimSprite* sprite = (AnimSprite*)(*it);
    if ( (who == 2) // both are killed
        || ((who == 0) && (sprite-> isAttacker()))  // Attacker is killed
        || ((who == 1) && (sprite-> isDefendant())) ) // Defender is killed
    {
      sprite-> changeSequence("exploding");

      qreal firingRelativePos = (Sprites::SkinSpritesData::single().intData("firing-width") - Sprites::SkinSpritesData::single().intData("exploding-width"))*m_theWorld->zoom();
      if (sprite-> looksToLeft())
      {
        sprite-> setPos(
          sprite-> x() + firingRelativePos,
          sprite-> y() );
      }

      if (sprite->isAttacker())
      {
        qCDebug(KSIRK_LOG) << "  removing a sprite";
        //qCDebug(KSIRK_LOG) << "i attack" << i;
        qCDebug(KSIRK_LOG) << "NKA" << NKA;

        sprite->setAnimated(NKA);

        QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
        if (sndCrashPath.isNull())
        {
          KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
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
        qCDebug(KSIRK_LOG) << "  removing a sprite";
        qCDebug(KSIRK_LOG) << "NKD" << NKD;

        sprite->setAnimated(NKD);

        QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
        if (sndCrashPath.isNull())
        {
          KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
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
      qCDebug(KSIRK_LOG) << "  keeping a sprite";
      
      if ((attackingCountry->nbArmies() % 10) == 0)
      {
        sprite-> changeSequence("cannon");
      }
      else if ((attackingCountry->nbArmies() % 5) == 0)
      {
        sprite-> changeSequence("cavalry");
      }
      else
      {
        sprite-> changeSequence("infantry");
      }

      sprite->setStatic();
      toArrive++;
    }
  }
  for (unsigned int i=0; i < toArrive; i++)
  {
    m_animFighters->oneArrived(0);
  }
  qCDebug(KSIRK_LOG) << "loop done";

  /*QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
  if (sndCrashPath.isNull())
  {
    KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
    exit(2);
  }
  if (KsirkSettings::soundEnabled())
  {
    m_audioPlayer->setCurrentSource(sndCrashPath);
    m_audioPlayer->play();
  }*/
  qCDebug(KSIRK_LOG) << "finished";
//   m_frame-> initTimer();
//    qCDebug(KSIRK_LOG)<<"OUT";
}


void KGameWindow::animExplosionForArena()
{
  qCDebug(KSIRK_LOG);

  Country *attackingCountry = firstCountry();
  Country *defendingCountry = secondCountry();

  m_animFighters->changeTarget(this, SLOT(slotExplosionFinished(AnimSpritesGroup*)));

  qCDebug(KSIRK_LOG) << "hidden; " << m_animFighters->size() << " fighters";
  AnimSpritesGroup::iterator it = m_animFighters->begin();
  
  int nbAttackerSurvivor=attackingCountry->owner()->getNbAttack()-NKA;
  int nbDefenderSurvivor=defendingCountry->owner()->getNbDefense()-NKD;

  qCDebug(KSIRK_LOG) << "nbAttackerSurvivor" << nbAttackerSurvivor;
  qCDebug(KSIRK_LOG) << "nbDefenderSurvivor" << nbDefenderSurvivor;
  qCDebug(KSIRK_LOG) << "nbSpriteAttacking" << nbSpriteAttacking;
  qCDebug(KSIRK_LOG) << "nbSpriteDefending" << nbSpriteDefending;

  for (int nbSpriteTreated=0;
      (nbSpriteTreated < nbSpriteAttacking)&&(it!=m_animFighters->end());
      it++,nbSpriteTreated++)
  {
    AnimSprite* sprite = (AnimSprite*)(*it);

    qCDebug(KSIRK_LOG) << "nbSpriteTreated" << nbSpriteTreated;

    if(dynamic_cast<CannonSprite*>(sprite) != 0)
    {
      qCDebug(KSIRK_LOG) << "*******CANNON*******";

      if (NKA>0)
      {
        sprite-> changeSequence("exploding");

        if (sprite->isAttacker())
        {
          qCDebug(KSIRK_LOG) << "  removing a sprite";
          //qCDebug(KSIRK_LOG) << "i attack" << i;
          qCDebug(KSIRK_LOG) << "NKA" << NKA;

          sprite->setAnimated(NKA);

          QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
          if (sndCrashPath.isNull())
          {
            KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
            exit(2);
          }
          if (KsirkSettings::soundEnabled())
          {
            m_audioPlayer->setCurrentSource(sndCrashPath);
            m_audioPlayer->play();
          }
        }
      }
      else
      {
        sprite-> changeSequence("cannon");

        sprite->setStatic();
        m_animFighters->oneArrived(0);
      }
    }
    else if(dynamic_cast<CavalrySprite*>(sprite) != NULL)
    {
      qCDebug(KSIRK_LOG) << "*******CAVALIER*******";

      if (NKA>0)
      {
        sprite-> changeSequence("exploding");

        if (sprite->isAttacker())
        {
          qCDebug(KSIRK_LOG) << "  removing a sprite";
          //qCDebug(KSIRK_LOG) << "i attack" << i;
          qCDebug(KSIRK_LOG) << "NKA" << NKA;

          sprite->setAnimated(NKA);

          QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
          if (sndCrashPath.isNull())
          {
            KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
            exit(2);
          }
          if (KsirkSettings::soundEnabled())
          {
            m_audioPlayer->setCurrentSource(sndCrashPath);
            m_audioPlayer->play();
          }
        }
      }
      else
      {
        sprite-> changeSequence("cavalry");

        sprite->setStatic();
        m_animFighters->oneArrived(0);
      }
    }
    else
    {
      if (nbAttackerSurvivor<=0)
      {
        sprite-> changeSequence("exploding");

        if (sprite->isAttacker())
        {
          qCDebug(KSIRK_LOG) << "  removing a sprite";
          //qCDebug(KSIRK_LOG) << "i attack" << i;
          qCDebug(KSIRK_LOG) << "NKA" << NKA;

          sprite->setAnimated(1);

          QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
          if (sndCrashPath.isNull())
          {
            KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
            exit(2);
          }
          if (KsirkSettings::soundEnabled())
          {
            m_audioPlayer->setCurrentSource(sndCrashPath);
            m_audioPlayer->play();
          }
        }
      }
      else
      {
        qCDebug(KSIRK_LOG) << "  keeping a sprite";
        if (sprite->isAttacker())
        {
          sprite-> changeSequence("infantry");

          nbAttackerSurvivor--;
        }
        sprite->setStatic();
        m_animFighters->oneArrived(0);
      }
    }
  }

  qCDebug(KSIRK_LOG) << "  loop done attack";

  for (int nbSpriteTreated=0;
      (nbSpriteTreated < nbSpriteDefending)&&(it!=m_animFighters->end());
      it++,nbSpriteTreated++)
  {
    qCDebug(KSIRK_LOG) << "nbSpriteTreated" << nbSpriteTreated;

    AnimSprite* sprite = (AnimSprite*)(*it);

    if(dynamic_cast<CannonSprite*>(sprite) != NULL)
    {
      qCDebug(KSIRK_LOG) << "*******CANNON*******";

      if (NKD>0)
      {
        sprite-> changeSequence("exploding");

        if (sprite->isDefendant())
        {
          qCDebug(KSIRK_LOG) << "  removing a sprite";
          //qCDebug(KSIRK_LOG) << "i attack" << i;
          qCDebug(KSIRK_LOG) << "NKD" << NKD;

          sprite->setAnimated(NKD);

          QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
          if (sndCrashPath.isNull())
          {
            KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
            exit(2);
          }
          if (KsirkSettings::soundEnabled())
          {
            m_audioPlayer->setCurrentSource(sndCrashPath);
            m_audioPlayer->play();
          }
        }
      }
      else
      {
        sprite-> changeSequence("cannon");

        sprite->setStatic();
        m_animFighters->oneArrived(0);
      }
    }
    else if(dynamic_cast<CavalrySprite*>(sprite) != NULL)
    {
      qCDebug(KSIRK_LOG) << "*******CAVALIER*******";

      if (NKD>0)
      {
        sprite-> changeSequence("exploding");

        if (sprite->isDefendant())
        {
          qCDebug(KSIRK_LOG) << "  removing a sprite";
          //qCDebug(KSIRK_LOG) << "i attack" << i;
          qCDebug(KSIRK_LOG) << "NKD" << NKD;

          sprite->setAnimated(NKD);

          QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
          if (sndCrashPath.isNull())
          {
            KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
            exit(2);
          }
          if (KsirkSettings::soundEnabled())
          {
            m_audioPlayer->setCurrentSource(sndCrashPath);
            m_audioPlayer->play();
          }
        }
      }
      else
      {
        sprite-> changeSequence("cavalry");

        sprite->setStatic();
        m_animFighters->oneArrived(0);
      }
    }
    else
    {
      if (nbDefenderSurvivor<=0)
      {
        sprite-> changeSequence("exploding");

        if (sprite->isDefendant())
        {
          qCDebug(KSIRK_LOG) << "  removing a sprite";
          qCDebug(KSIRK_LOG) << "NKD" << NKD;

          sprite->setAnimated(1);

          QString sndCrashPath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/crash.wav");
          if (sndCrashPath.isNull())
          {
            KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error!"));
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
        qCDebug(KSIRK_LOG) << "  keeping a sprite";

        if (sprite->isDefendant())
        {
          sprite-> changeSequence("infantry");

          nbDefenderSurvivor--;
        }
        sprite->setStatic();
        m_animFighters->oneArrived(0);
      }
    }
  }

  qCDebug(KSIRK_LOG) << "  loop done defense";

  qCDebug(KSIRK_LOG) << "finished";
}

void KGameWindow::stopExplosion()
{
    qCDebug(KSIRK_LOG);
    m_animFighters->hideAndRemoveAll();
}

void KGameWindow::initCombatBringBackForArena(Country *attackingCountry, Country *defendingCountry)
{
  qCDebug(KSIRK_LOG);
  int who = 0;
  if ((NKD != 0)&&(NKA != 0)) who = 2;
  else if (NKA != 0) who = 0;
  else if (NKD != 0) who = 1;
  else KMessageBox::information(0, i18n("Problem: no one destroyed"));

  //CannonSprite *newSprite;
  AnimSprite* newSprite;

  qreal leftRelativePos;
  qreal pointDepartAttaquantY;
  qreal pointDepartDefenseurY;

  qreal flagYDiff = (Sprites::SkinSpritesData::single().intData("flag-height") - Sprites::SkinSpritesData::single().intData("cannon-height"))*m_theWorld->zoom();
  qreal pointFlagX = backGnd()->boundingRect().width()/2;

  if ((attackingCountry->nbArmies() % 10) == 0)
  {
      pointDepartAttaquantY = attackingCountry-> pointCannon().y()*      m_theWorld->zoom();
  }
  else
  {
    if ((attackingCountry->nbArmies() % 5) == 0)
    {
        pointDepartAttaquantY = attackingCountry-> pointCavalry().y()*      m_theWorld->zoom();
    }
    else
    {
        pointDepartAttaquantY = attackingCountry-> pointInfantry().y()*      m_theWorld->zoom();
    }
  }

  if ((defendingCountry->nbArmies() % 10) == 0)
  {
      pointDepartDefenseurY = defendingCountry-> pointCannon().y()*      m_theWorld->zoom();
  }
  else
  {
    if ((defendingCountry->nbArmies() % 5) == 0)
    {
        pointDepartDefenseurY = defendingCountry-> pointCavalry().y()*      m_theWorld->zoom();
    }
    else
    {
        pointDepartDefenseurY = defendingCountry-> pointInfantry().y()*      m_theWorld->zoom();
    }
  }

  qreal pointArriveeY = (pointDepartAttaquantY+pointDepartDefenseurY)/2;

  QPointF start(pointFlagX,pointArriveeY);

  qreal rightRelativePos = (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("flag-width"))*m_theWorld->zoom();

  if (who == 0) //Attaquant detruit, ramene defenseur
  {
    if ((defendingCountry->nbArmies() % 10) == 0)
    {
      newSprite = new CannonSprite(m_theWorld->zoom(), backGnd(), 200);

      leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cannon-width"))*m_theWorld->zoom();
    }
    else
    {
      if ((defendingCountry->nbArmies() % 5) == 0)
      {
        newSprite = new CavalrySprite(m_theWorld->zoom(), backGnd(), 200);

        leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cavalry-width"))*m_theWorld->zoom();
      }
      else
      {
        newSprite = new InfantrySprite(m_theWorld->zoom(), backGnd(), 200);

        leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("infantry-width"))*m_theWorld->zoom();
      }
    }

    if (backGnd()->bgIsArena() || ((attackingCountry-> pointFlag().x() <= defendingCountry-> pointFlag().x()) && !((qAbs(attackingCountry-> pointFlag().x()-defendingCountry-> pointFlag().x()) > (backGnd()-> boundingRect().width() / 2)) && (attackingCountry->communicateWith(defendingCountry)))))
    {
        newSprite-> setPos((start)+QPointF(rightRelativePos, flagYDiff));
    }
    else
    {
        newSprite-> setPos((start)+QPointF(leftRelativePos, flagYDiff));
    }

    connect(newSprite,SIGNAL(atDestination(AnimSprite*)),this,SLOT(slotBring(AnimSprite*)));

    QPointF dest;
    //int nbPos;
    if ((defendingCountry->nbArmies() % 10) == 0)
    {
      //nbPos = (defendingCountry->nbArmies() / 10);
      //dest = QPointF(defendingCountry-> pointCannon().x()*m_theWorld->zoom(),(defendingCountry-> pointCannon().y()+(1-2*(nbPos%2))*(Sprites::SkinSpritesData::single().intData("cannon-height")+8)*((nbPos+1)/2))*m_theWorld->zoom());
      dest = QPointF(defendingCountry-> pointCannon().x()*m_theWorld->zoom(),(defendingCountry-> pointCannon().y()+(1-2*(0%2))*(Sprites::SkinSpritesData::single().intData("cannon-height")+8)*((0+1)/2))*m_theWorld->zoom());
    }
    else
    {
      if ((defendingCountry->nbArmies() % 5) == 0)
      {
        //nbPos = (defendingCountry->nbArmies() / 5);
        //dest = QPointF(defendingCountry-> pointCavalry().x()*m_theWorld->zoom(),(defendingCountry-> pointCavalry().y()+(1-2*(nbPos%2))*(Sprites::SkinSpritesData::single().intData("cavalry-height")+8)*((nbPos+1)/2))*m_theWorld->zoom());
        dest = QPointF(defendingCountry-> pointCavalry().x()*m_theWorld->zoom(),(defendingCountry-> pointCavalry().y()+(1-2*(0%2))*(Sprites::SkinSpritesData::single().intData("cavalry-height")+8)*((0+1)/2))*m_theWorld->zoom());
      }
      else
      {
        //nbPos = (defendingCountry->nbArmies()) % 5;

        //dest = QPointF(defendingCountry-> pointInfantry().x()*m_theWorld->zoom(),(defendingCountry-> pointInfantry().y()+(1-2*(nbPos%2))*(Sprites::SkinSpritesData::single().intData("infantry-height")+8)*((nbPos+1)/2))*m_theWorld->zoom());
        dest = QPointF(defendingCountry-> pointInfantry().x()*m_theWorld->zoom(),(defendingCountry-> pointInfantry().y()+(1-2*(0%2))*(Sprites::SkinSpritesData::single().intData("infantry-height")+8)*((0+1)/2))*m_theWorld->zoom());
      }
    }

    ((AnimSprite*)newSprite)-> setupTravel(attackingCountry, defendingCountry, newSprite-> pos(), dest);

    newSprite-> turnTowardDestination();
    qCDebug(KSIRK_LOG) << "add a sprite 4";
    m_animFighters->addSprite(newSprite);

    QString sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roll.wav");
    if (sndRoulePath.isNull())
    {
        KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br>Program cannot continue"), i18n("Error!"));
        exit(2);
    }
    if (KsirkSettings::soundEnabled())
    {
            m_audioPlayer->setCurrentSource(sndRoulePath);
            m_audioPlayer->play();
    }
  }
  else if (who == 1) //Defenseur detruit, ramene Attaquant
  {
    if ((attackingCountry->nbArmies() % 10) == 0)
    {
      newSprite = new CannonSprite(m_theWorld->zoom(), backGnd(), 200);

      leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cannon-width"))*m_theWorld->zoom();
    }
    else
    {
      if ((attackingCountry->nbArmies() % 5) == 0)
      {
        newSprite = new CavalrySprite(m_theWorld->zoom(), backGnd(), 200);

        leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cavalry-width"))*m_theWorld->zoom();
      }
      else
      {
        newSprite = new InfantrySprite(m_theWorld->zoom(), backGnd(), 200);

        leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("infantry-width"))*m_theWorld->zoom();
      }
    }

      if (backGnd()->bgIsArena() || ((attackingCountry-> pointFlag().x() <= defendingCountry-> pointFlag().x()) && !((qAbs(attackingCountry-> pointFlag().x()-defendingCountry-> pointFlag().x()) > (backGnd()-> boundingRect().width() / 2)) && (attackingCountry->communicateWith(defendingCountry)))))
      {
          newSprite-> setPos((start)+QPointF(leftRelativePos,flagYDiff));
      }
      else
      {
          newSprite-> setPos((start)+QPointF(rightRelativePos,flagYDiff));
      }

      connect(newSprite,SIGNAL(atDestination(AnimSprite*)),this,SLOT(slotBring(AnimSprite*)));

    QPointF dest;
    //int nbPos;
    if ((attackingCountry->nbArmies() % 10) == 0)
    {
      //nbPos = (attackingCountry->nbArmies() / 10);
      //dest = QPointF(attackingCountry-> pointCannon().x()*m_theWorld->zoom(),(attackingCountry-> pointCannon().y()+(1-2*(nbPos%2))*(Sprites::SkinSpritesData::single().intData("cannon-height")+8)*((nbPos+1)/2))*m_theWorld->zoom());
      dest = QPointF(attackingCountry-> pointCannon().x()*m_theWorld->zoom(),(attackingCountry-> pointCannon().y()+(1-2*(0%2))*(Sprites::SkinSpritesData::single().intData("cannon-height")+8)*((0+1)/2))*m_theWorld->zoom());
    }
    else
    {
      if ((attackingCountry->nbArmies() % 5) == 0)
      {
        //nbPos = (attackingCountry->nbArmies() / 5);
        //dest = QPointF(attackingCountry-> pointCavalry().x()*m_theWorld->zoom(),(attackingCountry-> pointCavalry().y()+(1-2*(nbPos%2))*(Sprites::SkinSpritesData::single().intData("cavalry-height")+8)*((nbPos+1)/2))*m_theWorld->zoom());
        dest = QPointF(attackingCountry-> pointCavalry().x()*m_theWorld->zoom(),(attackingCountry-> pointCavalry().y()+(1-2*(0%2))*(Sprites::SkinSpritesData::single().intData("cavalry-height")+8)*((0+1)/2))*m_theWorld->zoom());
      }
      else
      {
        //nbPos = attackingCountry->nbArmies() % 5;
        //dest = QPointF(attackingCountry-> pointInfantry().x()*m_theWorld->zoom(),(attackingCountry-> pointInfantry().y()+(1-2*(nbPos%2))*(Sprites::SkinSpritesData::single().intData("infantry-height")+8)*((nbPos+1)/2))*m_theWorld->zoom());
        dest = QPointF(attackingCountry-> pointInfantry().x()*m_theWorld->zoom(),(attackingCountry-> pointInfantry().y()+(1-2*(0%2))*(Sprites::SkinSpritesData::single().intData("infantry-height")+8)*((0+1)/2))*m_theWorld->zoom());
      }
    }

      ((AnimSprite*)newSprite)-> setupTravel(defendingCountry, attackingCountry, newSprite-> pos(), dest);

      newSprite-> turnTowardDestination();
      qCDebug(KSIRK_LOG) << "add a sprite 5";
      m_animFighters->addSprite(newSprite);

      QString sndRoulePath = m_dirs-> findResource("appdata", m_automaton->skin() + "/Sounds/roll.wav");
      if (sndRoulePath.isNull())
      {
          KMessageBox::error(0, i18n("Sound roule not found - Verify your installation<br>Program cannot continue"), i18n("Error!"));
          exit(2);
      }
      if (KsirkSettings::soundEnabled())
      {
              m_audioPlayer->setCurrentSource(sndRoulePath);
              m_audioPlayer->play();
      }
  }
  else if (who == 2)
  {
  } //Attaquant ET Defenseur detruits
  else  // error
  {
    qCCritical(KSIRK_LOG) << Q_FUNC_INFO << __FILE__ << __LINE__ << i18n("Bug: who should be 0, 1 or 2.");
    exit(1);
  }

  relativePosInArenaDefense=0;
  relativePosInArenaAttack=0;
}

/**
  * Disconnects the mouse events signals from their slots to avoid human
  * player actions when it is the turn of the AI
  */
void KGameWindow::disconnectMouse()
{
/*  if ( ! disconnect(m_frame, SIGNAL(signalLeftButtonDown(QPoint)),
                    this, SLOT(slotLeftButtonDown(QPoint))))
    qCCritical(KSIRK_LOG) << "cannot connect slotLeftButtonDown !";
  if ( ! disconnect(m_frame, SIGNAL(signalLeftButtonUp(QPoint)),
                    this, SLOT(slotLeftButtonUp(QPoint))))
    qCCritical(KSIRK_LOG) << "cannot connect slotLeftButtonUp !";
  if ( ! disconnect(m_frame, SIGNAL(signalRightButtonDown(QPoint)),
                    this, SLOT(slotRightButtonDown(QPoint))))
    qCCritical(KSIRK_LOG) << "cannot connect slotRightButtonDown !";*/
}

/**
  * Reconnect the mouse events signals to their slots to allow human players to
  * play
  */
void KGameWindow::reconnectMouse()
{
/*  if ( ! connect(m_frame, SIGNAL(signalLeftButtonDown(QPoint)),
                    this, SLOT(slotLeftButtonDown(QPoint))))
  qCCritical(KSIRK_LOG) << "cannot connect slotLeftButtonDown !";
  if ( ! connect(m_frame, SIGNAL(signalLeftButtonUp(QPoint)),
                    this, SLOT(slotLeftButtonUp(QPoint))))
  qCCritical(KSIRK_LOG) << "cannot connect slotLeftButtonUp !";
  if ( ! connect(m_frame, SIGNAL(signalRightButtonDown(QPoint)),
                    this, SLOT(slotRightButtonDown(QPoint))))
  qCCritical(KSIRK_LOG) << "cannot connect slotRightButtonDown !";*/
}

bool KGameWindow::haveAnimFighters() const
{
    return !m_animFighters->empty();
}

void KGameWindow::updateScrollArrows()
{
  qCDebug(KSIRK_LOG);
  if (m_uparrow != 0)
  {
    QPointF pos = m_frame->mapToScene(QPoint(m_frame->viewport()->width()/2,0));
    pos = pos + QPointF(-(m_uparrow->boundingRect().width()/2),m_uparrow->boundingRect().height());
    m_uparrow->setPos(pos);
    m_uparrow->setActive(false);
  }
  if (m_downarrow != 0)
  {
    QPointF pos = m_frame->mapToScene(QPoint(m_frame->viewport()->width()/2,m_frame->viewport()->height()));
    pos = pos - QPointF(m_downarrow->boundingRect().width()/2,m_downarrow->boundingRect().height());
    m_downarrow->setPos(pos);
    m_downarrow->setActive(false);
  }
  if (m_leftarrow != 0)
  {
    QPointF pos = m_frame->mapToScene(QPoint(0,m_frame->viewport()->height()/2));
    pos = pos - QPointF(m_downarrow->boundingRect().width()/2,m_downarrow->boundingRect().height());
    pos = pos + QPointF(m_leftarrow->boundingRect().width(),-(m_leftarrow->boundingRect().height()/2));
    m_leftarrow->setPos(pos);
    m_leftarrow->setActive(false);
  }
  if (m_rightarrow != 0)
  {
    QPointF pos = m_frame->mapToScene(QPoint(m_frame->viewport()->width(),m_frame->viewport()->height()/2));
    pos = pos - QPointF(m_rightarrow->boundingRect().width(),m_rightarrow->boundingRect().height()/2);
    m_rightarrow->hide();
    m_rightarrow->setPos(pos);
    m_rightarrow->show();
    m_rightarrow->setActive(false);
  }
}

} // closing namespace KsirK
