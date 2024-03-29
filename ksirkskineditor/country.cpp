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

/* begin                : Wed Jul 18 2001 */

#include "country.h"
#include "onu.h"

#include <KLocalizedString>
#include "ksirkskineditor_debug.h"
#include <QSvgRenderer>
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
  m_highlighting(nullptr),
  m_renderer(new QSvgRenderer()),
  m_highlighting_locked(false)
{
//   qCDebug(KSIRKSKINEDITOR_LOG) << m_name << ", " << this ;
}

Country::~Country()
{
//   qCDebug(KSIRKSKINEDITOR_LOG) << "Deleting country " << m_name << ", " << this ;
  delete m_renderer;
}

void Country::reset()
{
//   qCDebug(KSIRKSKINEDITOR_LOG) << "Country::reset " << m_name ;
}

bool Country::communicateWith(const Country* otherCountry) const
{
  if (!otherCountry)
  {
    qCDebug(KSIRKSKINEDITOR_LOG) << "OUT otherCountry null Country::communicateWith";
    return false;
  }

  // a country is considered to communicate with itself
  if (otherCountry == this) {return true;}

//    qCDebug(KSIRKSKINEDITOR_LOG) << "Country::communicateWith (" << name() << ", " << otherCountry-> name() << ")" << endl << flush;
  unsigned int nbNeighbours = neighbours().size();
  for (unsigned int i = 0; i < nbNeighbours; i++)
  {
    if (neighbours().at(i) == otherCountry)
    {
//            qCDebug(KSIRKSKINEDITOR_LOG) << "OUT true Country::communicateWith" << endl << flush;
      return true;
    }
  }
//    qCDebug(KSIRKSKINEDITOR_LOG) << "OUT false Country::communicateWith" << endl << flush;
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
  qCDebug(KSIRKSKINEDITOR_LOG) << pt;
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
//    qCDebug(KSIRKSKINEDITOR_LOG) << "Country::neighbours" << endl << flush;
  return m_neighbours;
}

/** No descriptions */
const QList< Country* >& Country::neighbours() const
{
//    qCDebug(KSIRKSKINEDITOR_LOG) << "Country::neighbours const" << endl << flush;
  return m_neighbours;
}

void Country::highlight(QGraphicsScene* scene, ONU* onu, const QColor& color, qreal opacity)
{
  qCDebug(KSIRKSKINEDITOR_LOG) << m_name << color << opacity;
  if (m_highlighting_locked)
  {
    return;
  }
  clearHighlighting();

  QDomNode countryElement = onu->svgDom()->elementById(m_name);
  if (countryElement.isNull())
  {
    qCWarning(KSIRKSKINEDITOR_LOG) << "Got a null element";
    return;
  }
//   qCDebug(KSIRKSKINEDITOR_LOG) <<"got country";

  onu->svgDom()->setCurrentNode(countryElement);
  onu->svgDom()->setStyleProperty(QStringLiteral("fill"), color.name());
  onu->svgDom()->setStyleProperty(QStringLiteral("fill-opacity"), QString::number(opacity));

//   qCDebug(KSIRKSKINEDITOR_LOG) <<"loading";
  QByteArray svg = onu->svgDom()->nodeToByteArray();
  m_renderer->load(svg);

//   qCDebug(KSIRKSKINEDITOR_LOG) <<"loaded";
  m_highlighting = new QGraphicsSvgItem();
  m_highlighting->setSharedRenderer(m_renderer);
  m_highlighting->setElementId(m_name);
  qCDebug(KSIRKSKINEDITOR_LOG) << "anchor point=" << m_anchorPoint;
  qCDebug(KSIRKSKINEDITOR_LOG) << "set highlighting pos to " << (m_anchorPoint.x()-m_highlighting->boundingRect().width()/2)
  << (m_anchorPoint.y()-m_highlighting->boundingRect().height()/2) ;
  m_highlighting->setPos(
      (m_anchorPoint.x()-m_highlighting->boundingRect().width()/2),
      (m_anchorPoint.y()-m_highlighting->boundingRect().height()/2));
  m_highlighting->setZValue(5);
  scene->addItem(m_highlighting);
//   m_highlighting->scale(onu->zoom(), onu->zoom());
//   qCDebug(KSIRKSKINEDITOR_LOG) << "done";
}

void Country::clearHighlighting()
{
//   qCDebug(KSIRKSKINEDITOR_LOG) << m_highlighting_locked << (void*)m_highlighting;
  if (!m_highlighting_locked && m_highlighting!=nullptr)
  {
    m_highlighting->hide();
    delete m_highlighting;
    m_highlighting = nullptr;
  }
//   qCDebug(KSIRKSKINEDITOR_LOG) << "done";
}

bool Country::isHighlightingLocked()
{
//   qCDebug(KSIRKSKINEDITOR_LOG);
  return m_highlighting_locked;
}

void Country::releaseHighlightingLock()
{
//   qCDebug(KSIRKSKINEDITOR_LOG);
  m_highlighting_locked=false;
}


QDataStream& operator>>(QDataStream& stream, Country* country)
{
  country->reset();
  quint32 nbArmies, nbAddedArmies;
  QString ownerName;
  stream >> ownerName >> nbArmies >> nbAddedArmies;
  qCDebug(KSIRKSKINEDITOR_LOG) << ownerName << nbArmies << nbAddedArmies;
//   country->owner(country->automaton()->playerNamed(ownerName));
  return stream;
}


}
