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

/* begin                : Wed Jul 18 2001 */

#define KDE_NO_COMPAT
#include "country.h"
#include "player.h"
#include "Sprites/backgnd.h"
#include "kgamewin.h"
#include "ksirksettings.h"
#include "Sprites/flagsprite.h"
#include "Sprites/skinSpritesData.h"
#include "GameLogic/onu.h"

#include <klocale.h>
#include <kdebug.h>
#include <ksvgrenderer.h>
#include <QString>
#include <QApplication>
#include <QDataStream>
#include <QBitmap>
#include <QGraphicsSvgItem>

#include <iostream>
#include <stdexcept>

namespace Ksirk
{

namespace GameLogic
{

Country::Country(GameAutomaton* game,
                  const QString& theName,
                  const QPointF& anchorPoint,
                  const QPointF& centralPoint,
                  const QPointF& flagPoint,
                  const QPointF& cannonPoint, 
                  const QPointF& cavalryPoint,
                  const QPointF& infantryPoint, 
                  unsigned int id) :
  m_automaton(game),
  m_belongsTo(0),
  m_flag(0),
  m_nbArmies(0),
  m_nbAddedArmies(0),
  m_name(theName),
  m_anchorPoint(anchorPoint),
  m_centralPoint(centralPoint),
  m_pointFlag(flagPoint),
  m_pointCannon(cannonPoint), 
  m_pointCavalry(cavalryPoint),
  m_pointInfantry(infantryPoint),
  m_id(id),
  m_highlighting(0),
  m_renderer(new KSvgRenderer()),
  m_highlighting_locked(false),
  m_nbArmiesItem(0)
{
//   kDebug() << m_name << ", " << this << endl;
}

Country::~Country()
{
//   kDebug() << "Deleting country " << m_name << ", " << this << endl;
  if (m_flag)
  {
    delete m_flag;
  }
  delete m_renderer;
}

void Country::reset()
{
//   kDebug() << "Country::reset " << m_name << endl;
  clearAllSprites();
  m_belongsTo = 0;
  nbArmies(1);
  createArmiesSprites();
  
  if (m_flag)
  {
    m_flag->hide();
    delete m_flag;
    m_flag = 0;
  }
}

void Country::createArmiesSprites()
{
  kDebug();
  BackGnd* bg = m_automaton->game()->backGnd();
  
  unsigned int armies = nbArmies();
  clearAllSprites();
  int i = 0;
  kDebug() << "adding cannons";
  while (armies >= 10) // Ajout des sprites de canon
  {
    CannonSprite *sprite = new CannonSprite(
        "cannon",
        Sprites::SkinSpritesData::single().intData("cannon-width"),
        Sprites::SkinSpritesData::single().intData("cannon-height"),
        Sprites::SkinSpritesData::single().intData("cannon-frames"),
        Sprites::SkinSpritesData::single().intData("cannon-versions"),
        bg->onu()->zoom(),
        bg);
    sprite-> setDestination(NULL);             // Sprite immobile
    if (m_automaton->game()->currentWidgetType() == KGameWindow::arenaType) {
      sprite-> setPos(
        m_pointCannon.x()*bg->onu()->zoom(),
        (m_pointCannon.y()+(1-2*(i%2))*(Sprites::SkinSpritesData::single().intData("cannon-height")+8)*((i+1)/2))*bg->onu()->zoom());
      if (this->id() == m_automaton->game()->secondCountry()->id()) {
        sprite->setLookLeft();
      }
    } else {
      sprite-> setPos((m_pointCannon.x()+5*i)*bg->onu()->zoom(),(m_pointCannon.y()+5*i)*bg->onu()->zoom());
    }
    m_spritesCannons.append(sprite);
    i++;
    armies -= 10;
  }
  i = 0;
  kDebug() << "adding cavalries" << armies;
  while (armies >= 5) // Adding the cavalryman  sprites
  {
    kDebug() << i << armies;
    CavalrySprite *sprite = new CavalrySprite("cavalry",
        Sprites::SkinSpritesData::single().intData("cavalry-width"),
        Sprites::SkinSpritesData::single().intData("cavalry-height"),
        Sprites::SkinSpritesData::single().intData("cavalry-frames"),
        Sprites::SkinSpritesData::single().intData("cavalry-versions"),
        bg->onu()->zoom(), bg);
    sprite-> setDestination(0);             // Sprite immobile
    if (m_automaton->game()->currentWidgetType() == KGameWindow::arenaType)
    {
      kDebug() << "arena";
      sprite-> setPos(
        m_pointCavalry.x()*bg->onu()->zoom(),
        (m_pointCavalry.y()+(1-2*(i%2))*(Sprites::SkinSpritesData::single().intData("cavalry-height")+8)*((i+1)/2))*bg->onu()->zoom());
      kDebug() << "setPos done";
      if (this->id() == m_automaton->game()->secondCountry()->id())
      {
        sprite->setLookLeft();
      }
    }
    else
    {
      sprite-> setPos((m_pointCavalry.x()+5*i)*bg->onu()->zoom(),(m_pointCavalry.y()+5*i)*bg->onu()->zoom());
    }
    kDebug() << "appending";
    m_spritesCavalry.append(sprite);
    i++;
    armies -= 5;
  }
  i = 0;
  kDebug() << "adding infantries";
  while (armies > 0) // Ajout des sprites de fantassin
  {
    InfantrySprite *sprite = new InfantrySprite(
        "infantry",
        Sprites::SkinSpritesData::single().intData("infantry-width"),
        Sprites::SkinSpritesData::single().intData("infantry-height"),
        Sprites::SkinSpritesData::single().intData("infantry-frames"),
        Sprites::SkinSpritesData::single().intData("infantry-versions"),
        bg->onu()->zoom(),
        bg);
    sprite-> setDestination(NULL);             // Sprite immobile
    if (m_automaton->game()->currentWidgetType() == KGameWindow::arenaType) {
      sprite-> setPos(
        m_pointInfantry.x()*bg->onu()->zoom(),
        (m_pointInfantry.y()+(1-2*(i%2))*(Sprites::SkinSpritesData::single().intData("infantry-height")+8)*((i+1)/2))*bg->onu()->zoom());
      if (this->id() == m_automaton->game()->secondCountry()->id()) {
        sprite->setLookLeft();
      }
    } else {
      sprite-> setPos((m_pointInfantry.x()+5*i)*bg->onu()->zoom(),(m_pointInfantry.y()+5*i)*bg->onu()->zoom());
    }
    m_spritesInfantry.append(sprite);
    i++;
    armies--;
  }

  kDebug() << "adding flag";
  if (m_belongsTo)
  {
    flag(m_belongsTo->flagFileName(), bg);
  }

  kDebug() << "adding nb armies text";
  if (m_nbArmiesItem == 0)
  {
    m_nbArmiesItem = new QGraphicsSimpleTextItem(QString::number(m_nbArmies),bg);
    m_nbArmiesItem->setPos((m_pointFlag.x()+5),(m_pointFlag.y()+7));
    m_nbArmiesItem->setZValue(20);
    m_nbArmiesItem->setBrush(Qt::white);
    m_nbArmiesItem->setPen(QPen(Qt::yellow));
  }
  if (!KsirkSettings::showArmiesNumbers())
  {
    m_nbArmiesItem->hide();
  }
  else
  {
    m_nbArmiesItem->show();
  }
  kDebug() << "Done";
}

void Country::flag(const QString& theFlagFileName, BackGnd *backGnd)
{
//   kDebug() << "Country("<<m_name<<", "<<this<<")::flag flagFileName " << theFlagFileName << endl;

  if (m_flag)
  {
    m_flag->hide();
    delete m_flag;
    m_flag = 0;
  }
  
  m_flag = new FlagSprite(theFlagFileName,
                           Sprites::SkinSpritesData::single().intData("flag-width"),
                           Sprites::SkinSpritesData::single().intData("flag-height"),
                           Sprites::SkinSpritesData::single().intData("flag-frames"), 
      Sprites::SkinSpritesData::single().intData("flag-versions"),
                           backGnd->onu()->zoom(),backGnd);
  m_flag-> setDestination(NULL);
  m_flag-> setPos(m_pointFlag.x()*backGnd->onu()->zoom(),m_pointFlag.y()*backGnd->onu()->zoom());
  m_flag-> setZValue(10);

//    qDebug("OUT Country::flag");
}

bool Country::communicateWith(const Country* otherCountry) const
{
  if (!otherCountry)
  {
    kDebug() << "OUT otherCountry null Country::communicateWith" << endl;
    return false;
  }

  // a country is considered to communicate with itself
  if (otherCountry == this) {return true;}

//    kDebug() << "Country::communicateWith (" << name() << ", " << otherCountry-> name() << ")" << endl << flush;
  unsigned int nbNeighbours = neighbours().size();
  for (unsigned int i = 0; i < nbNeighbours; i++)
  {
    if (neighbours().at(i) == otherCountry)
    {
//            kDebug() << "OUT true Country::communicateWith" << endl << flush;
      return true;
    }
  }
//    kDebug() << "OUT false Country::communicateWith" << endl << flush;
  return false;
}

/** @returns a pointer on the Player that holds this Country */
const Player * Country::owner() const
{
  return m_belongsTo;
}

Player * Country::owner() 
{
  return m_belongsTo;
}

/** At the end of this method, the Country belogns to the argument player */
void Country::owner(Player *player)
{  
  m_belongsTo = player;
  if (player != 0)
  {
    createArmiesSprites();
    flag(m_belongsTo->flagFileName(), m_automaton->game()-> backGnd());
  }
}

unsigned int Country::nbArmies() const
{
  return m_nbArmies;
}

unsigned int Country::nbAddedArmies()
{
  return m_nbAddedArmies;
}

void Country::nbArmies(unsigned int nb)
{
  m_nbArmies = nb;
  if (m_nbArmiesItem!=0)
  {
    m_nbArmiesItem->setText(QString::number(m_nbArmies));
  }
}

void Country::nbAddedArmies(unsigned int nb)
{
  m_nbAddedArmies = nb;
}

void Country::incrNbArmies(unsigned int nb)
{
  nbArmies(nbArmies() + nb);
}

void Country::incrNbAddedArmies(unsigned int nb)
{
  nbAddedArmies(nbAddedArmies() + nb);
}

void Country::decrNbAddedArmies(unsigned int nb)
{
  nbAddedArmies(nbAddedArmies() - nb);
}

void Country::decrNbArmies(unsigned int nb)
{
  nbArmies(nbArmies() - nb);
}

const QString Country::name() const
{
  return (m_name);
}

const QPointF& Country::anchorPoint() const
{
  return (m_anchorPoint);
}

const QPointF& Country::centralPoint() const
{
  return (m_centralPoint);
}

const QPointF& Country::pointFlag() const
{
  return (m_pointFlag);
}

const QPointF& Country::pointCannon() const
{
  return (m_pointCannon);
}

const QPointF& Country::pointCavalry() const
{
  return (m_pointCavalry);
}

const QPointF& Country::pointInfantry() const
{
  return (m_pointInfantry);
}

void Country::anchorPoint(const QPointF pt)
{
  m_anchorPoint = pt;
}

void Country::centralPoint(const QPointF pt)
{
  m_centralPoint = pt;
}

void Country::pointFlag(const QPointF pt)
{
  m_pointFlag = pt;
}

void Country::pointCannon(const QPointF pt)
{
  m_pointCannon = pt;
}

void Country::pointCavalry(const QPointF pt)
{
  m_pointCavalry = pt;
}

void Country::pointInfantry(const QPointF pt)
{
  m_pointInfantry = pt;
}

AnimSpritesList< CannonSprite >& Country::spritesCannons()
{
  return (m_spritesCannons);
}

AnimSpritesList< CavalrySprite >& Country::spritesCavalry()
{
  return (m_spritesCavalry);
}

AnimSpritesList< InfantrySprite >& Country::spritesInfantry()
{
  return (m_spritesInfantry);
}

/** No descriptions */
void Country::neighbours(const std::vector<Country*>& neighboursVect)
{
  m_neighbours = neighboursVect;
}

/** No descriptions */
std::vector< Country* >& Country::neighbours()
{
//    kDebug() << "Country::neighbours" << endl << flush;
  return m_neighbours;
}

/** No descriptions */
const std::vector< Country* >& Country::neighbours() const
{
//    kDebug() << "Country::neighbours const" << endl << flush;
  return m_neighbours;
}

/*!
    \fn Ksirk::Country::clearAllSprites()
 */
void Country::clearAllSprites()
{
  kDebug();
  m_spritesCannons.hideAndRemoveAll();
  m_spritesCavalry.hideAndRemoveAll();
  m_spritesInfantry.hideAndRemoveAll();
}

const QPointF& Country::pointFor(const AnimSprite* sprite)
{
  if (dynamic_cast<const InfantrySprite*>(sprite))
  {
    return pointInfantry();
  }
  else if (dynamic_cast<const CavalrySprite*>(sprite))
  {
    return pointCavalry();
  }
  else if (dynamic_cast<const CannonSprite*>(sprite))
  {
    return pointCannon();
  }
  else if (dynamic_cast<const FlagSprite*>(sprite))
  {
    return pointFlag();
  }
  else 
  {
    std::cerr << "Unknown sprite type" << std::endl;
    exit(1);
  }
}

void Country::saveXml(std::ostream& xmlStream)
{
  QString name = m_name.toUtf8();
  name = name.replace("&","&amp;");
  name = name.replace("<","&lt;");
  name = name.replace(">","&gt;");
  xmlStream << "<country name=\""<<name.toUtf8().data()<<"\" owner=\"";
  if (m_belongsTo == 0)
    xmlStream << "none";
  else
  {
    QString pname = m_belongsTo->name().toUtf8();
    pname = pname.replace("&","&amp;");
    pname = pname.replace("<","&lt;");
    pname = pname.replace(">","&gt;");
    xmlStream << pname.toUtf8().data();
  }
  xmlStream << "\" ";

  xmlStream << "nbArmies=\""<<nbArmies() << "\" ";
  xmlStream << "nbArmiesAdded=\""<<nbAddedArmies() << "\" ";

  xmlStream << " />" << std::endl;

}

void Country::send(QDataStream& stream)
{
  kDebug() << (m_belongsTo?m_belongsTo->name():"") << quint32(m_nbArmies) << quint32(m_nbAddedArmies);
  stream << m_name << (m_belongsTo?m_belongsTo->name():"") << quint32(m_nbArmies) << quint32(m_nbAddedArmies);
}

bool Country::hasAdjacentEnemy()
{
  for (uint j = 0; j < m_neighbours.size(); j++)
  {
    if ( m_neighbours[j]->owner() != m_belongsTo )
    {
      return true;
    }
  }
  return false;
}

void Country::highlight(const QColor& color, qreal opacity)
{
//   kDebug() << color;
  if (m_highlighting_locked)
  {
    return;
  }
  clearHighlighting();

  ONU* onu = m_automaton->game()->theWorld();
  if (onu == 0)
  {
    kWarning() << "onu is null" << endl;
    return;
  }
  QDomNode countryElement = onu->svgDom()->elementById(m_name);
  if (countryElement.isNull())
  {
    kWarning() << "Got a null element" << endl;
    return;
  }
//   kDebug() <<"got country"<< endl;

  onu->svgDom()->setCurrentNode(countryElement);
  onu->svgDom()->setStyleProperty("fill", color.name());
  onu->svgDom()->setStyleProperty("fill-opacity", QString::number(opacity));

//   kDebug() <<"loading"<< endl;
  QByteArray svg = onu->svgDom()->nodeToByteArray();
  m_renderer->load(svg);

//   kDebug() <<"loaded"<< endl;
  m_highlighting = new QGraphicsSvgItem(m_automaton->game()->backGndWorld());
  m_highlighting->setSharedRenderer(m_renderer);
  m_highlighting->setElementId(m_name);
  m_highlighting->setPos(
      (m_anchorPoint.x()-m_highlighting->boundingRect().width()/2)*onu->zoom(),
      (m_anchorPoint.y()-m_highlighting->boundingRect().height()/2)*onu->zoom());

  m_highlighting->scale(onu->zoom(), onu->zoom());
//   kDebug() << "done" << endl;
}

void Country::highlightAsAttacker()
{
//   kDebug();
  highlight(Qt::red, 0.6);
  m_highlighting_locked = true;
}

void Country::highlightAsDefender()
{
//   kDebug();
  highlight(Qt::yellow,0.6);
  m_highlighting_locked = true;
}

void Country::clearHighlighting()
{
//   kDebug() << m_highlighting_locked << (void*)m_highlighting;
  if (!m_highlighting_locked && m_highlighting!=0)
  {
    m_highlighting->hide();
    delete m_highlighting;
    m_highlighting = 0;
  }
//   kDebug() << "done" << endl;
}

bool Country::isHighlightingLocked()
{
//   kDebug();
  return m_highlighting_locked;
}

void Country::releaseHighlightingLock()
{
//   kDebug();
  m_highlighting_locked=false;
}


QDataStream& operator>>(QDataStream& stream, Country* country)
{
  country->reset();
  quint32 nbArmies, nbAddedArmies;
  QString ownerName;
  stream >> ownerName >> nbArmies >> nbAddedArmies;
  kDebug() << ownerName << nbArmies << nbAddedArmies;
  country->owner(country->automaton()->playerNamed(ownerName));
  country->nbArmies(nbArmies);
  country->nbAddedArmies(nbAddedArmies);
  country->createArmiesSprites();
  return stream;
}


void Country::copyForArena(Country* trueCountry)
{
  kDebug() << trueCountry->name();
  // remove existing elements
  clearAllSprites();
  if (m_flag)
  {
    m_flag->hide();
    delete m_flag;
    m_flag = 0;
  }

  // change parameters
  m_name = trueCountry->m_name;
  m_continent = trueCountry->m_continent;
  m_belongsTo = trueCountry->m_belongsTo;
  m_nbArmies = trueCountry->m_nbArmies;
  m_id = trueCountry->m_id;

  // make again the display
  createArmiesSprites();
}

} // closing namespace GameLogic

} // closing namespace Ksirk

#include "country.moc"
