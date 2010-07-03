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

#define KDE_NO_COMPAT

#ifndef KSIRK_COUNTRY_H
#define KSIRK_COUNTRY_H

#include "Sprites/infantrysprite.h"
#include "Sprites/cavalrysprite.h"
#include "Sprites/cannonsprite.h"
#include "Sprites/animspriteslist.h"

#include <kgame/kgameproperty.h>

#include <qpoint.h>
#include <qstring.h>

class QSvgRenderer;
class QGraphicsSvgItem;

namespace Ksirk
{

class BackGnd;
class FlagSprite;

namespace GameLogic
{
class Player;
class Continent;
class GameAutomaton;

/**
 * Each country of the map is represented by a Country object. It has a name,
 * a point for its flag and points for its canon, cavalry, etc.
 * Also, it stores pointers to the objects that represent  its owner and the
 * sprites of its flag and its armies.
 */
class Country : QObject
{
  Q_OBJECT 

public:
  /**
    * Constructor
    * @param theName The name of this country.
    * @param centralPoint The point around which the fighter will be placed.
    * @param flagPoint The point (top left corner) where the flag sprite is drawn.
    * @param cannonPoint The point (top left corner) where the cannon sprite is drawn.
    * @param cavalryPoint The point (top left corner) where the cavalry sprite is drawn.
    * @param infantryPoint The point (top left corner) where the infantry sprite is drawn.
    * @param id The unique integer identifier of this country.
    */
  Country(GameAutomaton* game,
      const QString& theName,
      const QPointF& anchorPoint,
      const QPointF& centralPoint,
      const QPointF& flagPoint,
      const QPointF& cannonPoint,
      const QPointF& cavalryPoint,
      const QPointF& infantryPoint);

  /** Default destructor */
  virtual ~Country();

  GameAutomaton* automaton() {return m_automaton;}

  /**
    * Removes the sprites (flag and soldiers), the owner, etc.
    * The sprites ared deleted.
    */
  void reset();

  /**
    * Creates the sprites necessary to display the armies of the country.
    * Eventually removes previously existing sprites.
    * @param backGnd The background onto which this country sprites will be drawn.
    */
  void createArmiesSprites();

  /**
    * Creates the sprite of the contry's flag. Eventually removes a previously
    * existing sprite.
    * @param theFlagFileName The flag's sprite file name :-)
    * @param backGnd The background onto which this country sprites will be drawn.
    */
  void flag(const QString& theFlagFileName, BackGnd *backGnd);
  
  /**
    * Test if this is a neighbour of country
    * @param country The country to test if this one communicate with.
    * @return true if @ref country communicate with this; false otherwise.
    */
  bool communicateWith(const Country *country) const;

  /**
    * Returns the continent this country is in.
    * @return The continent this country is in.
    */
  inline Continent* continent() {return m_continent;}
  
  /**
    * Sets the continent this country is in.
    */
  inline void setContinent(Continent* continent) {m_continent = continent;}
  
  /**
    * Change the owner of this to player and update the number of countries for 
    * previous and new owners.
    * @param player The new owner of this country.
    */
  void owner(Player *player);
  
  //@{
  /**
    * return a pointer to the Player owner of this country.
    */
  const Player* owner() const;
  Player* owner();
  //@}

  /**
    * Return the number of armies in this country
    */
  unsigned int nbArmies() const;

  /**
    * Change the number of armies to nb
    */
  void nbArmies(unsigned int nb);

  /**
    * Add nb armies. Defaults to 1.
    */
  void incrNbArmies(unsigned int nb=1);

  /**
    * Remove nb armies. Defaults to 1.
    */
  void decrNbArmies(unsigned int nb=1);

  /**
    * Return the name of the country
    */
  const QString name() const;

  /**
    * Return the localized name of the country
    */
  const QString i18name() const;

  const QPointF& anchorPoint() const;
  /**
    * Return a point inside the country territory around which are drawn the 
    * fighters.
    */
  const QPointF& centralPoint() const;

  /**
    * Return the point where the flag is displayed
    */
  const QPointF& pointFlag() const;

  /**
    * Return the point where the cannons are displayed
    */
  const QPointF& pointCannon() const;

  /**
    * Return the point where the cavalrymen are displayed
    */
  const QPointF& pointCavalry() const;

  /**
    * Return the point where the infantrymen are displayed
    */
  const QPointF& pointInfantry() const;

  /**
    * Set the anchor point.
    */
  void anchorPoint(const QPointF pt);

  /**
    * Set the point guaranteed to be inside this country territory and around 
    * which are drawn the fighters.
    */
  void centralPoint(const QPointF pt);

