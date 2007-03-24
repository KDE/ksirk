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

// include files for QT
#include <qmetaobject.h>
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qmessagebox.h>
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
#include <phonon/audioplayer.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <KToolBar>
 
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

namespace Ksirk
{
using namespace GameLogic;

/**
 * Prepares the sprites to be moved : removes the nb necessary sprites from
 * source, creates the moving sprites and gives them their destination, etc
 */
bool KGameWindow::initArmiesMovement(unsigned int nbABouger, Country *m_firstCountry, Country *m_secondCountry)
{
  kDebug() << "KGameWindow::initArmiesMovement -> " << nbABouger  << endl;
  KMessageParts messageParts;

  if (m_firstCountry-> nbArmies() <= nbABouger)
  {
    messageParts << I18N_NOOP("Cannot move %1 armies from %2 to %3") << QString::number(nbABouger)
      << m_firstCountry->name() << m_secondCountry->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return false;
  }
  else
  {
    messageParts << I18N_NOOP("Moving %1 armies from %2 to %3") << QString::number(nbABouger) << m_firstCountry->name() << m_secondCountry->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2);
  }

  AnimSpritesGroup* newGroup = new AnimSpritesGroup(this,SLOT(slotMovingArmiesArrived(AnimSpritesGroup*)));
  m_animSpritesGroups.push_back(newGroup);
  AnimSprite* sprite;
  if ((m_firstCountry-> nbArmies() > 10) && (nbABouger == 10)
          && (nbABouger < m_firstCountry-> nbArmies()))
  {
    sprite = new CannonSprite( 
        Sprites::SkinSpritesData::single().strData("cannon-file"), m_backGnd,
        Sprites::SkinSpritesData::single().intData("cannon-frames"),
        Sprites::SkinSpritesData::single().intData("cannon-versions"), 200);
    m_firstCountry-> decrNbArmies(10);
  }
  else if ((m_firstCountry-> nbArmies() > 5) && (nbABouger == 5)
          && (nbABouger < m_firstCountry-> nbArmies()))
  {
    sprite = new CavalrySprite( Sprites::
        SkinSpritesData::single().strData("cavalry-file"), m_backGnd, 
        Sprites::SkinSpritesData::single().intData("cavalry-frames"), 
        Sprites::SkinSpritesData::single().intData("cavalry-versions"), 200);
    m_firstCountry-> decrNbArmies(5);
  }
  else if ((m_firstCountry-> nbArmies() > 1) && (nbABouger == 1)
          && (nbABouger < m_firstCountry-> nbArmies()))
  {
    sprite = new InfantrySprite( 
        Sprites::SkinSpritesData::single().strData("infantry-file"), m_backGnd, 
        Sprites::SkinSpritesData::single().intData("infantry-frames"), 
        Sprites::SkinSpritesData::single().intData("infantry-versions"), 200);
    m_firstCountry->  decrNbArmies();
  }
  else 
  {
    messageParts << I18N_NOOP("Cannot move %1 armies from %2 to %3") 
      << QString::number(nbABouger) 
      << m_firstCountry->name() 
      << m_secondCountry->name();
    broadcastChangeItem(messageParts, ID_STATUS_MSG2, false);
    return false;
  }
  connect(sprite,SIGNAL(atDestination(AnimSprite*)),this,SLOT(slotMovingArmyArrived(AnimSprite*)));
  sprite-> setupTravel(m_firstCountry, m_secondCountry);
//   newGroup->addSprite(sprite);
  newGroup->addSprite(sprite);
  m_firstCountry-> createArmiesSprites(this-> m_backGnd);
  sprite->setAnimated();
//   kDebug() << "initArmiesMovement returns true" << endl;
  return true;
}

