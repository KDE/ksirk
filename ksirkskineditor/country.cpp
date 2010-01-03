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
#include "onu.h"

#include <klocale.h>
#include <kdebug.h>
#include <ksvgrenderer.h>
#include <QString>
#include <QApplication>
#include <QDataStream>
#include <QBitmap>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QDomNode>

namespace KsirkSkinEditor
{

Country::Country(
                  const QString& theName,
                  const QPointF& anchorPoint,
                  const QPointF& centralPoint,
                  const QPointF& flagPoint,
                  const QPointF& cannonPoint, 
                  const QPointF& cavalryPoint,
                  const QPointF& infantryPoint/*, 
                  unsigned int id*/) :
  m_name(theName),
  m_anchorPoint(anchorPoint),
  m_centralPoint(centralPoint),
  m_pointFlag(flagPoint),
  m_pointCannon(cannonPoint), 
  m_pointCavalry(cavalryPoint),
  m_pointInfantry(infantryPoint),
//   m_id(id),
  m_highlighting(0),
  m_renderer(new KSvgRenderer()),
  m_highlighting_locked(false)
{
//   kDebug() << m_name << ", " << this << endl;
}

Country::~Country()
{
//   kDebug() << "Deleting country " << m_name << ", " << this << endl;
  delete m_renderer;
}

void Country::reset()
{
//   kDebug() << "Country::reset " << m_name << endl;
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

const QString& Country::name() const
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
  kDebug() << pt;
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

/** No descriptions */
void Country::neighbours(const QList<Country*>& neighboursVect)
{
  m_neighbours = neighboursVect;
}

/** No descriptions */
QList< Country* >& Country::neighbours()
{
//    kDebug() << "Country::neighbours" << endl << flush;
  return m_neighbours;
}

/** No descriptions */
const QList< Country* >& Country::neighbours() const
{
//    kDebug() << "Country::neighbours const" << endl << flush;
  return m_neighbours;
}

void Country::highlight(QGraphicsScene* scene, ONU* onu, const QColor& color, qreal opacity)
{
  kDebug() << m_name << color << opacity;
  if (m_highlighting_locked)
  {
    return;
  }
  clearHighlighting();

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
  m_highlighting = new QGraphicsSvgItem();
  m_highlighting->setSharedRenderer(m_renderer);
  m_highlighting->setElementId(m_name);
  kDebug() << "anchor point=" << m_anchorPoint;
  kDebug() << "set highlighting pos to " << (m_anchorPoint.x()-m_highlighting->boundingRect().width()/2)
  << (m_anchorPoint.y()-m_highlighting->boundingRect().height()/2) ;
  m_highlighting->setPos(
      (m_anchorPoint.x()-m_highlighting->boundingRect().width()/2),
      (m_anchorPoint.y()-m_highlighting->boundingRect().height()/2));
  m_highlighting->setZValue(5);
  scene->addItem(m_highlighting);
//   m_highlighting->scale(onu->zoom(), onu->zoom());
//   kDebug() << "done" << endl;
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
//   country->owner(country->automaton()->playerNamed(ownerName));
  return stream;
}


}

#include "country.moc"