  /**
    * Set the point where the flag is displayed
    */
  void pointFlag(const QPointF pt);

  /**
    * Set the point where the cannons are displayed
    */
  void pointCannon(const QPointF pt);

  /**
    * Set the point where the cavalrymen are displayed
    */
  void pointCavalry(const QPointF pt);

  /**
    * Set the point where the infantrymen are displayed
    */
  void pointInfantry(const QPointF pt);

  /**
    * Return the list of cannon sprites
    */
  AnimSpritesList< CannonSprite >& spritesCannons();

  /**
    * Return the list of Cavalrymen sprites
    */
  AnimSpritesList< CavalrySprite >& spritesCavalry();

  /**
    * Return the list of Infantrymen sprites
    */
  AnimSpritesList< InfantrySprite >& spritesInfantry();

  /** Sets the list of neighbour countries. */
  void neighbours(const QList<Country*>& neighboursVect);

  //@{
  /** Returns the list of neighbour countries */
  QList< Country* >& neighbours();
  const QList< Country* >& neighbours() const;
  //@}
  void clearAllSprites();

  /** Returns the point for the given sprite, depending on its actual class */
  const QPointF& pointFor(const AnimSprite* sprite);

  /**
    * Saves a XML representation of the country for game saving purpose
    * @param xmlStream The stream to write on
    */
  void saveXml(QTextStream& xmlStream);

  /** 
    * Transmit data about this country on the network, through the given 
    * stream.
    */
  void send(QDataStream& stream);

  //@{
  /** Accessors to this country's unique integer identifier. */
/*  unsigned int id() const {return m_id;}
  unsigned int id() {return m_id;}
  void id(unsigned int id) {m_id = id;}*/
  //@}

  /**
    * Tests if there is at least one enemey adjacent to this country.
    * @return true if this country has an enemy neighbour and false otherwise.
    */
  bool hasAdjacentEnemy();

  void highlight(const QColor& color = Qt::white, qreal opacity = 1.0);

  void highlightAsAttacker();
  
  void highlightAsDefender();
  
  void clearHighlighting();

  bool isHighlightingLocked();
  void releaseHighlightingLock();

  /**
    * Copy information of the real country in a country of the arena.
    * @param trueCountry is the real country
    */
  void copyForArena(Country* trueCountry);

  private:
  GameAutomaton* m_automaton;
  
  /**
   * A pointer to the continent this country is in.
   */
  Continent* m_continent;
  
  /**
    * A pointer to the Player object that holds the country. 0 if it is not
    * affected.
    */
  Player* m_belongsTo;

  /**
    * A pointer to the sprite of the country's flag
    */
  FlagSprite* m_flag;

  /**
    * the number of armies held by the country (used to compute the number
    * of soldiers, horses and cannons
    */
  unsigned int  m_nbArmies;
  
  /**
    * The name of the country
    */
  QString m_name;

  /** the array of neigbours of this country */
  QList<Country*> m_neighbours;

  QPointF m_anchorPoint;

  /**
    * a point situated inside this country teritory such that any click on
    * this point (for example by an AI player) will be a click on this country
    */
  QPointF m_centralPoint;

  /**
    * the point of the upper left corner of the country's flag sprite
    */
  QPointF m_pointFlag;

  /**
    * the point of the upper left corner of the country's first cannon sprite
    * the subsequent cannons sprites are shifted by a fixed number of pixels
    */
  QPointF m_pointCannon;

  /**
    * the point of the upper left corner of the country's first cavalryman
    * sprite.The subsequent cavalrymen sprites are shifted by a fixed number
    * of pixels
    */
  QPointF m_pointCavalry;

  /**
    * the point of the upper left corner of the country's first soldier sprite
    * The subsequent soldier sprites are shifted by a fixed number of pixels
    */
  QPointF m_pointInfantry;

  /**
    * The list of the cannon sprites used to represent the armies of the
    * country
    */
  AnimSpritesList< CannonSprite > m_spritesCannons;

  /**
    * The list of the cavalrymen sprites used to represent the armies of the
    * country
    */
  AnimSpritesList< CavalrySprite > m_spritesCavalry;

  /**
    * The list of the soldiers sprites used to represent the armies of the
    * country
    */
  AnimSpritesList< InfantrySprite > m_spritesInfantry;

  QGraphicsSvgItem* m_highlighting;

  QSvgRenderer* m_renderer;

  bool m_highlighting_locked;

  QGraphicsSimpleTextItem* m_nbArmiesItem;
};

QDataStream& operator>>(QDataStream& stream, Country* country);
  
} // closing namespace GameLogic

} // closing namespace Ksirk

#endif // KSIRK_COUNTRY_H