void KGameWindow::initCombatMovement(Country *paysAttaquant, Country *paysDefenseur)
{
//   kDebug() << "KGameWindow::initCombatMovement" << endl;

  gameActionsToolBar-> hide();
  
  m_animFighters->clear();
  m_animFighters->changeTarget(this,SLOT(slotMovingFightersArrived(AnimSpritesGroup*)));
  
  // On doit connaitre
  //  - point de depart de l'attaquant (pointCanonn)
  qreal pointDepartAttaquantX = paysAttaquant-> pointCannon().x();
  //  - point de depart du defenseur (pointCannon)
  qreal pointDepartDefenseurX = paysDefenseur-> pointCannon().x();
  
  //  - point drapeau de l'attaquant
  qreal pointFlagAttaquantX = paysAttaquant-> pointFlag().x();
  //  - point drapeau du defenseur
  qreal pointFlagDefenseurX = paysDefenseur-> pointFlag().x();
  
  //  - point d'arrivee de l'attaquant (resp. du defenseur) (gauche ou droite de point drapeau defenseur
  //    selon position point drapeau attaquant)
  qreal pointArriveeAttaquantX;
  qreal pointArriveeAttaquantY;
  qreal pointArriveeDefenseurX;
  qreal pointArriveeDefenseurY;
  
  pointArriveeAttaquantY = pointArriveeDefenseurY = (paysDefenseur-> pointFlag().y() + Sprites::SkinSpritesData::single().intData("fighters-flag-y-diff"));
  
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

  int leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cannon-width"));
  int rightRelativePos = Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("flag-width");
  if (!((qAbs(pointFlagAttaquantX-pointFlagDefenseurX) > (m_backGnd-> boundingRect().width() / 2))))
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
  QPointF *pointArriveeAttaquant = new QPointF(pointArriveeAttaquantX,pointArriveeAttaquantY);
  QPointF *pointArriveeDefenseur = new QPointF(pointArriveeDefenseurX,pointArriveeDefenseurY);
  
  CannonSprite* attackingSprite = new CannonSprite( Sprites::SkinSpritesData::single().strData("cannon-file"), m_backGnd, Sprites::SkinSpritesData::single().intData("cannon-frames"), Sprites::SkinSpritesData::single().intData("cannon-versions"), 200);
  attackingSprite-> setAttacker();
  attackingSprite->setupTravel(paysAttaquant, paysDefenseur, pointArriveeAttaquant);
  (pointDepartAttaquantX <= pointArriveeAttaquantX) ? attackingSprite-> setLookRight() : attackingSprite-> setLookLeft();
  m_animFighters->addSprite(attackingSprite);

  QString sndRoulePath = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Sounds/roule.wav");
  if (sndRoulePath.isNull())
  {
      QMessageBox::critical(0, i18n("Error !"), i18n("Sound roule not found - Verify your installation\nProgram cannot continue"));
      exit(2);
  }
  m_audioPlayer->play(sndRoulePath);
  
  CannonSprite* defenderSprite = new CannonSprite( Sprites::SkinSpritesData::single().strData("cannon-file"), m_backGnd, Sprites::SkinSpritesData::single().intData("cannon-frames"), Sprites::SkinSpritesData::single().intData("cannon-versions"), 200);
  defenderSprite-> setDefendant();
  defenderSprite-> setupTravel(paysDefenseur, paysDefenseur, pointArriveeDefenseur);
  (pointDepartDefenseurX <= pointArriveeDefenseurX) ? defenderSprite-> setLookRight() : defenderSprite-> setLookLeft();
  m_animFighters->addSprite(defenderSprite);

  sndRoulePath = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Sounds/roule.wav");
  if (sndRoulePath.isNull())
  {
      QMessageBox::critical(0, i18n("Error !"), i18n("Sound roule not found - Verify your installation\nProgram cannot continue"));
      exit(2);
  }
  m_audioPlayer->play(sndRoulePath);
}

