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
#include "Sprites/flagsprite.h"
#include "Sprites/skinSpritesData.h"
#include "GameLogic/onu.h"

#include <klocale.h>
#include <kdebug.h>
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
  m_centralPoint(centralPoint), 
  m_pointFlag(flagPoint), 
  m_pointCannon(cannonPoint), 
  m_pointCavalry(cavalryPoint),
  m_pointInfantry(infantryPoint),
  m_id(id),
  m_highlighting(0)
{
//   kDebug() << k_funcinfo << m_name << ", " << this << endl;
}

Country::~Country()
{
//   kDebug() << "Deleting country " << m_name << ", " << this << endl;
  if (m_flag)
  {
    delete m_flag;
  }
}

void Country::reset()
{
//   kDebug() << "Country::reset " << m_name << endl;
  clearAllSprites();
  m_belongsTo = 0;
  nbArmies(1);
  createArmiesSprites(m_automaton->game()->backGnd());
  
  if (m_flag)
  {
    m_flag->hide();
//     delete m_flag;
    m_flag = 0;
  }
}

void Country::createArmiesSprites(BackGnd *backGnd)
{
  kDebug() << k_funcinfo << "zoom=" << backGnd->onu()->zoom() << endl;
  BackGnd* bg = backGnd==0?m_automaton->game()->backGnd():backGnd;
  
  unsigned int armies = nbArmies();
  clearAllSprites();
  int i = 0;
  while (armies >= 10) // Ajout des sprites de canon
  {
    CannonSprite *sprite = new CannonSprite(
        "cannon",
        backGnd,
        Sprites::SkinSpritesData::single().intData("cannon-frames"), 
        Sprites::SkinSpritesData::single().intData("cannon-versions"),
        backGnd->onu()->zoom());
    sprite-> setDestination(NULL);             // Sprite immobile
    sprite-> setPos((m_pointCannon.x()+5*i)*bg->onu()->zoom(),(m_pointCannon.y()+5*i)*bg->onu()->zoom());
    m_spritesCannons.append(sprite);
    i++;
    armies -= 10;
  }
  i = 0;
  while (armies >= 5) // Adding the cavalryman  sprites
  {
    CavalrySprite *sprite = new CavalrySprite(
        "cavalry",
        backGnd,
        Sprites::SkinSpritesData::single().intData("cavalry-frames"), 
        Sprites::SkinSpritesData::single().intData("cavalry-versions"),
        backGnd->onu()->zoom());
    sprite-> setDestination(NULL);             // Sprite immobile
    sprite-> setPos((m_pointCavalry.x()+5*i)*bg->onu()->zoom(),
                     (m_pointCavalry.y()+5*i)*bg->onu()->zoom());
    m_spritesCavalry.append(sprite);
    i++;
    armies -= 5;
  }
  i = 0;
  while (armies > 0) // Ajout des sprites de fantassin
  {
    InfantrySprite *sprite = new InfantrySprite(
        "infantry",
        backGnd,
        Sprites::SkinSpritesData::single().intData("infantry-frames"), 
        Sprites::SkinSpritesData::single().intData("infantry-versions"),
        backGnd->onu()->zoom());
    sprite-> setDestination(NULL);             // Sprite immobile
    sprite-> setPos((m_pointInfantry.x()+5*i)*bg->onu()->zoom(),
                     (m_pointInfantry.y()+5*i)*bg->onu()->zoom());
    m_spritesInfantry.append(sprite);
    i++;
    armies--;
  }

  if (m_belongsTo)
  {
    flag(m_belongsTo->flagFileName(), bg);
  }
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
  
  m_flag = new FlagSprite(theFlagFileName, backGnd,
      Sprites::SkinSpritesData::single().intData("flag-frames"), 
      Sprites::SkinSpritesData::single().intData("flag-versions"),
                           backGnd->onu()->zoom());
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
    createArmiesSprites(m_automaton->game()-> backGnd());
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
//  kDebug() << "Country::clearAllSprites()" << endl;
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

/*    xmlStream << "<flagPoint x=\""<<m_pointFlag.x()<<"\" y=\""<<m_pointFlag.y()<<"\" />" << std::endl;
    xmlStream << "<cavalryPoint x=\""<<m_pointCavalry.x()<<"\" y=\""<<m_pointCavalry.y()<<"\" />" << std::endl;
    xmlStream << "<infantryPoint x=\""<<m_pointInfantry.x()<<"\" y=\""<<m_pointInfantry.y()<<"\" />" << std::endl;

    xmlStream << "<neighbours>" << std::endl;
    for (unsigned int i = 0; i < m_spritesCannons.size() ; i++)
    {
      xmlStream << "<neighbour name=\""<<m_neighbours.at(i)->name()<<"\" />" << std::endl;
    }
    xmlStream << "</neighbours>" << std::endl;
    
    m_spritesCannons.saveXmlAll(xmlStream);

    m_spritesCavalry.saveXmlAll(xmlStream);

    m_spritesInfantry.saveXmlAll(xmlStream);
  
  xmlStream << "</country>" << std::endl;
*/
}

void Country::send(QDataStream& stream)
{
  
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

void Country::highlight(const QColor& color)
{
  clearHighlighting();
  
  QBrush brush(color);
  m_highlighting = new QGraphicsSvgItem(m_automaton->game()->backGnd());
//       m_pointFlag.x()-m_flag->boundingRect().width()/2,
//       m_pointFlag.y()-m_flag->boundingRect().height()/2,
//       m_flag->boundingRect().width()*2,
//       m_flag->boundingRect().height()*2,
//       m_automaton->game()->backGnd());
  m_highlighting->setSharedRenderer(m_automaton->game()->theWorld()->renderer());
  m_highlighting->setElementId(m_name);
  m_highlighting->setPos(
      m_centralPoint.x()-m_highlighting->boundingRect().width()/2,
      m_centralPoint.y()-m_highlighting->boundingRect().height()/2);

//   QImage image(m_automaton->game()->theWorld()->mask().createMaskFromColor(qRgb(0,0,m_id)));

//   QBitmap bm(QPixmap::fromImage(m_automaton->game()->theWorld()->mask()).createMaskFromColor(qRgb(0,0,m_id)));

  /*  QImage image(image2.createMaskFromColor(qRgb(0,0,0),Qt::MaskOutColor));*/

//   QPixmap pm(m_automaton->game()->backGnd()->pixmap());
//   pm.fill(Qt::black);
//   pm.setMask(bm);


//   m_highlighting = new QGraphicsPixmapItem(
//   pm,
//                                            m_automaton->game()->backGnd());
                                           
//   ((QGraphicsEllipseItem*)m_highlighting)->setBrush(brush);

}

void Country::highlightAsAttacker()
{
  QColor color(Qt::red);
  highlight(color);
}

void Country::highlightAsDefender()
{
  QColor color(Qt::yellow);
  highlight(color);
}

void Country::clearHighlighting()
{
  if (m_highlighting!=0)
  {
    m_highlighting->hide();
    delete m_highlighting;
    m_highlighting = 0;
  }
}

QDataStream& operator>>(QDataStream& stream, Country* country)
{
  quint32 nbArmies, nbAddedArmies;
  QString ownerName;
  stream >> ownerName >> nbArmies >> nbAddedArmies;
  country->owner(country->automaton()->playerNamed(ownerName));
  country->nbArmies(nbArmies);
  country->nbAddedArmies(nbAddedArmies);
  country->reset();
  return stream;
}


} // closing namespace GameLogic

} // closing namespace Ksirk

#include "country.moc"
