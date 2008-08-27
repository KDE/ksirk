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

#ifndef KSIRKSKINEDITORCOUNTRY_H
#define KSIRKSKINEDITORCOUNTRY_H

#include <QPoint>
#include <QString>
#include <QObject>
#include <QList>
#include <QColor>

class KSvgRenderer;
class QGraphicsSvgItem;

#include <ostream>

namespace KsirkSkinEditor
{

class Continent;

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
  Country(const QString& theName,
      const QPointF& anchorPoint,
      const QPointF& centralPoint,
      const QPointF& flagPoint, const QPointF& cannonPoint, const QPointF& cavalryPoint,
      const QPointF& infantryPoint/*, unsigned int id*/);

  /** Default destructor */
  virtual ~Country();

  /**
    * Removes the sprites (flag and soldiers), the owner, etc.
    * The sprites ared deleted.
    */
  void reset();

  /**
    * Creates the sprite of the contry's flag. Eventually removes a previously
    * existing sprite.
    * @param theFlagFileName The flag's sprite file name :-)
    * @param backGnd The background onto which this country sprites will be drawn.
    */
  void flag(const QString& theFlagFileName);
  
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
    * Return the name of the country
    */
  const QString& name() const;

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

  /** Sets the list of neighbour countries. */
  void neighbours(const QList<Country*>& neighboursVect);

  //@{
  /** Returns the list of neighbour countries */
  QList< Country* >& neighbours();
  const QList< Country* >& neighbours() const;
  //@}

  /**
    * Saves a XML representation of the country for game saving purpose
    * @param xmlStream The stream to write on
    */
//   void saveXml(std::ostream& xmlStream);

  //@{
  /** Accessors to this country's unique integer identifier. */
/*  unsigned int id() const {return m_id;}
  unsigned int id() {return m_id;}
  void id(unsigned int id) {m_id = id;}*/
  //@}

  void highlight(const QColor& color = Qt::white, qreal opacity = 1.0);

  void clearHighlighting();

  bool isHighlightingLocked();
  void releaseHighlightingLock();

  private:
  
  /**
   * A pointer to the continent this country is in.
   */
  Continent* m_continent;
  
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

  /** The unique integer identifier of this country. */
//   unsigned int m_id;

  QGraphicsSvgItem* m_highlighting;

  KSvgRenderer* m_renderer;

  bool m_highlighting_locked;

};

QDataStream& operator>>(QDataStream& stream, Country* country);

}

#endif