void KGameWindow::animCombat()
{
  kDebug()<<"KGameWindow::animCombat"<< endl;
  m_animFighters->changeTarget(this, SLOT(slotFiringFinished(AnimSpritesGroup*)));

  AnimSpritesGroup::iterator it, it_end;
  it = m_animFighters->begin(); it_end = m_animFighters->end();
  for (; it != it_end; it++)
  {
    CannonSprite* sprite = (CannonSprite*)(*it);
    sprite-> changeSequence(
        Sprites::SkinSpritesData::single().strData("firing-file"),
        Sprites::SkinSpritesData::single().intData("firing-frames"), 
        Sprites::SkinSpritesData::single().intData("firing-versions"));
    int firingRelativePos = Sprites::SkinSpritesData::single().intData("cannon-width") - Sprites::SkinSpritesData::single().intData("firing-width");
    if (sprite-> looksToLeft()) 
    {
      sprite-> setPos(sprite-> x() + firingRelativePos,sprite-> y() - Sprites::SkinSpritesData::single().intData("fighters-flag-y-diff"));
    }
    sprite->setAnimated(1);

    QString sndCanonPath = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Sounds/canon.wav");
    if (sndCanonPath.isNull())
    {
      QMessageBox::critical(0, i18n("Error !"),
          i18n("Sound canon not found - Verify your installation\nProgram cannot continue"));
      exit(2);
    }
    if (KsirkSettings::soundEnabled())
    {
      m_audioPlayer->play(sndCanonPath);
    }
  }
}

void KGameWindow::stopCombat()
{
//   kDebug()<<"KGameWindow::stopCombat"<< endl;
  AnimSpritesGroup::iterator it = m_animFighters->begin();
  while (it != m_animFighters->end())
  {
    CannonSprite* sprite = (CannonSprite*)(*it);
//     it = m_animFighters.remove(it);
    sprite-> changeSequence(
        Sprites::SkinSpritesData::single().strData("cannon-file"),
        Sprites::SkinSpritesData::single().intData("cannon-frames"), 
        Sprites::SkinSpritesData::single().intData("cannon-versions"));
    if (sprite-> looksToLeft()) 
    {
      sprite-> setPos(sprite->x() + Sprites::SkinSpritesData::single().intData("cannon-width"),sprite-> y() + Sprites::SkinSpritesData::single().intData("fighters-flag-y-diff"));
    }
    else 
    {
//       sprite-> setX(sprite-> x());
    }
  }
}

void KGameWindow::animExplosion(int who)
{
  kDebug() << "KGameWindow::animExplosion " << who << endl;

  m_animFighters->changeTarget(this, SLOT(slotExplosionFinished(AnimSpritesGroup*)));

  kDebug() << "KGameWindow::animExplosion hidden; " << m_animFighters->size() << " fighters" << endl;
  AnimSpritesGroup::iterator it = m_animFighters->begin();
  for (;it != m_animFighters->end();it++)
  {
    CannonSprite* sprite = (CannonSprite*)(*it);
    if ( (who == 2) // both are killed
        || ((who == 0) && (sprite-> isAttacker()))  // Attacker is killed
        || ((who == 1) && (sprite-> isDefendant())) ) // Defender is killed
    {
      kDebug() << "  removing a sprite" << endl;
      sprite-> changeSequence(
          Sprites::SkinSpritesData::single().strData("exploding-file"),
          Sprites::SkinSpritesData::single().intData("exploding-frames"), 
          Sprites::SkinSpritesData::single().intData("exploding-versions"));
      sprite->setAnimated(1);
    }
    else  // the sprite is not the one (or one the several) killed
    {
      kDebug() << "  keeping a sprite" << endl;
      sprite-> changeSequence(
          Sprites::SkinSpritesData::single().strData("cannon-file"),
          Sprites::SkinSpritesData::single().intData("cannon-frames"), 
          Sprites::SkinSpritesData::single().intData("cannon-versions"));
      sprite->setStatic();
      m_animFighters->oneArrived(0);
    }
  }
  kDebug() << "  loop done" << endl;

  QString sndCrashPath = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Sounds/crash.wav");
  if (sndCrashPath.isNull())
  {
    KMessageBox::information(this, i18n("Sound crash not found - Verify your installation\nProgram cannot continue"), i18n("KsirK - Error !"));
    exit(2);
  }
  if (KsirkSettings::soundEnabled())
  {
    m_audioPlayer->play(sndCrashPath);
  }
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

    int flagYDiff = Sprites::SkinSpritesData::single().intData("fighters-flag-y-diff");
    int leftRelativePos = - (Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("cannon-width"));
    int rightRelativePos = Sprites::SkinSpritesData::single().intData("width-between-flag-and-fighter") + Sprites::SkinSpritesData::single().intData("flag-width");
    if (who == 0) //Attaquant detruit, ramene defenseur
    {
        newSprite = new CannonSprite( Sprites::SkinSpritesData::single().strData("cannon-file"), m_backGnd, Sprites::SkinSpritesData::single().intData("cannon-frames"), Sprites::SkinSpritesData::single().intData("cannon-versions"), 200);
        if ((paysAttaquant-> pointFlag().x() <= paysDefenseur-> pointFlag().x())
        && !(
        (qAbs(paysAttaquant-> pointFlag().x()-paysDefenseur-> pointFlag().x()) > (m_backGnd-> boundingRect().width() / 2))
            && (paysAttaquant->communicateWith(paysDefenseur))
        ))
        {
            newSprite-> setPos(  (paysDefenseur-> pointFlag())+QPoint(rightRelativePos, flagYDiff));
        }
        else
        {
            newSprite-> setPos(  (paysDefenseur-> pointFlag())+QPoint(leftRelativePos, flagYDiff));
        }
        ((AnimSprite*)newSprite)-> setupTravel(paysDefenseur, paysDefenseur, newSprite-> pos(), paysDefenseur-> pointCannon());
        newSprite-> turnTowardDestination();

        QString sndRoulePath = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Sounds/roule.wav");
        if (sndRoulePath.isNull())
        {
            QMessageBox::critical(0, i18n("Error !"), i18n("Sound roule not found - Verify your installation\nProgram cannot continue"));
            exit(2);
        }
        m_audioPlayer->play(sndRoulePath);
    }
    else if (who == 1) //Defenseur detruit, ramene Attaquant
    {
        newSprite = new CannonSprite( Sprites::SkinSpritesData::single().strData("cannon-file"), m_backGnd, Sprites::SkinSpritesData::single().intData("cannon-frames"), Sprites::SkinSpritesData::single().intData("cannon-versions"), 200);
        if ((paysAttaquant-> pointFlag().x() <= paysDefenseur-> pointFlag().x())
        && !(
        (qAbs(paysAttaquant-> pointFlag().x()-paysDefenseur-> pointFlag().x()) > (m_backGnd-> boundingRect().width() / 2))
            && (paysAttaquant->communicateWith(paysDefenseur))
        ))
        {
            newSprite-> setPos( (paysDefenseur-> pointFlag())+QPoint(leftRelativePos,flagYDiff));
        }
        else
        {
            newSprite-> setPos( (paysDefenseur-> pointFlag())+QPoint(rightRelativePos,flagYDiff));
        }
        ((AnimSprite*)newSprite)-> setupTravel(paysDefenseur, paysAttaquant, newSprite-> pos(), paysAttaquant-> pointCannon());
        newSprite-> turnTowardDestination();

        QString sndRoulePath = m_dirs-> findResource("appdata", GameAutomaton::changeable().skin() + "/Sounds/roule.wav");
        if (sndRoulePath.isNull())
        {
            QMessageBox::critical(0, i18n("Error !"), i18n("Sound roule not found - Verify your installation\nProgram cannot continue"));
            exit(2);
        }
        m_audioPlayer->play(sndRoulePath);
    }
    else if (who == 2) 
    {
    } //Attaquant ET Defenseur detruits
    else  // error
    {
      QMessageBox::critical(0, i18n("Error !"), i18n("who should be 0, 1 or 2."));
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


/**
returns a pointer to the country that is currently attacked
@return the attacked country ; 0 if none
*/
const Country* KGameWindow::getAttackedCountry() const
{
    return m_secondCountry;
}

/**
returns a pointer to the currently attacking  country
@return the attacking country ; 0 if none
*/
const Country* KGameWindow::getAttackingCountry() const
{
    return m_firstCountry;
}

bool KGameWindow::haveAnimFighters() const
{
    return !m_animFighters->empty();
}

} // closing namespace KsirK
